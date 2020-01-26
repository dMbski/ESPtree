#ifndef _ESPI2S_TO_WS_H_
#define _ESPI2S_TO_WS_H_
#include <Arduino.h>

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
/*
struct un_color24
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
};
*/
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

void prepareI2S(uint32_t pixelcount);
//  Sends rgb values from rgbdata over HSPI from un_color24
//  powerfactor - 100 skips this percentage of value to set when sending (good for power or global brightness steering)
//  Sends rgb values from rgbdata over HSPI from un_color32
void sendI2S(un_color32 *rgbdata32, uint32_t pixelcount, uint8_t neo_type, uint8_t powerfactor);

#endif
