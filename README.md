# ESPtree
another ESP8266 controller for neopixels strip, with WEB interface and some cool features
THIS IS **NOT COMPLETED**...barely tested 
>big mess in source, *but yet usable* ;-)
>
><img width="300" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek01.jpg" /> <img height="400" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek02.jpg" /><img width="300" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek03.jpg" />
><img width="300" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek04.jpg" /><img width="300" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek05.jpg" />

#### Use library Adafruit or NeoPixelBus or even SPI transfer (emulate 800kHz 3.2MHz/4). 
Select library in file helpers.cpp. 
With *Adafruit NeoPixel* library, **pin, quantity and color type of leds** are changable at runtime via WEB.

With *SPI_transfer*, **type and quantity of leds** are changable either.

SPI transfer with write32 method works nicely. This method does not need additional buffer (when write to strip use only 4x3 bytes). 
SPI is not usable with this adaptation.
But there is SPI.writeBytes with hardware buffer 64 bytes, but when using this method strip randomly flickers. 

*Additionally in SPI library* (file SPI.cpp) to speed up, remove waitings at the end of using method to send data, SPI.write32.

Every method has cons and pros.

Choose wisely.
#### Support for required max amperage draw, with correction (PowerFactor).
Beware, when use pin used to connectivity *ex. NeoEsp8266Dma800KbpsMethod needs UART0 RX pin -GPIO3 (which is used also to upload firmware)* strip will lights without control.
#### Support for OTA.
Firmware update via httpupdate from configtree page (at the bottom). See credentials data from helpers.cpp (admin,admin).
#### WebSockets ?
Works in all aspects (sending parameters, led data) but...
When sending data has low speed just interferes with other functions. There are some glitches then, even WDT resets. 
There is feature when cycle is to long *(4xPERIOD_TASK_WIFI)* twice , resets ESP. 
#### Use key to change effect.
I tested this on MCU v2 clone with key marked Flash which is connected to GPIO0. Changable in helpers.cpp (USE_KEY_GPIO). Enabled INPUT_PULLUP.
#### Use WPS connectivity
I use VSS with platformio and when it is enabled, I get compiller error. Tested in Arduino IDE, works.
#### Use status LED.
Use led connected to pin specified by LED_BUILTIN (MCU v2 GPIO16).
#### Arduino IDE
To run this project in Arduino IDE, just copy /src folder to Arduino's sketches folder. Rename it to esptree and **rename** file main.cpp to esptree.ino. **Download needed libraries.**
#### Usable and tested only with esp8266:
- ESP01 module -need to disable OTA, no need to use SPIFF (all pages in flash);
- ESP12e -all features

firmware.bin - compiled for ESP12 module with Adafruit library
