# nitroxanalyser
Code for a DIY nitrox analayser based on https://github.com/ppppaoppp/DIY-Nitrox-Analyzer-04_12_2019

Hardware is essentially the same except for a connection from the battery to the ADC for battery voltage. There are a few differences in the software - I went with gain 16 for the mV readings which I think will work for the sensor I have in mind. The battery needs gain two thirds for that range, so the program swaps as needed. I had a look at what the running average library was doing and decided to just bring that into the program since I just needed a couple of bits of it.

In Adafruit_SSD1306.cpp I comment out the lines that display the splash screen and display my own. Vanity of vanities; all is vanity.

It will be obvious to anyone who is a C programmer that I'm not one.
