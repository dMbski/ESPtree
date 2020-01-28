#include <Arduino.h>
#ifndef ESP8266
#error Tested only on ESP8266!
#error comment/delete these error lines to proceed
#endif
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <EEPROM.h>
#include "helpers.cpp"
#include "ledeffects.h"
#if defined(USE_ADA_NEOPIXEL)
#include <Adafruit_NeoPixel.h>
#elif defined(USE_SPITRANSFER)
#include "espspi_to_ws281x.h"
#elif defined(USE_I2STRANSFER)
#include "espi2s_to_ws281x.h"
//it is from NeoPixel library helper functions for Esp8266.
// (see https://github.com/Makuna/NeoPixelBus) - with some changes
#endif

#ifdef USE_MDNS
#include <ESP8266mDNS.h>
#endif
#ifdef USE_OTA
#include <ESP8266HTTPUpdateServer.h>
#endif
#ifdef USE_FTP
#endif
//#include "pages.cpp"  //conflicts with F macro
//progmem storing pages
extern char page_main[]; //workaround R-- conflicts with Fmacro just
extern char page_configwifi[];
extern char page_infopage[];
extern char page_functions[];
extern char page_restartpage1[];
extern char page_restartpage2[];
extern char page_configtree[];
extern char page_show[];
extern char page_configshow[];
//functions definitions
String EncryptionToStr(uint8_t enctype);
void xmas_resetconfig();
void xmas_writeconfig(u8 wifimode);
#ifdef USE_SERIAL
void xmas_printcfg(struct_xmas_config *cfg);
#endif
void servers_start();
void servers_stop();
void servers_config();
void servers_handleWS(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght);
void Task_Wifi();
void Task_StatusLED();
void Task_Effect();
void Task_BufferToStrip();
void Task_UpdateWS();
#if defined(USE_KEY) || defined(USE_KEYC)
void Task_Button();
#endif

void NextEffect();
void PrevEffect();
String NeoPixeltypeToStr(uint16_t pt);
//configuration, check in helpers.cpp

struct_xmas_config Xmas;
ESP8266WebServer ServerHTML(SERVER_HTML_PORT);
WebSocketsServer ServerWS(SERVER_WS_PORT);
#ifdef USE_OTA
ESP8266HTTPUpdateServer ServerHttpOTA;
#endif

#if defined(USE_ADA_NEOPIXEL)
Adafruit_NeoPixel *ALEDStrip1;
#elif defined(USE_SPITRANSFER)
void TaskUpdateSPI();
#elif defined(USE_I2STRANSFER)
void TaskUpdateI2S();
#endif

u8_t LEDStripIsDirty = false;

un_color32 *PixBuffer;
uint16_t PixBuffCount;
u8_t PixBuffIsDirty = true;

String StrBuffer1 = ""; //just buffers to deal with html
String StrBuffer2 = "";

u8_t WlanStatus = WLAN_INIT; //actually what to do with wifi
u32_t Enter_TaskWifi = PERIOD_TASK_WIFI;
u32_t Enter_TaskStatusLed = PERIOD_TASK_STATUSLED;
u32_t Enter_TaskEffect = PERIOD_TASK_EFFECT;
u32_t Enter_TaskUpdateWS = PERIOD_TASK_UPDATEWS;
bool TaskUpdateWSsendLEDs = false;
bool TaskUpdateWSsendData = false;
#ifdef USE_KEY
u32_t Enter_TaskButton = PERIOD_TASK_BUTTON;
#endif
u32_t TaskEffectChange = 0;
u8_t TaskEffectId = 0;
u32_t TaskEffectProcTimeMax = 0;
u32_t TaskEffectprocTime = PERIOD_TASK_UPDATEWS;
u32_t TaskEffectUpdTimeMax = 0;
u32_t TaskEffectPowerNeed = 0;
u32_t TaskEffectPFStrip1 = 100;
u32_t TaskWifiConnectTimeout = TASK_WIFI_TRYGIVEUP_MS;
u32_t TaskWifiAPTimeout = DEF_XMAS_AP_WLANOFF_MS;
u8_t LedStatus = 0; //just for small light-heartbeat

void setup()
{

  SERIALBEGIN;
  SERIALPRINTF("\r\nStart...");

  WiFi.persistent(false); //dont store wifi config in sdk flash, we'll take of that
  pinMode(LEDSTATUS_PIN, OUTPUT);
#ifdef USE_KEY
  pinMode(USE_KEY_GPIO, INPUT);
#endif
#ifdef USE_KEYC
  pinMode(USE_KEYC_GPIO, INPUT);
#endif

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_START, Xmas); //loads config from eeporm
  SERIALPRINTF("\r\nRead & check config from eeprom...");
  if (Xmas.Magic != EEPROM_MAGIC || Xmas.Ver != XMAS_VER)
  {
    SERIALPRINTF("\r\nMagic or ver are different, reset config to default...");
    xmas_resetconfig();
  }
#ifdef USE_SERIAL
  xmas_printcfg(&Xmas);
#endif
  servers_config();
  PixBuffCount = Xmas.Stripe1.LedCounts; //all leds from all strips

#if defined(USE_ADA_NEOPIXEL)
  ALEDStrip1 = new Adafruit_NeoPixel(uint16_t(Xmas.Stripe1.LedCounts), uint8_t(Xmas.Stripe1.PinNo), neoPixelType(Xmas.Stripe1.neoPixelType));
  ALEDStrip1->begin();
  ALEDStrip1->clear();
  ALEDStrip1->show();
#elif defined(USE_SPITRANSFER)
  prepareHSPI();
#elif defined(USE_I2STRANSFER)
  prepareI2S(PixBuffCount);
#endif

  PixBuffer = new un_color32[PixBuffCount];
  PixFill(0);

  TaskEffectChange = millis() + Xmas.EffectTimeoutMs;
  TaskEffectId = Xmas.EffectStartWith;
  SERIALPRINTD("\r\n_____Setup ends. Free ram:", ESP.getFreeHeap());
}

void loop()
{
  u32_t ttime = millis();
  if (PixBuffIsDirty) //buffer to neopixel
  {
    Task_BufferToStrip();
  };
  if (LEDStripIsDirty) //update neopixel strip
  {
#if defined(USE_ADA_NEOPIXEL)
    ALEDStrip1->show();
#elif defined(USE_SPITRANSFER)
    TaskUpdateSPI();
#elif defined(USE_I2STRANSFER)
    TaskUpdateI2S();
#endif
    LEDStripIsDirty = false;
  };
  ttime = millis() - ttime;
  if (ttime > TaskEffectUpdTimeMax)
  {
    TaskEffectUpdTimeMax = ttime;
    SERIALPRINTD("\r\nMax Update Strip time:", TaskEffectUpdTimeMax);
  }
  if (millis() > Enter_TaskStatusLed) //update status led
  {
    Task_StatusLED();

    if (!LedStatus)
      Enter_TaskStatusLed = millis() + ((WlanStatus * PERIOD_TASK_STATUSLED));
    else
      Enter_TaskStatusLed = millis() + PERIOD_TASK_STATUSLED;
  };
  if (millis() > Enter_TaskWifi) //process wifi changes
  {
    Task_Wifi();
    Enter_TaskWifi = millis() + PERIOD_TASK_WIFI;
  };
#if defined(USE_KEY) || defined(USE_KEYC)
  if (millis() > Enter_TaskButton)
  {
    Task_Button();
    Enter_TaskButton = millis() + PERIOD_TASK_BUTTON;
  }
#endif
  if (Xmas.EffectTimeoutMs > 0)
  {
    if (millis() > TaskEffectChange)
    {
      NextEffect();
    }
  };
  ttime = millis();
  if (millis() > Enter_TaskEffect) //update effect
  {
    Task_Effect();
    Enter_TaskEffect = millis() + PERIOD_TASK_EFFECT;
  };
  if ((millis() > Enter_TaskUpdateWS) && (WlanStatus < WLAN_ESP_NOWIFI))
  {
    Task_UpdateWS();
    Enter_TaskUpdateWS = millis() + PERIOD_TASK_UPDATEWS;
    TaskEffectprocTime = ((millis() - ttime) + TaskEffectprocTime) / 2 + TaskEffectUpdTimeMax;
  }
  ttime = millis() - ttime;
  if (ttime > TaskEffectProcTimeMax)
  {
    if (TaskEffectProcTimeMax > (PERIOD_TASK_WIFI * 4))
    { //something is wrong, restart
      WlanStatus = WLAN_ESP_RESTART;
      SERIALPRINTD("\r\n----Restart, exceed max proccessing time (twice), ms:", ttime);
    }
    TaskEffectProcTimeMax = ttime;
    TaskEffectprocTime = (TaskEffectProcTimeMax + TaskEffectprocTime) / 2 + TaskEffectUpdTimeMax;
    SERIALPRINTD("\r\nMax Effect processing time:", TaskEffectProcTimeMax);
  }
  if (WlanStatus < WLAN_ESP_NOWIFI)
  {
    ServerHTML.handleClient();
    ServerWS.loop();
#ifdef USE_MDNS
    MDNS.update();
#endif
  }
}

//------------------------------
//------------------------------
void xmas_resetconfig()
{
  SERIALPRINTF("\r\nResetting config...");
  PGM_P defapname = PSTR(DEF_XMAS_APNAME);
  memset(&Xmas, 0, sizeof(Xmas)); //zeroing
  Xmas.Magic = EEPROM_MAGIC;
  Xmas.Ver = XMAS_VER;
  strcpy_P(Xmas.APname, defapname);
  Xmas.WifiMode = WIFI_MODE_AP;
  Xmas.APWifiTimeoutMs = DEF_XMAS_AP_WLANOFF_MS;
  Xmas.OTAenable = DEF_XMAS_OTAENABLE;
  Xmas.Stripe1.PinNo = DEF_XMAS_LEDPIN;
  Xmas.Stripe1.neoPixelType = DEF_XMAS_LEDTYPE;
  Xmas.Stripe1.AmperageMax = DEF_XMAS_AMPMAX;
  Xmas.Stripe1.LedCounts = DEF_XMAS_LEDCOUNT;
  for (uint16_t i = 0; i < MAX_MIDDLEPOINTS; i++)
  {
    Xmas.Stripe1.MiddlePoints[i] = (Xmas.Stripe1.LedCounts / MAX_MIDDLEPOINTS) / 2 + i * (Xmas.Stripe1.LedCounts / MAX_MIDDLEPOINTS);
    if (Xmas.Stripe1.MiddlePoints[i] >= Xmas.Stripe1.LedCounts)
      Xmas.Stripe1.MiddlePoints[i] = Xmas.Stripe1.LedCounts - 1;
  }
}
//-------------------------------
void xmas_writeconfig(u8 wifimode)
{
  Xmas.WifiMode = wifimode;
  EEPROM.put(EEPROM_START, Xmas);
  EEPROM.commit();
  SERIALPRINTD("\r\nEEPROM write with wifimode=", wifimode);
}
//------------
String NeoPixeltypeToStr(uint16_t pt)
{
  if (pt == NEO_RGB)
    return F("NEO_RGB");
  if (pt == NEO_RBG)
    return F("NEO_RBG");
  if (pt == NEO_GRB)
    return F("NEO_GRB");
  if (pt == NEO_GBR)
    return F("NEO_GBR");
  if (pt == NEO_BRG)
    return F("NEO_BRG");
  if (pt == NEO_BGR)
    return F("NEO_BGR");
  return String(pt);
}
//------------
#ifdef USE_SERIAL
void xmas_printcfg(struct_xmas_config *cfg)
{
  SERIALPRINTF("\r\nConfig details:");
  SERIALPRINTD("\r\nConfig Size:", long(sizeof(struct_xmas_config))); //without long compiles but shows error overri
  SERIALPRINTD("\r\nMagic:", cfg->Magic);
  SERIALPRINTD("\r\nVer:", cfg->Ver);
  SERIALPRINTD("\r\nSTAname(router's SSID):", cfg->STAname);
  SERIALPRINTD("\r\nSTApass(router's pass):", cfg->STApass);
  SERIALPRINTD("\r\nAPname(standalone ap):", cfg->APname);
  SERIALPRINTD("\r\nWifiMode:", cfg->WifiMode);
  SERIALPRINTF("  (0-WIFI_MODE_AP 1-WIFI_MODE_TRY 2-WIFI_MODE_CONNECTING 3-WIFI_MODE_STA)");
  SERIALPRINTD("\r\nOTAenable:", cfg->OTAenable);
  SERIALPRINTD("\r\nAPmode disable Wifi (sec):", cfg->APWifiTimeoutMs / 1000);
  SERIALPRINTD("\r\nAutochange Effect (sec):", cfg->EffectTimeoutMs / 1000);
  SERIALPRINTD("\r\nDefault Effect:", cfg->EffectStartWith);
  SERIALPRINTD("\r\nStripe1.Ledcounts:", cfg->Stripe1.LedCounts);
  SERIALPRINTD("\r\nStripe1.AmperageMax(mA):", cfg->Stripe1.AmperageMax);
  SERIALPRINTD("\r\nStripe1.PinNo:", cfg->Stripe1.PinNo);
  SERIALPRINTD("\r\nStripe1.neoPixelType:", NeoPixeltypeToStr(cfg->Stripe1.neoPixelType));
  //TODO: add middlepoints
#if defined(USE_SPITRANSFER)
  SERIALPRINTF("\r\nUse SPI. Check source for details");
  SERIALPRINTF("\r\nUsing pin MOSI-GPIO13 for dataout.");
#elif defined(USE_I2STRANSFER)
  SERIALPRINTF("\r\nUse i2s DMA. Check source for details");
  SERIALPRINTF("\r\nUsing pin RX-GPIO3 for dataout.");
#elif defined(USE_ADA_NEOPIXEL)
  SERIALPRINTF("\r\nUse Adafruit NeoPixel library.");
#endif
}
#endif
//----------------
#ifdef USE_SPITRANSFER
//------------------
void TaskUpdateSPI()
{
  uint32_t powneed;
  powneed = sendSPI32(PixBuffer, PixBuffCount, Xmas.Stripe1.neoPixelType, TaskEffectPFStrip1);

  if (powneed == 0)
    TaskEffectPFStrip1 = 100;
  else if (powneed > Xmas.Stripe1.AmperageMax)
  {
    TaskEffectPFStrip1 = (Xmas.Stripe1.AmperageMax * 100) / powneed;
  }
  else
  {
    TaskEffectPFStrip1 = 100;
  };

  TaskEffectPowerNeed = (powneed + TaskEffectPowerNeed) / 2;
}
#endif

#ifdef USE_I2STRANSFER
void TaskUpdateI2S()
{
  uint32_t powneed;
  powneed = sendI2S(PixBuffer, PixBuffCount, Xmas.Stripe1.neoPixelType, TaskEffectPFStrip1);

  if (powneed == 0)
    TaskEffectPFStrip1 = 100;
  else if (powneed > Xmas.Stripe1.AmperageMax)
  {
    TaskEffectPFStrip1 = (Xmas.Stripe1.AmperageMax * 100) / powneed;
  }
  else
  {
    TaskEffectPFStrip1 = 100;
  };

  TaskEffectPowerNeed = (powneed + TaskEffectPowerNeed) / 2;
}
#endif

//----------------
void Task_BufferToStrip()
{
//calculations for power are done here only for ada
//sendspi, sendi2s returns power needed
#if defined(USE_ADA_NEOPIXEL)
  un_color32 cc;
  uint32_t allpix = 0;
  for (uint16_t i = 0; i < PixBuffCount; i++)
  {
    cc = PixBuffer[i];
    allpix = allpix + cc.c8.r + cc.c8.g + cc.c8.b;

    if (TaskEffectPFStrip1 < 100)
    {
      cc.c8.r = (cc.c8.r * TaskEffectPFStrip1) / 100;
      cc.c8.g = (cc.c8.g * TaskEffectPFStrip1) / 100;
      cc.c8.b = (cc.c8.b * TaskEffectPFStrip1) / 100;
    };

    ALEDStrip1->setPixelColor(i, cc.c8.r, cc.c8.g, cc.c8.b);
    if (!(i % 100))
      yield();
  }
  //calc PowerFactor
  allpix = (allpix * 10) / 127;
  if (allpix == 0)
    TaskEffectPFStrip1 = 100;
  else if (allpix > Xmas.Stripe1.AmperageMax)
  {
    TaskEffectPFStrip1 = (Xmas.Stripe1.AmperageMax * 100) / allpix;
  }
  else
  {
    TaskEffectPFStrip1 = 100;
  };

  TaskEffectPowerNeed = (allpix + TaskEffectPowerNeed) / 2;
#endif
  PixBuffIsDirty = false;
  LEDStripIsDirty = true;
};
//----------------
void NextEffect()
{
  TaskEffectId++;
  if (TaskEffectId >= EFFECTS_COUNT)
    TaskEffectId = 0;
  TaskEffectChange = millis() + Xmas.EffectTimeoutMs;
  SERIALPRINTD("\r\nNext effect no:", TaskEffectId);
}
//-----------------
void PrevEffect()
{
  if (TaskEffectId >= 1)

    TaskEffectId--;
  else
    TaskEffectId = EFFECTS_COUNT - 1;
  TaskEffectChange = millis() + Xmas.EffectTimeoutMs;
  SERIALPRINTD("\r\nPrev effect no:", TaskEffectId);
}

//---------------
void ChangeEffect(u8_t effid)
{
  if (effid >= EFFECTS_COUNT)
    return;
  TaskEffectId = effid;
  TaskEffectChange = millis() + Xmas.EffectTimeoutMs;
  SERIALPRINTD("\r\nChanged to effect no:", TaskEffectId);
}
//----------------------------------------------
void toHEX(char *buff, uint8_t val)
{
  const char h[] = "0123456789ABCDEF";
  buff[0] = h[(val >> 4) & 0xF];
  buff[1] = h[val & 0xF];
}
void Task_UpdateWS()
{
  static uint8_t tfase = 0;
  static uint16_t ledno = 0;
  static uint16_t ledpack = 1;
  char b[] = "*000000";
  if (ledno >= PixBuffCount)
  {
    ledno = 0;
    tfase = 0;
    TaskUpdateWSsendLEDs = false;
  }
  if (TaskUpdateWSsendLEDs)
    tfase = 7;
  if (tfase == 0)
  {
    StrBuffer1 = "!#";
    StrBuffer1.concat(TaskEffectId);
  }
  if (tfase == 1 && TaskUpdateWSsendData)
  {
    StrBuffer1 = "!f";
    StrBuffer1.concat(TaskEffectPFStrip1);
  }
  if (tfase == 2 && TaskUpdateWSsendData)
  {
    StrBuffer1 = "!p";
    StrBuffer1.concat(TaskEffectPowerNeed);
  }
  if (tfase == 3 && TaskUpdateWSsendData)
  {
    StrBuffer1 = "!L";
    StrBuffer1.concat(PixBuffCount);
  }
  if (tfase == 4 && TaskUpdateWSsendData)
  {
    StrBuffer1 = "!t";
    StrBuffer1.concat(TaskEffectprocTime);
  }
  if (tfase == 5 && TaskUpdateWSsendData)
  {
    StrBuffer1 = "!N";
    StrBuffer1.concat(ledno);
  }
  if (tfase == 6 && TaskUpdateWSsendData)
  {
    StrBuffer1 = "!r";
    StrBuffer1.concat((millis() / 1000));
  }
  if (tfase == 7 && TaskUpdateWSsendLEDs)
  {
    if (TaskEffectprocTime > PERIOD_TASK_UPDATEWS)
    {
      ledpack = 1;
      tfase = 0;
      SERIALPRINTD("\r\nPERIOD_UPDATEWS exceed:", TaskEffectprocTime);
      return;
    }
    else if (TaskEffectprocTime > PERIOD_TASK_UPDATEWS / 2)
    {
      ledpack = 1;
    }
    else if (TaskEffectprocTime < PERIOD_TASK_UPDATEWS / 2)
    {
      ledpack++;
    }
    if (ledpack > 10)
      ledpack = 10;
    un_color32 c;
    for (uint16_t i = 0; i < ledpack; i++)
    {
      c = PixGet(ledno);
      toHEX(&b[1], c.c8.r);
      toHEX(&b[3], c.c8.g);
      toHEX(&b[5], c.c8.b);
      ServerWS.broadcastTXT(&b[0], 7);
      ledno++;
      yield();
      if (ledno >= PixBuffCount)
      {
        ledno = PixBuffCount;
        return;
      };
    };
  }
  else
  {
    tfase++;
    if (StrBuffer1.length() > 0)
    {
      ServerWS.broadcastTXT(StrBuffer1);
      StrBuffer1 = "";
    }
  }
  if (tfase > 7)
    tfase = 0;
}
//---------------
void Task_Effect()
{
  PixEffect(TaskEffectId);
  PixBuffIsDirty = true;
}
//------------------
void Task_StatusLED()
{

  digitalWrite(LEDSTATUS_PIN, LedStatus);
  LedStatus = !LedStatus;
}
//-------------
#if defined(USE_KEY) || defined(USE_KEYC)
void Task_Button()
{
  if ((WlanStatus < WLAN_STA_RUN) || ((WlanStatus >= WLAN_WPS_BEGIN) && (WlanStatus <= WLAN_WPS_END)))
    return; //dont use when initiate
  uint8_t ast;
#ifdef USE_KEY
  static uint8_t lst0;
  ast = digitalRead(USE_KEY_GPIO);
  if (ast == lst0)
  {
    ast = !ast;
    if (ast)
    {
      NextEffect();
      PixFill(COL_BLACK);
    }
  }
  lst0 = ast;
#endif
#ifdef USE_KEYC
  static uint8_t lst1;
  ast = digitalRead(USE_KEYC_GPIO);
  if (ast == lst1)
  {
    ast = !ast;
    if (ast)
    {
      PrevEffect();
      PixFill(COL_WHITE);
    }
  }
  lst1 = ast;
#endif
}
#endif
//-------------
void Task_Wifi()
{
  int8_t vstatus;
  switch (WlanStatus)
  {
  case WLAN_INIT:
    SERIALPRINTF("\r\nTaskWifi: WLAN_INIT");
#if defined(USE_KEYC) && defined(USE_WPS)
    if (!digitalRead(USE_KEYC_GPIO))
    {
      SERIALPRINTF("\r\nTaskWifi: Used button, entering WPS");
      WlanStatus = WLAN_WPS_BEGIN;
      break;
    }
#endif
    if (WiFi.getMode() != WIFI_STA)
    {
      SERIALPRINTF("\r\nChange mode to WIFI_STA...");
      WiFi.mode(WIFI_STA);
      break;
    }
    else
    {
      SERIALPRINTF("\r\nDisconecting...");
      WiFi.disconnect();
      if (WiFi.status() == WL_CONNECTED)
        break;
    }
    WiFi.scanNetworks(true, false);
    WlanStatus = WLAN_SCAN_BEGIN;
    SERIALPRINTF("\r\nChange to: WLAN_SCAN_BEGIN");
    break;
  case WLAN_SCAN_BEGIN:
    vstatus = WiFi.scanComplete();
    if (vstatus < 0)
    {
      SERIALPRINTD("\r\nScanning in progress:", vstatus);
    }
    else
    {
      WlanStatus = WLAN_SCAN_END;
    }
    break;
  case WLAN_SCAN_END:
    vstatus = WiFi.scanComplete();
    SERIALPRINTD("\r\nTaskWifi: WLAN_SCAN_END, discovered networks:", vstatus);
    for (int8_t i = 0; i < vstatus; i++)
    {
      SERIALPRINTD("\r\nNo:", i);
      SERIALPRINTD(" : SSID:", WiFi.SSID(i));
      SERIALPRINTD(" : RSSI:", WiFi.RSSI(i));
      SERIALPRINTD(" : Auth:", EncryptionToStr(WiFi.encryptionType(i)));
    }
    SERIALPRINTD("\r\nFree ram:", ESP.getFreeHeap());
    if (Xmas.WifiMode > WIFI_MODE_AP)
    {
      WlanStatus = WLAN_STA_TRY1;
      TaskWifiConnectTimeout = millis() + TASK_WIFI_TRYGIVEUP_MS;
    }
    else
    {
      WlanStatus = WLAN_AP_BEGIN;
    }
    break;
  case WLAN_STA_TRY1:
    SERIALPRINTD("\r\nTaskWifi: WLAN_STA_TRY1, connecting to:", Xmas.STAname);
    WiFi.mode(WIFI_STA);
    WiFi.begin(Xmas.STAname, Xmas.STApass);
    WlanStatus = WLAN_STA_TRY2;
    break;
  case WLAN_STA_TRY2:
    SERIALPRINTF("\r\nTaskWifi: WLAN_STA_TRY2, connecting...");
    WlanStatus = WLAN_STA_TRY3;
    break;
  case WLAN_STA_TRY3:
    SERIALPRINTF("\r\nTaskWifi: WLAN_STA_TRY3.");
    if (WiFi.status() != WL_CONNECTED)
    {
      if (millis() < TaskWifiConnectTimeout)
      {
        WlanStatus = WLAN_STA_TRY2;
      }
      else
      {
        SERIALPRINTF("\r\nTaskWifi: WLAN_STA_TRY3. Time is up, not connected");
        WlanStatus = WLAN_AP_BEGIN;
      }
    }
    else
    {
      SERIALPRINTF("\r\nTaskWifi: WLAN_STA_TRY3. Connected.");
      SERIALPRINTD("\r\n.....my IP adress:", WiFi.localIP());
      SERIALPRINTD("\r\ngateway IP adress:", WiFi.gatewayIP());
      SERIALPRINTD("\r\n.............mask:", WiFi.subnetMask());
      WlanStatus = WLAN_STA_RUN;
      if (Xmas.WifiMode < WIFI_MODE_STA)
      {
        Xmas.WifiMode = WIFI_MODE_STA;
        xmas_writeconfig(WIFI_MODE_STA);
      }
      //initiate servers here
      servers_start(); //starting servers
    }
    break;
  case WLAN_STA_RUN:
    //SERIALPRINTF("\r\nTaskWifi: WLAN_STA_RUN.");
    if (WiFi.status() != WL_CONNECTED)
    {
      SERIALPRINTD("\r\nTaskWifi: WLAN_STA_RUN. Dropped connection, status:", WiFi.status());
      WlanStatus = WLAN_STA_TRY1;
      SERIALPRINTF("\r\nChange to: WLAN_STA_TRY1");
    }
    //maybe some housekeeping
    break;
#ifdef USE_WPS
  case WLAN_WPS_BEGIN:
    SERIALPRINTF("\r\nTaskWifi: WLAN_WPS_BEGIN.");
    servers_stop();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (WiFi.status() == WL_CONNECTED)
      break;
    TaskWifiConnectTimeout = millis() + (3 * TASK_WIFI_TRYGIVEUP_MS);
    WiFi.beginWPSConfig();
    WlanStatus = WLAN_WPS_TRY;
    break;
  case WLAN_WPS_TRY:
    SERIALPRINTF("\r\nTaskWifi: WLAN_WPS_TRY. Connecting");
    if (WiFi.status() != WL_CONNECTED)
    {
      SERIALPRINTF(".");
      if (millis() > TaskWifiConnectTimeout)
      {
        SERIALPRINTF("\r\nTaskWifi: WLAN_WPS_TRY. Time is up, not connected.");
        WlanStatus = WLAN_AP_BEGIN;
      };
    }
    else
    {
      SERIALPRINTF("WPS Success!.");
      WlanStatus = WLAN_WPS_END;
    }
    break;
  case WLAN_WPS_END:
    SERIALPRINTF("\r\nTaskWifi: WLAN_WPS_END. Connected.");
    SERIALPRINTD("\r\n.....my IP adress:", WiFi.localIP());
    WlanStatus = WLAN_STA_RUN;
    strcpy(Xmas.STAname, WiFi.SSID().c_str());
    strcpy(Xmas.STApass, WiFi.psk().c_str());
    Xmas.WifiMode = WIFI_MODE_STA;
    xmas_writeconfig(WIFI_MODE_STA);
#ifdef USE_SERIAL
    xmas_printcfg(&Xmas);
#endif
    servers_start();
    break;
#endif
  case WLAN_AP_BEGIN:
    SERIALPRINTF("\r\nTaskWifi: WLAN_AP_BEGIN. Creating AP...");
    TaskWifiAPTimeout = millis() + Xmas.APWifiTimeoutMs;
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    if (WiFi.softAP(Xmas.APname))
    {
      SERIALPRINTD("\r\nSuccess, AP name:", Xmas.APname);
      SERIALPRINTD("\r\n.....my IP adress:", WiFi.softAPIP());
      WlanStatus = WLAN_AP_RUN;
      servers_start(); //starting servers
    }
    else
      SERIALPRINTF("\r\nFailed creating AP.");
    break;
  case WLAN_AP_RUN:
    //maybe some housekeeping
    if (WiFi.softAPgetStationNum() > 0)
    {
      TaskWifiAPTimeout = millis() + Xmas.APWifiTimeoutMs;
    }
    if (millis() > TaskWifiAPTimeout)
      WlanStatus = WLAN_AP_WLANOFF;
    break;
  case WLAN_AP_WLANOFF:
    SERIALPRINTF("\r\nTaskWifi: WLAN_AP_WLANOFF...disable Wifi.");
    servers_stop();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    SERIALPRINTF("\r\nChange to: WLAN_ESP_NOWIFI");
    WlanStatus = WLAN_ESP_NOWIFI;
    break;
  case WLAN_ESP_NOWIFI:
    //run without wifi
    SERIALPRINTD("\r\nFree ram:", ESP.getFreeHeap());
    break;
  case WLAN_ESP_RESTART:
    SERIALPRINTF("\r\nTaskWifi: WLAN_ESP_RESTART...");
    ESP.restart();
  }
}
//---------------
String EncryptionToStr(uint8_t enctype)
{
  if (enctype == ENC_TYPE_NONE)
    return F("OPEN");
  if (enctype == ENC_TYPE_AUTO)
    return F("WPAauto");
  if (enctype == ENC_TYPE_CCMP)
    return F("WPA2psk");
  if (enctype == ENC_TYPE_WEP)
    return F("WEP");
  if (enctype == ENC_TYPE_TKIP)
    return F("WPApsk");
  return F("unknown");
}
//--------
void servers_stop()
{
  SERIALPRINTF("\r\nStoping servers...");
  ServerHTML.close();
  ServerWS.close();
}
//---------------
void servers_start() //starting servers
{
  SERIALPRINTF("\r\nStarting servers...");
  SERIALPRINTF("\r\nHTML port:80   WEBsockets port:81");
  ServerHTML.begin();
  ServerWS.begin();
  ServerWS.onEvent(servers_handleWS);
#ifdef USE_MDNS
  SERIALPRINTF("\r\nBegin mDNS responder with http, ws.");
  MDNS.begin(DEF_XMAS_HOSTNAME);
  MDNS.addService("http", "tcp", SERVER_HTML_PORT);
  MDNS.addService("ws", "tcp", SERVER_WS_PORT);
#endif
}
//---------------
void servers_handleWS(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght)
{
  SERIALPRINTD("\r\nHandle WS, lenght: ", lenght);
  if (type == WStype_TEXT)
  {
    if (lenght < 2)
      return;
    if (payload[0] == '!')
    { //maybe some commands
      if (payload[1] == '#')
      { //change effect !#number
        ChangeEffect(strtol((const char *)&payload[2], NULL, 10));
      }
      if (payload[1] == 'p')
      { //preview leds  !p
        TaskUpdateWSsendLEDs = true;
      }
      if (payload[1] == '!')
      { //enables sending data !!
        TaskUpdateWSsendData = true;
      }
      if (payload[1] == 's')
      { //pause sending data !!
        TaskUpdateWSsendData = false;
      }
    }
    ServerWS.broadcastTXT(payload, lenght);
  }
}
//---------------
void servers_handleNotFound()
{
  SERIALPRINTD("\r\nHandle ServerHtml.notfound: ", ServerHTML.uri());
  StrBuffer1 = F("ERROR 404, Not Found: ");
  StrBuffer1 = StrBuffer1 + ServerHTML.uri();
  StrBuffer1 = StrBuffer1 + F("\nArguments:");
  for (uint8_t i = 0; i < ServerHTML.args(); i++)
  {
    StrBuffer1 = StrBuffer1 + F("\nName: ") + ServerHTML.argName(i) + F(" Val: ") + ServerHTML.arg(i);
  };
  SERIALPRINTD("\r\nSending response:\r\n", StrBuffer1);
  ServerHTML.send(404, "text/plain", StrBuffer1);
  StrBuffer1 = "";
}
//--------
void servers_handleWifiListString()
{
  SERIALPRINTF("\r\nHandle WifiListString.");
  StrBuffer1 = "";
  const char fieldd = ';';
  for (uint8_t i = 0; i < WiFi.scanComplete(); i++)
  {
    StrBuffer1.concat(F("{"));
    StrBuffer1.concat(WiFi.SSID(i));
    StrBuffer1.concat(fieldd);
    StrBuffer1.concat(WiFi.RSSI(i));
    StrBuffer1.concat(fieldd);
    StrBuffer1.concat(EncryptionToStr(WiFi.encryptionType(i)));
    StrBuffer1.concat(F("}"));
  }
  SERIALPRINTD("\r\nSending bytes: ", StrBuffer1.length());
  ServerHTML.send(200, "text/html", StrBuffer1);
  SERIALPRINTD("\r\nSend:", StrBuffer1);
  StrBuffer1 = "";
}
//--------
void servers_handleCfgWifiSave()
{
  SERIALPRINTF("\r\nHandle cfgwifisave with args:");
  boolean ch = false;
  u8_t wlmode = WIFI_MODE_TRY;
  for (uint8_t i = 0; i < ServerHTML.args(); i++)
  {
    SERIALPRINTD("\r\nName: ", ServerHTML.argName(i));
    SERIALPRINTD(" =", ServerHTML.arg(i));
    if (ServerHTML.argName(i) == "staname")
    {
      strcpy(Xmas.STAname, ServerHTML.arg(i).c_str());
      ch = true;
    }
    if (ServerHTML.argName(i) == "stapass")
    {
      strcpy(Xmas.STApass, ServerHTML.arg(i).c_str());
      ch = true;
    }
    if (ServerHTML.argName(i) == "wlanmode")
    {
      if (ServerHTML.arg(i) == "AP")
        wlmode = WIFI_MODE_AP;
      ch = true;
    }
  };
  if (ch)
  {
    xmas_writeconfig(wlmode);
    ServerHTML.send_P(200, "text/html", page_restartpage1);
  }
  else
    ServerHTML.send(200, "text/plain", F("Not Saved!"));
}
//--------
String WifiModeToStr(u8_t wifimode)
{
  if (wifimode == WIFI_MODE_AP)
    return F("Standalone AP!");
  if (wifimode == WIFI_MODE_TRY)
    return F("STA first try!");
  if (wifimode > WIFI_MODE_TRY)
    return F("Connect to STA");
  return (F("Unknown mode: "));
}
//----------
String IPToStr(uint32_t addr)
{
  String ret = "";
  const char dot = '.';
  ret = addr & 0x000000ff;
  ret = ret + dot;
  ret = ret + ((addr & 0x0000ff00) >> 8);
  ret = ret + dot;
  ret = ret + ((addr & 0x00ff0000) >> 16);
  ret = ret + dot;
  ret = ret + ((addr & 0xff000000) >> 24);
  return ret;
}
//----------
void servers_handleInfoString()
{
  SERIALPRINTF("\r\nHandle infostring.");
  StrBuffer1 = "";
  StrBuffer1.concat(F("{Chip Id:;"));
  StrBuffer1.concat(String(ESP.getChipId(), HEX));
  StrBuffer1.concat(F(";}{Cpu Freq:;"));
  StrBuffer1.concat(ESP.getCpuFreqMHz());
  StrBuffer1.concat(F(";Mhz}{Flash Size:;"));
  StrBuffer1.concat(ESP.getFlashChipSize());
  StrBuffer1.concat(F(";bytes}{Flash Size Real:;"));
  StrBuffer1.concat(ESP.getFlashChipRealSize());
  StrBuffer1.concat(F(";bytes}{Sketch Size:;"));
  StrBuffer1.concat(ESP.getSketchSize());
  StrBuffer1.concat(F(";bytes}{Sketch Free Space:;"));
  StrBuffer1.concat(ESP.getFreeSketchSpace());
  StrBuffer1.concat(F(";bytes}{Free Heap:;"));
  StrBuffer1.concat(ESP.getFreeHeap());
  StrBuffer1.concat(F(";bytes}{Heap Fragmentation:;"));
  StrBuffer1.concat(F(";not supported}{Last Reset Reason:;"));
  StrBuffer1.concat(ESP.getResetReason());
  StrBuffer1.concat(F(";}{STA SSID to connect:;<b>"));
  StrBuffer1.concat(Xmas.STAname);
  StrBuffer1.concat(F("</b>;}{Standalone AP SSID:;"));
  StrBuffer1.concat(Xmas.APname);
  StrBuffer1.concat(F(";}"));
  //stripeinfo
#if defined(USE_ADA_NEOPIXEL)
  StrBuffer1.concat(F("{Use Adafruit NeoPixel;;}"));
#elif defined(USE_SPITRANSFER)
  StrBuffer1.concat(F("{Use SPI transfer;Pin SPI-MOSI;GPIO13}"));
#elif defined(USE_SPITRANSFER)
  StrBuffer1.concat(F("{Use i2s transfer;Pin i2s-sdo;GPIO3}"));
#endif
  StrBuffer1.concat(F("{Max Effect Processing Time:;"));
  StrBuffer1.concat(TaskEffectProcTimeMax);
  StrBuffer1.concat(F(";msec}{Max Update Strip Time:;"));
  StrBuffer1.concat(TaskEffectUpdTimeMax);
  StrBuffer1.concat(F(";msec}{WebSockets Port:;"));
  StrBuffer1.concat(SERVER_WS_PORT);
  StrBuffer1.concat(F(";}{IP config:;IP: "));
  //info about network
  if (WiFi.getMode() == WIFI_AP)
    StrBuffer1.concat(IPToStr(WiFi.softAPIP()));
  else
    StrBuffer1.concat(IPToStr(WiFi.localIP()));
  StrBuffer1.concat(F("<br>Gateway: "));
  StrBuffer1.concat(IPToStr(WiFi.gatewayIP()));
  StrBuffer1.concat(F("<br>Mask: "));
  StrBuffer1.concat(IPToStr(WiFi.subnetMask()));
  StrBuffer1.concat(F(";}"));
  SERIALPRINTD("\r\nSending bytes: ", StrBuffer1.length());
  ServerHTML.send(200, "text/plain", StrBuffer1);
  SERIALPRINTD("\r\nSend:", StrBuffer1);
  StrBuffer1 = "";
}
//--------
void servers_handleEffListString()
{
  SERIALPRINTF("\r\nHandle efflist.");
  StrBuffer1 = "";
  for (int i = 0; i < EFFECTS_COUNT; i++)
  {
    StrBuffer1.concat(F("{"));
    StrBuffer1.concat(i + 1);
    StrBuffer1.concat(F(";R;"));
    StrBuffer1.concat(i);
    StrBuffer1.concat(F(";radeff;"));
    StrBuffer1.concat(EffectName(i));
    StrBuffer1.concat(";}");
  }

  SERIALPRINTD("\r\nSending bytes: ", StrBuffer1.length());
  ServerHTML.send(200, "text/plain", StrBuffer1);
  SERIALPRINTD("\r\nSend:", StrBuffer1);
  StrBuffer1 = "";
}
//--------
void servers_handleConfigShowString()
{
  SERIALPRINTF("\r\nHandle ConfigShowString."); //-----------------
  StrBuffer1 = "{Number of LEDs;T;";
  StrBuffer1.concat(Xmas.Stripe1.LedCounts);
  StrBuffer1.concat(F("}{Middle points settings;T;From bottom-0 to top}"));
  uint16_t lstpoint = 0;
  for (int i = 0; i < MAX_MIDDLEPOINTS; i++)
  {
    StrBuffer1.concat(F("{Point "));
    StrBuffer1.concat(i);
    StrBuffer1.concat(F(";N;"));
    StrBuffer1.concat(Xmas.Stripe1.MiddlePoints[i]);
    StrBuffer1.concat(F(";midp"));
    StrBuffer1.concat(i);
    StrBuffer1.concat(";");
    StrBuffer1.concat(lstpoint);
    StrBuffer1.concat(";");
    StrBuffer1.concat(Xmas.Stripe1.LedCounts - 1);
    StrBuffer1.concat("}");
    lstpoint = Xmas.Stripe1.MiddlePoints[i];
  }
  SERIALPRINTD("\r\nSending bytes: ", StrBuffer1.length());
  ServerHTML.send(200, "text/plain", StrBuffer1);
  SERIALPRINTD("\r\nSend:", StrBuffer1);
  StrBuffer1 = "";
}

//--------
void servers_handleWifiRescan()
{
  SERIALPRINTF("\r\nHandle wifirescan.");
  StrBuffer1 = F("Rescanning begin, wait 15sec for response...");
  ServerHTML.send(200, "text/plain", StrBuffer1);
  StrBuffer1 = "";
  SERIALPRINTF("\r\nResponse sended, rescanning...");
  servers_stop();
  WiFi.scanDelete();
  WlanStatus = WLAN_INIT;
}
//--------
void servers_handleRestartESP()
{
  SERIALPRINTF("\r\nHandle restartESP.");
  ServerHTML.send_P(200, "text/html", page_restartpage2);
  WlanStatus = WLAN_ESP_RESTART;
}
//--------
void servers_handleSaveRestESP()
{
  SERIALPRINTF("\r\nHandle SaveRestESP.");
  xmas_writeconfig(Xmas.WifiMode);
  servers_handleRestartESP();
}
//--------
void servers_handleWPS()
{
  SERIALPRINTF("\r\nHandle WPScmd.");
#ifdef USE_WPS
  StrBuffer1 = F("WPS scan begin...");
  ServerHTML.send(200, "text/plain", StrBuffer1);
  WlanStatus = WLAN_WPS_BEGIN;
#else
  StrBuffer1 = F("<b>WPS not Supported!</b><i>Enable option USE_WPS in helpers.cpp.</i> Wait 15sec for reload.");
  ServerHTML.send(200, "text/plain", StrBuffer1);
#endif
  SERIALPRINT(StrBuffer1);
  StrBuffer1 = "";
}
//--------
void servers_handleConfigListString()
{
  SERIALPRINTF("\r\nHandle ConfigListString.");
  StrBuffer1 = F("{Xmas ver;T;");
  StrBuffer1.concat(XMAS_VER);
  StrBuffer1.concat(F("}{Standalone AP name;C;"));
  StrBuffer1.concat(Xmas.APname);
  StrBuffer1.concat(F(";apname;5;32}{Standalone, shutdown WiFi (sec);N;"));
  StrBuffer1.concat(Xmas.APWifiTimeoutMs / 1000);
  StrBuffer1.concat(F(";apwifioff;200;9999}{OTA enable;B;"));
  StrBuffer1.concat(Xmas.OTAenable);
  StrBuffer1.concat(F(";otaenable}{Autochange effect (sec);N;"));
  StrBuffer1.concat(Xmas.EffectTimeoutMs / 1000);
  StrBuffer1.concat(F(";autoeffect;0;3600}{Number of LEDs;N;"));
  StrBuffer1.concat(Xmas.Stripe1.LedCounts);
  StrBuffer1.concat(F(";ledcounts;10;300}{Maximum Amperage (mA);N;"));
  StrBuffer1.concat(Xmas.Stripe1.AmperageMax);
  StrBuffer1.concat(F(";ampmax;60;15000}{Used with;T;Adafruit library}"));
  StrBuffer1.concat(F("{Neopixel GPIO pin;N;"));
  StrBuffer1.concat(Xmas.Stripe1.PinNo);
  StrBuffer1.concat(F(";pinno;1;16}"));
  StrBuffer1.concat(F("{Used with;T;SPI and Adafruit library}"));
  StrBuffer1.concat(F("{<b>Neopixel Type</b>;R;6;neopixeltype;NEO_RGB;1}"));
  StrBuffer1.concat(F("{;R;9;neopixeltype;NEO_RBG;0}"));
  StrBuffer1.concat(F("{;R;82;neopixeltype;NEO_GRB;0}"));
  StrBuffer1.concat(F("{;R;161;neopixeltype;NEO_GBR;0}"));
  StrBuffer1.concat(F("{;R;88;neopixeltype;NEO_BRG;0}"));
  StrBuffer1.concat(F("{;R;164;neopixeltype;NEO_BGR;0}"));
  StrBuffer1.concat(F("{Saved type;T;"));
  StrBuffer1.concat(NeoPixeltypeToStr(Xmas.Stripe1.neoPixelType));
  StrBuffer1.concat(F("}"));
  SERIALPRINTD("\r\nSending bytes: ", StrBuffer1.length());
  ServerHTML.send(200, "text/plain", StrBuffer1);
  SERIALPRINTD("\r\nSend:", StrBuffer1);
  StrBuffer1 = "";
}
//--------
void servers_handleConfigShowSave()
{
  SERIALPRINTF("\r\nHandle ConfigShowSave");
  String n = "";
  for (int i = 0; i < ServerHTML.args(); i++)
  {
    for (int p = 0; p < MAX_MIDDLEPOINTS; p++)
    {
      n = F("midp");
      n.concat(p);
      if (ServerHTML.argName(i) == n)
      {
        Xmas.Stripe1.MiddlePoints[p] = uint16_t(ServerHTML.arg(i).toInt());
        SERIALPRINTD("\r\nSet midpoint ", p);
        SERIALPRINTD(" = ", Xmas.Stripe1.MiddlePoints[p]);
      }
    }
  }
  ChangeEffect(EFF_MPOINTS);
  ServerHTML.send_P(200, "text/html", page_configshow);
}
//--------
void servers_handleCfgTree1Save()
{
  SERIALPRINTF("\r\nHandle cfgtree1save");
  boolean ch = false;
  for (int i = 0; i < ServerHTML.args(); i++)
  {
    if (ServerHTML.argName(i) == "apname")
    {
      strcpy(Xmas.APname, ServerHTML.arg(i).c_str());
      ch = true;
    }
    if (ServerHTML.argName(i) == "otaenable")
    {
      Xmas.OTAenable = u8_t(ServerHTML.arg(i).toInt());
      ch = true;
    }
    if (ServerHTML.argName(i) == "apwifioff")
    {
      Xmas.APWifiTimeoutMs = 1000 * uint32_t(ServerHTML.arg(i).toInt());
      if (Xmas.APWifiTimeoutMs < 1)
        Xmas.APWifiTimeoutMs = DEF_XMAS_AP_WLANOFF_MS;
      ch = true;
    }
    if (ServerHTML.argName(i) == "autoeffect")
    {
      Xmas.EffectTimeoutMs = 1000 * uint32_t(ServerHTML.arg(i).toInt());
      ch = true;
    }
    if (ServerHTML.argName(i) == "pinno")
    {
      Xmas.Stripe1.PinNo = u8_t(ServerHTML.arg(i).toInt());
      ch = true;
    }
    if (ServerHTML.argName(i) == "neopixeltype")
    {
      Xmas.Stripe1.neoPixelType = uint16_t(ServerHTML.arg(i).toInt());
      ch = true;
    }
    if (ServerHTML.argName(i) == "ledcounts")
    {
      Xmas.Stripe1.LedCounts = uint16_t(ServerHTML.arg(i).toInt());
      ch = true;
    }
    if (ServerHTML.argName(i) == "ampmax")
    {
      Xmas.Stripe1.AmperageMax = uint32_t(ServerHTML.arg(i).toInt());
      ch = true;
    }
  };
  if (ch)
  {
    xmas_writeconfig(Xmas.WifiMode);
    ServerHTML.send_P(200, "text/html", page_restartpage1);
  }
  else
    ServerHTML.send(200, "text/plain", F("Not Saved!"));
}
//--------
void servers_config() //setting up on events
{
  SERIALPRINTF("\r\nConfigure server's on events.");

  ServerHTML.on("/", []() { ServerHTML.send_P(200, "text/html", page_main); });
  ServerHTML.on("/configwifi", []() { ServerHTML.send_P(200, "text/html", page_configwifi); });
  ServerHTML.on("/show", []() { ServerHTML.send_P(200, "text/html", page_show); });
  ServerHTML.on("/wifiliststring", HTTP_GET, servers_handleWifiListString);
  ServerHTML.on("/infostring", HTTP_GET, servers_handleInfoString);
  ServerHTML.on("/effectliststring", HTTP_GET, servers_handleEffListString);
  ServerHTML.on("/infopage", []() { ServerHTML.send_P(200, "text/html", page_infopage); });
  ServerHTML.on("/functions.js", []() { ServerHTML.send_P(200, "application/javascript", page_functions); });
  ServerHTML.on("/configtree", []() { ServerHTML.send_P(200, "text/html", page_configtree); });
  ServerHTML.on("/configliststring", HTTP_GET, servers_handleConfigListString);
  ServerHTML.on("/rescancmd", HTTP_GET, servers_handleWifiRescan);
  ServerHTML.on("/restartcmd", HTTP_GET, servers_handleRestartESP);
  ServerHTML.on("/saverestarcmd", HTTP_POST, servers_handleSaveRestESP);
  ServerHTML.on("/wpscmd", HTTP_GET, servers_handleWPS);
  ServerHTML.on("/cfgtree1save", HTTP_POST, servers_handleCfgTree1Save);
  ServerHTML.on("/cfgwifisave", HTTP_POST, servers_handleCfgWifiSave);
  ServerHTML.on("/configshow", []() { ServerHTML.send_P(200, "text/html", page_configshow); });
  ServerHTML.on("/configshowsave", HTTP_POST, servers_handleConfigShowSave);
  ServerHTML.on("/configshowstring", HTTP_GET, servers_handleConfigShowString);
  //ServerHTML.on("/cfgshowsave", HTTP_POST, servers_handleCfgWifiSave);
  ServerHTML.onNotFound(servers_handleNotFound);

  if (Xmas.OTAenable)
  {
#ifdef USE_OTA
    SERIALPRINTD("\r\nConfigure httpOTA, user:", DEF_XMAS_OTAUSER);
    SERIALPRINTD("   pass:", DEF_XMAS_OTAPASS);
    SERIALPRINTD("   link:", DEF_XMAS_OTAPATH);
    ServerHttpOTA.setup(&ServerHTML, DEF_XMAS_OTAPATH, DEF_XMAS_OTAUSER, DEF_XMAS_OTAPASS);
#else
    SERIALPRINTF("\r\nOption OTA is enabled but service is disabled in source (uncomment USE_OTA)");
#endif
  }
  else
  {
    SERIALPRINTF("\r\nService httpOTA is disabled.");
  }
}
