# ESPtree
another ESP8266 controller for neopixels strip, with (ugly but functional) WEB interface and some cool features (own SPI transfer method)
THIS IS **NOT COMPLETED**... Tested with ESP01 and ESP12. SPI method, works stable (300leds 17ms update time).
>big mess in source, *but yet usable* ;-)
>
><img width="300" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek01.jpg" /> <img height="400" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek02.jpg" /><img width="300" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek03.jpg" />
><img width="300" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek04.jpg" /><img width="300" src="https://github.com/dMbski/ESPtree/blob/master/screens/Schowek05.jpg" />

#### Use library Adafruit or _SPI_ or _i2s_transfer (emulated WS signal). 
Select library in file helpers.cpp. 
With *Adafruit NeoPixel* library, **pin, quantity and color type of leds** are changable at runtime via WEB.

With *SPI_transfer* or *I2S_transfer*, **type and quantity of leds** are changable either.

SPI transfer uses small HSPI fifo buffer (60 from 64bytes) to buffer data for 5 leds. HSPI speed is calculated for 160MHZ CPU (80MHz works too), there is compiller message with SPI speed. WS signal is created by simulate WS timing in 4bit SPI data (1 led in 3 x uint32) and is generated via MOSI pin. Just connect it to WS281x DIN pin. 

This implementation takes over SPI (MOSI pin) and is not usable as proper SPI bus, but...
To use SPI with another device, AND gate is needed. To simulate CS (active high) connect to A, B to MOSI-this could be TTL level shifter either.

I use simple transistor voltage level shifter.
>
>Standard SPI.writebytes does not work at all. Only SPI.write32 works. Just set SPI freq around 3.2MHz and send wspacket prepared by _prepareSPIpacket()_ function.
>I wanted to use https://github.com/MetalPhreak/ESP8266_SPI_Driver but finally had to write own method to transfer without waiting for ends.
>Thanks to His library I wrote my SPI method to send data with HSPI buffer. 
>

**I2S_transfer** uses large RAM buffer for DMA transfer. This method is _copied_ from NeoPixelBus library (https://github.com/Makuna/NeoPixelBus) and little changed. I _borrowed_ it because of need to change the type and number of leds during runtime.

Every method has cons and pros.

Choose wisely.
#### Support for required max amperage draw, with correction (PowerFactor).
Beware, when use pin used to connectivity *ex. NeoEsp8266Dma800KbpsMethod needs UART0 RX pin -GPIO3 (which is used also to upload firmware)* strip will lights without control.
#### Support for OTA.
Firmware update via httpupdate from configtree page (at the bottom). See credentials data from helpers.cpp (admin,admin).
#### WebSockets ?
Works in all aspects (sending parameters, led data) but...
When sending data has low speed just interferes with other functions (mostly on preview leds). There are some glitches then, even WDT resets. Works best with i2s method.
There is feature when cycle is to long *(4xPERIOD_TASK_WIFI)* twice , resets ESP. 
#### Use key to change effect.
I tested this on MCU v2 clone with key marked Flash which is connected to GPIO0. Changable in helpers.cpp (USE_KEY_GPIO). Enabled INPUT_PULLUP.
#### Use WPS connectivity
~~I use VSS with platformio and when it is enabled, I get compiller error. Tested in Arduino IDE, works.~~
Works.
#### Use status LED.
Use led connected to pin GPIO02. Led build in ESP12's pcb.
#### Arduino IDE
To run this project in Arduino IDE, just copy /src folder to Arduino's sketches folder. Rename it to esptree and **rename** file main.cpp to esptree.ino. **Download needed libraries.**
#### Usable and tested only with esp8266:
- ESP01 module -need to disable OTA, no need to use SPIFF (all pages in flash);
- ESP12e -all features

**TODO** (some ideas):
-clean code mess
-fully implement different types of leds (color type) in string (id could be used)
-**implement SPIFF** webpages
-implement SPIFF graphics and animations
-system cfg on SPIFF ()
-implement some sound depending effects (VU meter, brightness, etc)
-effects playlist (SPIFF file)


firmware.bin - compiled for ESP12, use SPI method.
