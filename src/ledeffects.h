#ifndef LEDEFFECTS_H
#define LEDEFFECTS_H
#include "helpers.cpp"
//functions
uint32_t HueToRGB32(uint32_t h, uint8_t sat, uint8_t val);
void PixFill(uint32_t col32);
void PixScrollUp();
void PixScrollDown();
void PixSparkle(uint16_t count, uint32_t col32);
void PixFadeAll(uint8_t steps);
uint32_t PixFade32(uint32_t c, uint8_t steps);
void PixSmooth();
inline uint16_t PixNext(uint16_t currpix);
void PixSet(uint16_t pixno, uint32_t col32);
void PixSet(uint16_t pixno, uint8_t r, uint8_t g, uint8_t b);
un_color32 PixGet(uint16_t pixno);
inline uint32_t PixColor32(uint8_t r, uint8_t g, uint8_t b);
inline uint8 PixGetRed(uint32_t c32);
inline uint8 PixGetGreen(uint32_t c32);
inline uint8 PixGetBlue(uint32_t c32);
void PixEffect(u8_t effno); //processing effects

//external declarations
extern un_color32 *PixBuffer;
extern uint16_t PixBuffCount;
extern struct_xmas_config Xmas;
extern u8_t WlanStatus;
// definitions
#define HUE_RES 4 //resolution for huetorgb conversion 0=<hue<(360*hue_res)
#define HUE_MAX (HUE_RES * 360)

#define EFFECTS_COUNT 17
#define EFF_TESTRGB 0
#define EFF_RAINBOW 1
#define EFF_BLUE_SP 2
#define EFF_RED_SP 3
#define EFF_RAINBOW_SP 4
#define EFF_RIB_RAND 5
#define EFF_TWO_RAINBOW 6
#define EFF_TWO_RG 7
#define EFF_TWO_COMETS 8
#define EFF_COMET_RAND 9
#define EFF_MPOINTS 10
#define EFF_MP_MATRIX 11
#define EFF_MP_REDF 12
#define EFF_MP_LINE 13
#define EFF_MP_BARS 14
#define EFF_MP_HORX 15
#define EFF_MP_HXMAS 16

#define EFF0_NAME F("Check (RRRGGB), all (G ap, B sta, Y wps)")
#define EFF1_NAME F("Scrolled down rainbow")
#define EFF2_NAME F("Wet blue comet with sparkles")
#define EFF3_NAME F("Red glowing heat")
#define EFF4_NAME F("Random sparkle rainbow")
#define EFF5_NAME F("Random ribbons")
#define EFF6_NAME F("Two point rainbow")
#define EFF7_NAME F("Two point (Red Green)")
#define EFF8_NAME F("Two comets")
#define EFF9_NAME F("Rainbow comet")
#define EFF10_NAME F("Vert middle points rainbow")
#define EFF11_NAME F("Vert slow green rain")
#define EFF12_NAME F("Vert red glowing up")
#define EFF13_NAME F("Vert lines rainbow")
#define EFF14_NAME F("Horiz rainbow bars")
#define EFF15_NAME F("Horiz rainbow x")
#define EFF16_NAME F("Horiz Christmas Bars")

String EffectName(uint8_t no)
{
  if (no == 0)
    return (EFF0_NAME);
  if (no == 1)
    return (EFF1_NAME);
  if (no == 2)
    return (EFF2_NAME);
  if (no == 3)
    return (EFF3_NAME);
  if (no == 4)
    return (EFF4_NAME);
  if (no == 5)
    return (EFF5_NAME);
  if (no == 6)
    return (EFF6_NAME);
  if (no == 7)
    return (EFF7_NAME);
  if (no == 8)
    return (EFF8_NAME);
  if (no == 9)
    return (EFF9_NAME);
  if (no == 10)
    return (EFF10_NAME);
  if (no == 11)
    return (EFF11_NAME);
  if (no == 12)
    return (EFF12_NAME);
  if (no == 13)
    return (EFF13_NAME);
  if (no == 14)
    return (EFF14_NAME);
  if (no == 15)
    return (EFF15_NAME);
  if (no == 16)
    return (EFF16_NAME);
  return "";
};

#define COL_RED 0x00FF0000
#define COL_GREN 0x0000FF00
#define COL_BLUE 0x000000FF
#define COL_MAGENTA 0x00FF00FF
#define COL_YELOW 0x00FFFF00
#define COL_WHITE 0x00FFFFFF
#define COL_BLACK 0x00000000
//-------------------------------------------------------
void PixEffect(u8_t effno)
{
  if (effno >= EFFECTS_COUNT)
    return;
  uint16_t currled;
  uint32_t currcol32;
  static uint16_t lastled = 0;
  static uint32_t lastcol32 = 0;
  static uint8_t colsat = 0;
  static uint8_t colval = 0;
  static uint16_t fase = 0;
  static int8_t direction = 0;
  static u8_t lasteffect = 0;                     //to detect effect change
  const uint8_t linebuffcount = MAX_MIDDLEPOINTS; //width of e buffer, one line, int becouse some eff have negative numbers
  static int32_t linebuff[linebuffcount];

  if (lastled > PixBuffCount)
    lastled = 0;

  switch (effno)
  {
  case EFF_TESTRGB:
    lastled++;
    fase++;
    if (lastled >= PixBuffCount)
    {
      lastled = 0;
      //set for wlna
      if (WlanStatus <= WLAN_SCAN_END)
        currcol32 = COL_WHITE;
      else if (WlanStatus <= WLAN_STA_RUN)
        currcol32 = COL_BLUE;
      else if (WlanStatus <= WLAN_WPS_END)
        currcol32 = COL_YELOW;
      else if (WlanStatus <= WLAN_AP_RUN)
        currcol32 = COL_GREN;
      else
        currcol32 = 0x00800000; //half red
      PixFill(currcol32);
      break;
    }
    if (fase > 5)
      fase = 0;
    if (fase < 3)
      currcol32 = COL_RED;
    else if (fase < 5)
      currcol32 = COL_GREN;
    else
      currcol32 = COL_BLUE;
    PixSet(lastled, currcol32);
    break; //end EFF_TESTRGB
  case EFF_RAINBOW:
    if (lastcol32 > HUE_MAX)
      lastcol32 = 0;
    PixScrollDown();
    PixSet(PixBuffCount - 1, HueToRGB32(lastcol32, 255, 255));
    lastcol32++;
    break; //end EFF_RAINBOW
  case EFF_BLUE_SP:
    PixSmooth();
    lastled = random(PixBuffCount - 1);
    currled = (PixBuffCount - lastled) / 2;
    colsat = random(25);
    while (currled)
    {
      currled--;
      currcol32 = PixColor32(colsat, colsat, colsat + random(230));
      PixSet(lastled + currled, currcol32);
    }
    PixSparkle(1, COL_WHITE);
    break; //end EFF_BLUE_SP
  case EFF_RED_SP:
    fase = random(PixBuffCount / 4);
    PixFadeAll(fase / 3);
    currcol32 = PixColor32(random(255), 0, 0);
    PixSparkle(fase, currcol32);
    if (!(fase % 4))
      PixSparkle(1, COL_YELOW);
    break; //end EFF_RED_SP
  case EFF_RAINBOW_SP:
    if (lastcol32 >= HUE_MAX)
      lastcol32 = 0;
    fase = 2 + random(10);
    PixFadeAll(fase / 2);
    PixSparkle(fase, HueToRGB32(lastcol32++, 255, 200));
    break; //end EFF_RAINBOW_SP
  case EFF_RIB_RAND:
    if (lasteffect != effno)
    {
      lastled = 0;
    }
    if (lastled == 0)
    {
      lastcol32 = random(COL_WHITE);
      fase = random(PixBuffCount / 5);
      if (fase < 2)
        fase = 1;
    }
    PixSet(lastled - 1, lastcol32);
    PixSet(lastled + 1, lastcol32);
    PixFadeAll(1);
    PixSet(lastled, lastcol32);
    lastled = lastled + fase;
    break; //end EFF_RIB_RAND
  case EFF_TWO_RAINBOW:
    PixFadeAll(lastled / 20);
    if (lastcol32 > HUE_MAX)
      lastcol32 = 0;
    PixSet(PixBuffCount - lastled, HueToRGB32(lastcol32 + (180 * HUE_RES), 255, 255));
    PixSet(lastled, HueToRGB32(lastcol32, 255, 255));
    lastled++;
    lastcol32++;
    break; //end EFF_TWO_RAINBOW
  case EFF_TWO_RG:
    PixFadeAll((4 * lastled) / (PixBuffCount + 1));
    PixSet(PixBuffCount - lastled, COL_GREN);
    PixSet(lastled, COL_RED);
    PixSparkle(lastled % 3, COL_WHITE);
    lastled++;
    break; //end EFF_TWO_RG
  case EFF_TWO_COMETS:
    if (lastled > (PixBuffCount / 2))
    {
      lastled = 0;
      lastcol32 = random(COL_WHITE);
    }
    PixFadeAll((lastled * 8) / (PixBuffCount + 1));
    PixSet(lastled, lastcol32);
    PixSet((PixBuffCount / 2) + lastled, lastcol32);
    lastled++;
    break; //end EFF_TWO_COMETS
  case EFF_COMET_RAND:
    if (lastled == 0)
    {
      lastcol32 = HueToRGB32(random(HUE_MAX), 255, 255);
      colval = 255 / (PixBuffCount / 2);
    }
    PixFadeAll(colval / 2);
    if (!(lastled % 8))
      PixSparkle(1, COL_WHITE);
    PixSet(PixBuffCount - lastled, lastcol32);
    lastled++;
    break; //end EFF_COMET_RAND
  case EFF_MPOINTS:
    if (lastcol32 > HUE_MAX)
      lastcol32 = 0;

    currcol32 = HueToRGB32(lastcol32, 255, 255);
    PixFill(currcol32);
    if (lastcol32 % 2)
    {
      for (currled = 0; currled < MAX_MIDDLEPOINTS; currled++)
      {
        PixSet(Xmas.Stripe1.MiddlePoints[currled], COL_WHITE);
      }
    }
    lastcol32++;
    break; //end EFF_MPOINTS
  case EFF_MP_MATRIX:
    if (lasteffect != effno)
    {
      for (fase = 0; fase < linebuffcount; fase++)
      {
        linebuff[fase] = MAX_MIDDLEPOINTS + random(MAX_MIDDLEPOINTS);
      }
      fase = 0;
    }
    if (fase >= linebuffcount)
    {
      fase = 0;
      PixFadeAll(255 / (MAX_MIDDLEPOINTS * 2));
    };
    if ((linebuff[fase] < MAX_MIDDLEPOINTS) && (linebuff[fase] >= 0))
    {
      PixSet(Xmas.Stripe1.MiddlePoints[linebuff[fase]] - fase, COL_GREN);
      PixSet(Xmas.Stripe1.MiddlePoints[linebuff[fase]] + fase, COL_GREN);
    }
    linebuff[fase] = linebuff[fase] - 1;
    if (linebuff[fase] < (-MAX_MIDDLEPOINTS))
    {
      linebuff[fase] = MAX_MIDDLEPOINTS + random(MAX_MIDDLEPOINTS);
    }
    fase++;
    break; //end EFF_MP_MATRIX
  case EFF_MP_REDF:
    PixFadeAll(fase);
    for (colsat = MAX_MIDDLEPOINTS - 1; colsat > 0; colsat--) //allmiddlepoints from top
    {
      currled = Xmas.Stripe1.MiddlePoints[colsat];
      lastled = Xmas.Stripe1.MiddlePoints[colsat - 1];
      for (colval = 0; colval < MAX_MIDDLEPOINTS; colval++) //horizont
      {
        PixSet(currled + colval, PixGet(lastled + colval).c32);
        PixSet(currled - colval, PixGet(lastled - colval).c32);
      }
    }

    PixSparkle(fase / 3, COL_WHITE);
    PixSparkle(1, COL_YELOW);
    if (fase > MAX_MIDDLEPOINTS)
    {
      fase = 0;
      PixSparkle(1, COL_WHITE);
      for (direction = 0; direction < MAX_MIDDLEPOINTS; direction++) //draw 0
      {
        currled = Xmas.Stripe1.MiddlePoints[fase];
        colval = random(100);
        currcol32 = PixColor32(55 + colval * 2, colval / 3, colval / 3);
        PixSet(currled + direction, currcol32);
        PixSet(currled - direction, currcol32);
      }
      break;
    }

    fase++;
    break; //end EFF_MP_RED
  case EFF_MP_LINE:

    if (fase >= (MAX_MIDDLEPOINTS * 2))
    {
      fase = 0;
      PixFill(HueToRGB32(lastcol32, 255, 255));
      lastcol32 = random(HUE_MAX);
      break;
    }
    lastled = 0;
    while (lastled < MAX_MIDDLEPOINTS)
    {
      currled = Xmas.Stripe1.MiddlePoints[lastled];
      PixSet((currled - (MAX_MIDDLEPOINTS)) + fase, HueToRGB32(lastcol32, 255, 255));
      lastled++;
    }
    fase++;
    lastcol32 += 4;
    break; // end EFF_MP_LINE
  case EFF_MP_BARS:
    PixSmooth();
    if (direction > 5)
    {
      PixFadeAll(1);
      direction--;
      break;
    }

    if ((fase > MAX_MIDDLEPOINTS) || (lastcol32 > HUE_MAX))
    {
      fase = 0;
      lastcol32 = random(HUE_MAX);
    }
    currcol32 = HueToRGB32(lastcol32, 255, 255);
    for (uint8_t x = 0; x < MAX_MIDDLEPOINTS; x++)
    {
      currled = Xmas.Stripe1.MiddlePoints[fase];
      PixSet(currled + x, currcol32);
      PixSet(currled - x, currcol32);
    }
    fase++;
    lastcol32 = lastcol32 + HUE_RES * (fase * HUE_RES);
    direction = random(fase * 3);
    break; // end EFF_MP_BARS
  case EFF_MP_HORX:
    if (lastled >= MAX_MIDDLEPOINTS)
    {
      lastled = 1;
      lastcol32 = random(HUE_MAX);
      PixSmooth();
      break;
    }

    if (lastled % 2)
      currcol32 = HueToRGB32(HUE_MAX - lastcol32, 255, 255);
    else
      currcol32 = HueToRGB32(lastcol32, 255, 255);

    PixSet(Xmas.Stripe1.MiddlePoints[lastled] - fase, currcol32);
    PixSet(Xmas.Stripe1.MiddlePoints[lastled] + fase, currcol32);
    if (fase < 1)
    {
      fase = (Xmas.Stripe1.MiddlePoints[lastled] - Xmas.Stripe1.MiddlePoints[lastled - 1]) / 2;
      lastled++;
    }
    fase--;
    break; //END EFF_MP_HORX
  case EFF_MP_HXMAS:
    if (lastcol32 > HUE_MAX)
    {
      lastcol32 = 0;
      colsat++;
    }
    currcol32 = HueToRGB32(lastcol32, colsat, 255);
    if (lastled >= MAX_MIDDLEPOINTS)
    {
      lastled = 0;
      PixFadeAll(1);
      lastcol32++;
    }

    if (fase > 20)
    {
      fase = 0;
      lastled++;
    }
    if ((fase < 10) && (lastled < (MAX_MIDDLEPOINTS / 2)))
    {
      currcol32 = PixFade32(COL_WHITE, colsat);
    }
    if ((fase > 10) && (lastled > (MAX_MIDDLEPOINTS / 2)))
    {
      currcol32 = PixFade32(COL_WHITE, colsat);
    }
    PixSet(Xmas.Stripe1.MiddlePoints[lastled] + int(10 - fase), currcol32);

    fase++;
    break; //END EFF_MP_HXMAS
  }
  lasteffect = effno;
}
//-------------------------------------------------------
inline uint32_t PixColor32(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}
//------------
inline uint8 PixGetRed(uint32_t c32)
{
  return ((0x00FF0000 & c32) >> 16);
}
//------------
inline uint8 PixGetGreen(uint32_t c32)
{
  return ((0x0000FF00 & c32) >> 8);
}
//-------------
inline uint8 PixGetBlue(uint32_t c32)
{
  return (0x000000FF & c32);
}
//--------
uint32_t PixFade32(uint32_t c, uint8_t steps)
{
  un_color32 ret;
  ret.c32 = c;
  if (ret.c8.r > steps)
    ret.c8.r = ret.c8.r - steps;
  else
    ret.c8.r = 0;
  if (ret.c8.g > steps)
    ret.c8.g = ret.c8.g - steps;
  else
    ret.c8.g = 0;
  if (ret.c8.b > steps)
    ret.c8.b = ret.c8.b - steps;
  else
    ret.c8.b = 0;
  return ret.c32;
}
//--------
void PixFadeAll(uint8_t steps)
{
  if (steps == 0)
    return;
  for (uint16_t i = 0; i < PixBuffCount; i++)
  {
    PixSet(i, PixFade32(PixGet(i).c32, steps));
    yield();
  }
}
//--------
void PixSmooth()
{
  int32_t r, g, b;
  un_color32 cc;
  for (uint16_t i = 1; i < (PixBuffCount - 1); i++)
  {
    r = 0;
    g = 0;
    b = 0;
    for (int s = -1; s < 2; s++)
    {
      cc = PixGet(i + s);
      r += cc.c8.r;
      g += cc.c8.g;
      b += cc.c8.b;
    }
    r = r / 3;
    g = g / 3;
    b = b / 3;
    for (int s = -1; s < 2; s++)
    {
      cc = PixGet(i + s);
      PixSet(i + s, (cc.c8.r + r) / 2, (cc.c8.g + g) / 2, (cc.c8.b + b) / 2);
    }
    i++;
    yield();
  }
}
//--------
void PixSparkle(uint16_t count, uint32_t col32)
{
  while (count > 0)
  {
    PixSet(random(PixBuffCount), col32);
    count--;
    yield();
  };
}
//--------
un_color32 PixGet(uint16_t pixno)
{
  un_color32 ret;
  if (pixno >= PixBuffCount)
    ret.c32 = COL_BLACK;
  else
    ret.c32 = PixBuffer[pixno].c32;
  return ret;
}
//--------
void PixSet(uint16_t pixno, uint32_t col32) //TODO: protect id
{
  if (pixno >= PixBuffCount)
    return;
  PixBuffer[pixno].c32 = col32;
}
//---------
void PixSet(uint16_t pixno, uint8_t r, uint8_t g, uint8_t b) //TODO: protect id
{
  if (pixno >= PixBuffCount)
    return;
  PixBuffer[pixno].c8.r = r;
  PixBuffer[pixno].c8.g = g;
  PixBuffer[pixno].c8.b = b;
}
//---------
inline uint16_t PixNext(uint16_t currpix)
{
  currpix++;
  if (currpix >= PixBuffCount)
    currpix = 0;
  return currpix;
}
//---------
void PixScrollUp()
{
  for (uint16_t i = (PixBuffCount - 1); i > 0; i--)
  {
    PixSet(i, PixGet(i - 1).c32);
    yield();
  }
}
//----------
void PixScrollDown()
{
  for (uint16_t i = 1; i < PixBuffCount; i++)
  {
    PixSet(i - 1, PixGet(i).c32);
    yield();
  }
}
//----------
void PixFill(uint32_t col32)
{
  for (uint16_t i = 0; i < PixBuffCount; i++)
  {
    PixSet(i, col32);
    yield();
  }
}
//-------
uint32_t HueToRGB32(uint32_t h, uint8_t sat, uint8_t val)
{
  int32 i, p, q;
  uint8_t r = 0, g = 0, b = 0;
  if (sat == 0)
  {
    return uint32_t(((uint32_t)(val << 16)) + ((uint32_t)(val << 8)) + val);
  };
  while (h > (HUE_MAX - 1))
  {
    h = h - HUE_MAX;
  };
  i = h / (60 * HUE_RES);
  p = (256 * val - sat * val) / 256;
  if (i & 1)
  {
    q = (256 * 60 * HUE_RES * val - h * sat * val + 60 * HUE_RES * sat * val * i) / (256 * (60 * HUE_RES));
    switch (i)
    {
    case 1:
      r = q;
      g = val;
      b = p;
      break;
    case 3:
      r = p;
      g = q;
      b = val;
      break;
    case 5:
      r = val;
      g = p;
      b = q;
      break;
    }
  }
  else
  {
    //q = (256 * 255 * (60 * HUE_RES) + h * 255 * 255 - (60 * HUE_RES) * 255 * 255 * (i + 1)) / (256 * (60 * HUE_RES));
    q = (256 * 60 * HUE_RES * val + h * sat * val - 60 * HUE_RES * sat * val * (i + 1)) / (256 * (60 * HUE_RES));
    switch (i)
    {
    case 0:
      r = val;
      g = q;
      b = p;
      break;
    case 2:
      r = p;
      g = val;
      b = q;
      break;
    case 4:
      r = q;
      g = p;
      b = val;
      break;
    }
  }
  return uint32_t(((uint32_t)(r << 16)) + ((uint32_t)(g << 8)) + b);
};

#endif