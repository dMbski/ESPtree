#ifndef HELPERS_CPP
#define HELPERS_CPP
#include <Arduino.h>
#include <c_types.h>
#ifndef u8_t
typedef uint8_t u8_t;
typedef uint16_t u16_t;
#endif

//configurations
#define SERVER_WS_PORT 81
#define SERVER_HTML_PORT 80



/* Use only one library bellow. Comment one*/
//#define USE_ADA_NEOPIXEL
//#define USE_SPITRANSFER //use hspi bus pin mosi gpio13 D7 to output data 
                        //no need to additional buffer but spi (all pins?)is not usable for other purposes
                        //with some hardware it is usable for other purposes-should disable sending WS data
#define USE_I2STRANSFER //use i2s bus pin SD gpio3 (RXD0 ) to output data, rest pins SCK gpio15, WS gpio2

//#define USE_FTPSRV    //NOT implemented:USE ftp_server with spiff
// #define USE_SPIFFS   //NOT implemented: plan to move pages to spiff and complie to http use flash or spiff
//should then be a warning-info page, when there is nothing on spiff to serve

//#define USE_SERIAL //comment this to disable serial communications (maybe for fastled?)
#define USE_WPS        //uncomment this for use WPS not tested
#define USE_OTA //uncomment to use OTA httpupdate with link /DEF_XMAS_OTAPATH
//#define USE_MDNS //uncomment to use mDNS service for web DEF_XMAS_HOSTNAME.local

//buttons active low
#define USE_KEY //use gpio0 key to change mode-effect
#define USE_KEY_GPIO 0

//used to enable WPS at boot, prev mode, effect
#define USE_KEYC    
#define USE_KEYC_GPIO   4


#define LEDSTATUS_PIN  2//use buildin led -LED_BUILTIN

#if defined(USE_ADA_NEOPIXEL)
#undef USE_SPITRANSFER
#undef USE_I2STRANSFER
#include <Adafruit_NeoPixel.h>
#elif defined(USE_SPITRANSFER)
#undef USE_NEOPIXELBUS
#undef USE_ADA_NEOPIXEL
#undef USE_I2STRANSFER

#ifndef HSPI
#define HSPI 1
#endif
#elif defined(USE_I2STRANSFER)
#undef USE_ADA_NEOPIXEL
#undef USE_SPITRANSFER
#else
#error Uncomment one method to use: Adafruit, NeopixelBus, SPI, I2S.
#endif


#ifndef USE_ADA_NEOPIXEL
/*  Definitions from the Adafruit NeoPixel library. Need for some functions.*/
#define NEO_RGB 0x06
#define NEO_RBG 0x09
#define NEO_GRB 0x52
#define NEO_GBR 0xA1
#define NEO_BRG 0x58
#define NEO_BGR 0xA4

#endif

#ifdef USE_FTPSRV
#ifndef USE_SPIFFS
#define USE_SPIFFS
#warning Enabled USE_SPIFFS, because of USE_FTPSRV
#endif
#include <ESP8266FtpServer.h>
#endif
#ifdef USE_SPIFFS
#include <spiffs.h>
#warning SPIFF not implemented!
#endif
#define DEF_XMAS_OTAUSER "admin"
#define DEF_XMAS_OTAPASS "admin"
#define DEF_XMAS_OTAPATH "/firmware"
#define DEF_XMAS_HOSTNAME "XMAS_TREE"
#define DEF_XMAS_APNAME "XMAS_TREE_AP"
#ifdef USE_OTA
#define DEF_XMAS_OTAENABLE true
#else
#define DEF_XMAS_OTAENABLE false
#endif
#define DEF_XMAS_LEDCOUNT 100 //
#define DEF_XMAS_LEDPIN 13     //only for adafruits library
#define DEF_XMAS_AMPMAX 500
#define DEF_XMAS_AP_WLANOFF_MS uint32_t(30 * 60 * 1000) //in millis disable AP WIFI after last connection 30min
#ifdef USE_ADA_NEOPIXEL
#define DEF_XMAS_LEDTYPE (NEO_GRB + NEO_KHZ800)
#else
#define DEF_XMAS_LEDTYPE (NEO_RGB)
#endif
//helpful definitions
#define EEPROM_SIZE 250 //should be minimal (buffred in ram) & need to be <= SPI_FLASH_SEC_SIZE (4092)
#define EEPROM_START 1

#define XMAS_VER 10 //change this will clear stored settings = default settings
#define EEPROM_MAGIC 0x0F

//definitions for Xmas.WifiMode
#define WIFI_MODE_AP 0         //default starts in Ap mode
#define WIFI_MODE_TRY 1        //when STA param are entered AND NOT yet TESTED
#define WIFI_MODE_CONNECTING 2 //STA param tested OK, -not used
#define WIFI_MODE_STA 3        //after succesfuly connection to STA

//definitions for WlanStatus
#define WLAN_INIT 0       //
#define WLAN_SCAN_BEGIN 1 //scanning networks
#define WLAN_SCAN_END 2   //end scanning
#define WLAN_STA_TRY1 3   //connecting to ap 1st attempt
#define WLAN_STA_TRY2 4   //connecting to ap 2nd attempt
#define WLAN_STA_TRY3 5   //connecting to ap last attempt (cycle to try2 within timeout period)
#define WLAN_STA_RUN 6    //connected to STA router
#define WLAN_WPS_BEGIN 10 //begin wps connection -pushbutton on router
#define WLAN_WPS_TRY 11
#define WLAN_WPS_END 12
#define WLAN_AP_BEGIN 20    //start AP mode
#define WLAN_AP_RUN 21      //running AP
#define WLAN_AP_WLANOFF 29  //ESP will disable WIFI communication
#define WLAN_ESP_NOWIFI 80  //ESP running with disabled wifi
#define WLAN_ESP_RESTART 99 //ESP will restart

//constats for entering task periods (ms )
#define PERIOD_TASK_WIFI 500 // controlls restart, when loop period exceed
#define PERIOD_TASK_STATUSLED 200
#define PERIOD_TASK_EFFECT 25    //25 for 100led or i2s (50 for 300leds)
#define PERIOD_TASK_BUTTON 200
#define PERIOD_TASK_UPDATEWS 80

#define TASK_WIFI_TRYGIVEUP_MS 20000 //in millis, when attempt to connect to STA, give up after 20sec

#ifdef USE_SERIAL
#define SERIALBEGIN Serial.begin(115200) //here U can change serial speed
#define SERIALPRINTF(x) Serial.print(F(x))
#define SERIALPRINT(x) Serial.print(x)
#define SERIALLN Serial.print(F("/r/n"))
#define SERIALPRINTD(x, y) \
    Serial.print(F(x));    \
    Serial.print(y)
#else
#define SERIALBEGIN
#define SERIALPRINTF(x)
#define SERIALPRINT(x)
#define SERIALLN
#define SERIALPRINTD(x, y)
#endif

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

#define MAX_MIDDLEPOINTS 10
struct struct_xmas_stripe
{
    uint8_t PinNo{};
    uint16_t neoPixelType{};
    uint16_t LedCounts{};
    uint32_t AmperageMax{};
    uint16_t MiddlePoints[MAX_MIDDLEPOINTS]{};
};
struct struct_xmas_config
{
    u8_t Magic = EEPROM_MAGIC;
    u8_t Ver = XMAS_VER;
    char STAname[33]{};
    char STApass[65]{};
    char APname[33]{};
    u8_t WifiMode{};
    u8_t OTAenable{};
    uint32_t APWifiTimeoutMs{};
    uint32_t EffectTimeoutMs{};
    u8_t EffectStartWith{};
    struct_xmas_stripe Stripe1{};
}; //extern xmas_config Xmas;
#endif