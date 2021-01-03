#ifndef LEDEFFECTS_H
#define LEDEFFECTS_H
#include "helpers.cpp"
//functions
void PixDrawMpXY(int8_t x, int8_t y, uint32_t col32);
un_color32 PixGetMpXY(int8_t x, int8_t y, uint32_t retnocolor);
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
#define EFF16_NAME F("Center color slow")

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
  uint16_t tm16_currled;
  uint32_t tm32_currcol;
  un_color32 tmun_pixcol;
  static uint16_t st16_lastled = 0;
  static uint32_t st32_lastcolor = 0;
  static uint8_t st8_colsat = 0;
  static uint8_t st8_colval = 0;
  static uint16_t st16_fase = 0;
  static int8_t st8_direction = 0;

  if (st16_lastled > PixBuffCount)
    st16_lastled = 0;

  switch (effno)
  {
  case EFF_TESTRGB:
    st16_fase++;
    if (st16_lastled >= PixBuffCount)
    {
      st16_lastled = 0;
      //set for wlna
      if (WlanStatus <= WLAN_SCAN_END)
        tm32_currcol = COL_WHITE;
      else if (WlanStatus <= WLAN_STA_RUN)
        tm32_currcol = COL_BLUE;
      else if (WlanStatus <= WLAN_WPS_END)
        tm32_currcol = COL_YELOW;
      else if (WlanStatus <= WLAN_AP_RUN)
        tm32_currcol = COL_GREN;
      else
        tm32_currcol = 0x00800000; //half red
      PixFill(tm32_currcol);
      break;
    }
    if (st16_fase > 5)
      st16_fase = 0;
    if (st16_fase < 3)
      tm32_currcol = COL_RED;
    else if (st16_fase < 5)
      tm32_currcol = COL_GREN;
    else
      tm32_currcol = COL_BLUE;
    PixSet(st16_lastled, tm32_currcol);
    st16_lastled++;
    break; //end EFF_TESTRGB
  case EFF_RAINBOW:
    if (st32_lastcolor > HUE_MAX)
      st32_lastcolor = 0;
    PixScrollDown();
    PixSet(PixBuffCount - 1, HueToRGB32(st32_lastcolor, 255, 255));
    st32_lastcolor++;
    break; //end EFF_RAINBOW
  case EFF_BLUE_SP:
    st16_fase = random(PixBuffCount / 10);
    st16_lastled += st16_fase + 1;
    while (st16_fase)
    {
      tmun_pixcol = PixGet(st16_lastled + st16_fase);
      tmun_pixcol.c8.r = random(32);
      tmun_pixcol.c8.g = tmun_pixcol.c8.r;
      tmun_pixcol.c8.b = tmun_pixcol.c8.b + tmun_pixcol.c8.r;
      if (tmun_pixcol.c8.b < tmun_pixcol.c8.r)
        tmun_pixcol.c32 = COL_BLUE / 3;
      PixSet(st16_lastled - st16_fase, tmun_pixcol.c32);
      st16_fase--;
    }
    if (st16_lastled % 2)
    {
      PixSparkle(1, COL_WHITE);
    }
    else
    {
      PixSmooth();
    }

    break; //end EFF_BLUE_SP
  case EFF_RED_SP:
    st16_fase = random(PixBuffCount / 10);
    PixFadeAll(st16_fase / 3);
    tm32_currcol = PixColor32(random(255), 0, 0);
    PixSparkle(st16_fase, tm32_currcol);
    if (!(st16_fase % 4))
      PixSparkle(1, COL_YELOW);
    break; //end EFF_RED_SP
  case EFF_RAINBOW_SP:
    if (st32_lastcolor >= HUE_MAX)
      st32_lastcolor = 0;
    PixFadeAll(2);
    PixSparkle(random(PixBuffCount / 10), HueToRGB32(st32_lastcolor, 255, 200));
    st32_lastcolor++;
    break; //end EFF_RAINBOW_SP
  case EFF_RIB_RAND:
    if (st8_direction < 1)
      st8_direction = 1;
    st16_lastled += st8_direction;
    if (st16_lastled > PixBuffCount)
    {
      st32_lastcolor = random(HUE_MAX);
      st8_direction = random(30);
      PixFadeAll(2);
    }
    PixSet(st16_lastled, HueToRGB32(st32_lastcolor, 255, 255));
    break; //end EFF_RIB_RAND
  case EFF_TWO_RAINBOW:
    PixFadeAll(st16_lastled / 20);
    if (st32_lastcolor > HUE_MAX)
      st32_lastcolor = 0;
    PixSet(PixBuffCount - st16_lastled, HueToRGB32(st32_lastcolor + (180 * HUE_RES), 255, 255));
    PixSet(st16_lastled, HueToRGB32(st32_lastcolor, 255, 255));
    st16_lastled++;
    st32_lastcolor++;
    break; //end EFF_TWO_RAINBOW
  case EFF_TWO_RG:
    PixFadeAll((4 * st16_lastled) / (PixBuffCount + 1));
    PixSet(PixBuffCount - st16_lastled, COL_GREN);
    PixSet(st16_lastled, COL_RED);
    PixSparkle(st16_lastled % 3, COL_WHITE);
    st16_lastled++;
    break; //end EFF_TWO_RG
  case EFF_TWO_COMETS:
    if (st16_lastled > (PixBuffCount / 2))
    {
      st16_lastled = 0;
      st32_lastcolor = random(COL_WHITE);
    }
    PixFadeAll((st16_lastled * 8) / (PixBuffCount + 1));
    PixSet(st16_lastled, st32_lastcolor);
    PixSet((PixBuffCount / 2) + st16_lastled, st32_lastcolor);
    st16_lastled++;
    break; //end EFF_TWO_COMETS
  case EFF_COMET_RAND:
    if (st16_lastled == 0)
    {
      st32_lastcolor = HueToRGB32(random(HUE_MAX), 255, 255);
      st8_colval = 255 / (PixBuffCount / 2);
    }
    PixFadeAll(st8_colval / 2);
    if (!(st16_lastled % 8))
      PixSparkle(1, COL_WHITE);
    PixSet(PixBuffCount - st16_lastled, st32_lastcolor);
    st16_lastled++;
    break; //end EFF_COMET_RAND
  case EFF_MPOINTS:
    if (st32_lastcolor > HUE_MAX)
      st32_lastcolor = 0;

    tm32_currcol = HueToRGB32(st32_lastcolor, 255, 255);
    PixFill(tm32_currcol);
    if (st32_lastcolor % 2)
    {
      for (tm16_currled = 0; tm16_currled < MAX_MIDDLEPOINTS; tm16_currled++)
      {
        PixSet(Xmas.Stripe1.MiddlePoints[tm16_currled], COL_WHITE);
      }
    }
    st32_lastcolor++;
    break; //end EFF_MPOINTS
  case EFF_MP_MATRIX:
    if (st8_direction < -(MAX_MIDDLEPOINTS / 2))
    {
      st8_direction = (MAX_MIDDLEPOINTS / 2);
    }
    if (st8_direction < 0)
      PixFadeAll(1);
    for (int8_t column = -30; column < 30; column++)
    {
      if (st8_direction > 0) //upper half chaos
      {
        tm32_currcol = PixColor32(0, 135 + random(120), 0);
        PixDrawMpXY(column, st8_direction, tm32_currcol);
      }
      else
      {
        tm32_currcol = PixGetMpXY(column, st8_direction + 1, COL_BLACK).c32;
        PixDrawMpXY(column, st8_direction, tm32_currcol);
      }
    }
    st8_direction--;
    break; //end EFF_MP_MATRIX
  case EFF_MP_REDF:
    PixFadeAll(st16_fase);
    for (st8_colsat = MAX_MIDDLEPOINTS - 1; st8_colsat > 0; st8_colsat--) //allmiddlepoints from top
    {
      tm16_currled = Xmas.Stripe1.MiddlePoints[st8_colsat];
      st16_lastled = Xmas.Stripe1.MiddlePoints[st8_colsat - 1];
      for (st8_colval = 0; st8_colval < MAX_MIDDLEPOINTS; st8_colval++) //horizont
      {
        PixSet(tm16_currled + st8_colval, PixGet(st16_lastled + st8_colval).c32);
        PixSet(tm16_currled - st8_colval, PixGet(st16_lastled - st8_colval).c32);
      }
    }

    PixSparkle(st16_fase / 3, COL_WHITE);
    PixSparkle(1, COL_YELOW);
    if (st16_fase > MAX_MIDDLEPOINTS)
    {
      st16_fase = 0;
      PixSparkle(1, COL_WHITE);
      for (st8_direction = 0; st8_direction < MAX_MIDDLEPOINTS; st8_direction++) //draw 0
      {
        tm16_currled = Xmas.Stripe1.MiddlePoints[st16_fase];
        st8_colval = random(100);
        tm32_currcol = PixColor32(55 + st8_colval * 2, st8_colval / 3, st8_colval / 3);
        PixSet(tm16_currled + st8_direction, tm32_currcol);
        PixSet(tm16_currled - st8_direction, tm32_currcol);
      }
      break;
    }

    st16_fase++;
    break; //end EFF_MP_RED
  case EFF_MP_LINE:

    if (st8_colsat > 0) //pause and fade
    {
      st8_colsat--;
      PixFadeAll(1);
      break;
    }

    if (st16_fase >= (MAX_MIDDLEPOINTS * 2))
    {
      st16_fase = 0;
      PixFill(HueToRGB32(st32_lastcolor, 255, 255));
      st32_lastcolor = random(HUE_MAX);
      st8_colsat = random(60);
      break;
    }
    st16_lastled = 0;
    while (st16_lastled < MAX_MIDDLEPOINTS)
    {
      tm16_currled = Xmas.Stripe1.MiddlePoints[st16_lastled];
      PixSet((tm16_currled - (MAX_MIDDLEPOINTS)) + st16_fase, HueToRGB32(st32_lastcolor, 255, 255));
      st16_lastled++;
    }
    st16_fase++;
    st32_lastcolor += HUE_RES;
    break; // end EFF_MP_LINE
  case EFF_MP_BARS:
    PixSmooth();
    if (st8_direction > 5)
    {
      PixFadeAll(1);
      st8_direction--;
      break;
    }

    if ((st16_fase > MAX_MIDDLEPOINTS) || (st32_lastcolor > HUE_MAX))
    {
      st16_fase = 0;
      st32_lastcolor = random(HUE_MAX);
    }
    tm32_currcol = HueToRGB32(st32_lastcolor, 255, 255);
    for (uint8_t x = 0; x < MAX_MIDDLEPOINTS; x++)
    {
      tm16_currled = Xmas.Stripe1.MiddlePoints[st16_fase];
      PixSet(tm16_currled + x, tm32_currcol);
      PixSet(tm16_currled - x, tm32_currcol);
    }
    st16_fase++;
    st32_lastcolor = st32_lastcolor + HUE_RES * (st16_fase * HUE_RES);
    st8_direction = random(st16_fase * 3);
    break; // end EFF_MP_BARS
  case EFF_MP_HORX:
    if (st16_fase > MAX_MIDDLEPOINTS) //thru mid point
    {
      st16_fase = 0;
      st16_lastled = 0;
      PixFadeAll(1);
      break;
    }
    PixDrawMpXY(st16_lastled, st16_fase - (MAX_MIDDLEPOINTS / 2), st32_lastcolor);
    PixDrawMpXY(-st16_lastled, st16_fase - (MAX_MIDDLEPOINTS / 2), st32_lastcolor);

    st16_lastled++;
    if (st16_lastled > MAX_MIDDLEPOINTS) //run thru x
    {
      st16_lastled = 0;
      st32_lastcolor = HueToRGB32(random(HUE_MAX), 255, 255);
      st16_fase++;
    }
    break; //END EFF_MP_HORX
  case EFF_MP_HXMAS:
    //splash
    if (st16_fase > 0)
    {
      st16_fase--;
      PixFadeAll(1);
      break;
    }

    if (st16_lastled > 10)
    {
      st16_lastled = 0;
      PixFill(st32_lastcolor);
      st32_lastcolor = HueToRGB32(random(HUE_MAX), 255, 255);
      st16_fase = random(128);
      break;
    }

    tm32_currcol = st32_lastcolor;

    for (int16_t tx = (-st16_lastled); tx < (st16_lastled); tx++)
    {
      int16_t he = sqrt(st16_lastled * st16_lastled - tx * tx);
      for (int16_t ty = (-he); ty < he; ty++)
      {
        PixDrawMpXY(tx, ty, tm32_currcol);
      }
    }
    st16_lastled++;
    st16_fase = random(30 - st16_lastled);
    break; //END EFF_MP_HXMAS
  }
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
//draws pixel,  x0 y0 are in center of Xmas.Stripe1.MiddlePoints
void PixDrawMpXY(int8_t x, int8_t y, uint32_t col32)
{
  int8_t cy = MAX_MIDDLEPOINTS / 2;
  cy = y + cy;
  if ((cy < 0) || (cy >= MAX_MIDDLEPOINTS))
    return;
  //left & right margin
  uint16_t lm;
  if (cy > 0)
  {
    lm = (Xmas.Stripe1.MiddlePoints[cy] - Xmas.Stripe1.MiddlePoints[cy - 1]) / 2; //middle first
    lm = Xmas.Stripe1.MiddlePoints[cy] - lm;
  }
  else
  {
    lm = 0;
  }
  uint16_t rm;
  if (cy < (MAX_MIDDLEPOINTS - 1))
  {
    rm = (Xmas.Stripe1.MiddlePoints[cy + 1] - Xmas.Stripe1.MiddlePoints[cy]) / 2;
    rm = Xmas.Stripe1.MiddlePoints[cy] + rm;
  }
  else
  {
    rm = PixBuffCount - 1;
  }
  int16_t pixx = Xmas.Stripe1.MiddlePoints[cy] + x;
  if ((pixx < lm) || (pixx > rm))
    return;
  PixSet(pixx, col32);
}
//------------
un_color32 PixGetMpXY(int8_t x, int8_t y, uint32_t retnocolor)
{
  int8_t cy = MAX_MIDDLEPOINTS / 2;
  un_color32 ret;
  ret.c32 = retnocolor;
  cy = y + cy;
  if ((cy < 0) || (cy >= MAX_MIDDLEPOINTS))
    return ret;
  //left & right margin
  uint16_t lm;
  if (cy > 0)
  {
    lm = (Xmas.Stripe1.MiddlePoints[cy] - Xmas.Stripe1.MiddlePoints[cy - 1]) / 2; //middle first
    lm = Xmas.Stripe1.MiddlePoints[cy] - lm;
  }
  else
  {
    lm = 0;
  }
  uint16_t rm;
  if (cy < (MAX_MIDDLEPOINTS - 1))
  {
    rm = (Xmas.Stripe1.MiddlePoints[cy + 1] - Xmas.Stripe1.MiddlePoints[cy]) / 2;
    rm = Xmas.Stripe1.MiddlePoints[cy] + rm;
  }
  else
  {
    rm = PixBuffCount - 1;
  }
  int16_t pixx = Xmas.Stripe1.MiddlePoints[cy] + x;
  if ((pixx < lm) || (pixx > rm))
    return ret;

  return (PixGet(pixx));
}
//------------
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