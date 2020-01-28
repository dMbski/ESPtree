#ifndef _ESPSPI_TO_WS_H_
#define _ESPSPI_TO_WS_H_
#include <Arduino.h>
#include "spi_register.h"

#ifndef HSPI
#define HSPI 1
#endif

#ifndef ESP8266
#error "ONLY SUPPORTS ESP8266!"
//if ESP is not detected properly comment this
#endif

// neo_type
#ifndef NEO_RGB
/*  Definitions from the Adafruit NeoPixel library. Some standard*/
#define NEO_RGB 0x06 //mostly for WS2811
#define NEO_RBG 0x09
#define NEO_GRB 0x52 //default for WS2812
#define NEO_GBR 0xA1
#define NEO_BRG 0x58
#define NEO_BGR 0xA4
#endif

struct un_color24
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
};

#ifndef _un_color32_
#define _un_color32_
union un_color32 {
    uint32_t c32;
    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t id;
    } c8;
};
#endif

//////////// SPI transfer
void prepareHSPI();
//  Sends rgb values from rgbdata over HSPI from un_color24
//  powerfactor - 100 skips this percentage of value to set when sending (good for power or global brightness steering)
void sendSPI24(un_color24 *rgbdata24, uint32_t pixelcount, uint8_t neo_type, uint8_t powerfactor);
inline void prepareSPIpacket(un_color24 *ledcolor, uint32_t *wspack, uint8_t neo_type, uint32_t powerfactor);

//  Returns powerneed mA
//  Sends rgb values from rgbdata over HSPI from un_color32
uint32_t sendSPI32(un_color32 *rgbdata32, uint32_t pixelcount, uint8_t neo_type, uint8_t powerfactor);
inline void prepareSPIpacket(un_color32 *ledcolor, uint32_t *wspack, uint8_t neo_type, uint32_t powerfactor);

//used privatly

uint32_t byteToSPIData(uint8_t val);
inline uint32_t setNibble(uint32_t innumber, uint32_t nibble, uint8_t nonibb);




#endif
