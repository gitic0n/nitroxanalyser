#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1015.h>
#include <eab.h>

//OLED definitions
#define SCREEN_WIDTH 128              // OLED display width, in pixels
#define SCREEN_HEIGHT 64              // OLED display height, in pixels
#define OLED_RESET     1              // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_ADS1115 myADC(0x48); // Define ADC address

const byte buttonPin = A3;    // Define calibration push button

float currentmv = 0;              //Define ADC reading value
//const float multiplier = 0.0625F; //ADC value/bit for gain of 2
const float sensorMultiplier = 0.0078125; // ADC value/bit for gain of 16
const float batteryMultiplier = 187.5; // ADC value/bit for gain of 2/3
const int calCount = 400;         //Calibration samples
const int readDelay = 2;          //ADC read delay between samples
float calValue = 0;               //Calibration value (%/mV)
float percentO2;                  //% O2
byte buttonState;                 //Button state

// Running average variables
uint8_t RAsize;
uint8_t cnt;
uint8_t idx;
float   sum;
float*  ar;

void setup() {
  Serial.begin(9600);       //Start serial port (debugging purposes only)
  RAsize = 30;   // Number of readings to average
  initRA();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   //Start OLED Display
  display.drawBitmap(0, 0, eab11, 128, 64, 1);
  display.display();
  delay(2000);
  myADC.begin();                               //Start ADC
  pinMode(buttonPin, INPUT_PULLUP);            //Define push button as active low
  //Startup calibration
  calibrate();
}

void loop() {
  //On-demand calibration
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    calibrate();
  }
  show(getmV(), getBat());
}

float getmV() {
  myADC.setGain(GAIN_SIXTEEN);                     //Set ADC gain
  for (int x = 0; x <= RAsize; x++) {
    float millivolts = 0;
    millivolts = myADC.readADC_Differential_0_1();
    addValue(millivolts);
    delay(16);
    return getAverage();
  }
}

float getBat() {
  myADC.setGain(GAIN_TWOTHIRDS);                     //Set ADC gain
  uint16_t batVoltageReading = myADC.readADC_SingleEnded(2);
  float batVoltage = (batVoltageReading * batteryMultiplier) / 1000000;
  return batVoltage;
}

void show(float mv, float bat) {
  mv *= sensorMultiplier;
  percentO2 = mv * calValue; //Convert mV ADC reading to % O2
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(percentO2, 1);
  display.println(" %");
  display.setCursor(0, 16);
  display.print(mv, 2);
  display.println(" mV");
  display.setTextSize(2);
  display.setCursor(0, 36);
  display.setTextSize(1);
  display.print(bat, 2);
  display.print(" V");
  display.setCursor(0, 48);
  display.print("MOD @ 1.4bar: ");
  display.print(10 * (1.4 / (percentO2 / 100) - 1), 0);
  display.print("m");

  display.display();
}

void calibrate() {

  myADC.setGain(GAIN_SIXTEEN);                     //Set ADC gain
  display.clearDisplay();        //CLS
  display.display();             //CLS
  display.setTextColor(WHITE);   //WHITE for monochrome
  display.setTextSize(1);
  display.setCursor(20, 32);
  display.println("CALIBRATING");

  display.display();
  display.clearDisplay();

  long sum = 0;
  for (int i = 0; i < calCount; i++) {
    float millivolts = 0;
    millivolts = myADC.readADC_Differential_0_1();
    sum += millivolts;
    delay(readDelay);
  }
  currentmv = sum / calCount;

  currentmv = currentmv * sensorMultiplier;
  calValue = 20.900 / currentmv;
}

void initRA()
{
  ar = (float*) malloc(RAsize * sizeof(float));
  if (ar == NULL) RAsize = 0;
  cnt = 0;
  idx = 0;
  sum = 0.0;

  for (uint8_t i = 0; i < RAsize; i++)
  {
    ar[i] = 0.0; // keeps addValue simpler
  }
}

// adds a new value to the data-set
void addValue(const float value)
{
  if (ar == NULL) return;  // allocation error

  sum -= ar[idx];
  ar[idx] = value;
  sum += ar[idx];
  idx++;

  if (idx == RAsize) idx = 0;  // faster than %

  // update count as last otherwise if ( cnt == 0) above will fail
  if (cnt < RAsize) cnt++;
}

float getAverage()
{
  if (cnt == 0) return NAN;
  return sum / cnt;
}
