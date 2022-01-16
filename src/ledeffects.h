#ifndef LEDEFFECTS_H
#define LEDEFFECTS_H
#include "helpers.cpp"
#include "font8x8_basic.h"
// functions
void PixDrawMpXY(int8_t x, int8_t y, uint32_t col32);
un_color32 PixGetMpXY(int8_t x, int8_t y, uint32_t retnocolor);
void PixLineMpDegr(int16_t d, int8_t len);
uint32_t HueToRGB32(uint32_t h, uint8_t sat, uint8_t val);
void PixFillAll(uint32_t col32);
void PixScrollUp();
void PixScrollDown();
void PixSparkle(uint16_t count, uint32_t col32);
void PixFadeOutAll(uint8_t steps);               // darken
void PixFadeInAll(uint8_t steps);                // lighten
void PixFadeToAll(uint8_t steps, uint32_t dcol); // fade to col
uint32_t PixFadeOut32(uint32_t c, uint8_t steps);
uint32_t PixFadeIn32(uint32_t c, uint8_t steps);
uint32_t PixFadeTo32(uint32_t c, uint32_t d);
void PixSmooth();
inline uint16_t PixNext(uint16_t currpix);
void PixSet(uint16_t pixno, uint32_t col32);
void PixSet(uint16_t pixno, uint8_t r, uint8_t g, uint8_t b);
un_color32 PixGet(uint16_t pixno);
inline uint32_t PixColor32(uint8_t r, uint8_t g, uint8_t b);
inline uint8 PixGetRed(uint32_t c32);
inline uint8 PixGetGreen(uint32_t c32);
inline uint8 PixGetBlue(uint32_t c32);
void PixEffect(u8_t effno); // processing effects

// external declarations
extern un_color32 *PixBuffer;
extern uint16_t PixBuffCount;
extern struct_xmas_config Xmas;
extern u8_t WlanStatus;
// definitions
#define HUE_RES 6 // resolution for huetorgb conversion 0=<hue<(360*hue_res)
#define HUE_MAX (HUE_RES * 360)

#define EFFECTS_COUNT 20
#define EFF1_TESTRGB 0
#define EFF2_RAINBOW 1
#define EFF3_BLUE_SP 2
#define EFF4_RED_SP 3
#define EFF5_RAINBOW_SP 4
#define EFF6_RIB_RAND 5
#define EFF7_TWO_RAINBOW 6
#define EFF8_TWO_RG 7
#define EFF9_COMET_RAND 8
#define EFF10_MPOINTS 9
#define EFF11_MP_MATRIX 10
#define EFF12_MP_REDF 11
#define EFF13_MP_LINE 12
#define EFF14_MP_BARS 13
#define EFF15_MP_HORX 14
#define EFF16_MP_CLOCK 15
#define EFF17_MP_CIRCLE 16
#define EFF18_MP_SCANS 17
#define EFF19_EFF_MP_BALL 18
#define EFF20_MP_ANIM1 19

#define EFF1_NAME F("Check (RRRGGB), all (G ap, B sta, Y wps)")
#define EFF2_NAME F("Scrolled down rainbow")
#define EFF3_NAME F("Wet blue comet with sparkles")
#define EFF4_NAME F("Red glowing heat")
#define EFF5_NAME F("Random sparkle rainbow")
#define EFF6_NAME F("Random ribbons")
#define EFF7_NAME F("Two point rainbow")
#define EFF8_NAME F("Two point (Red Green)")
#define EFF9_NAME F("Rainbow comet")
#define EFF10_NAME F("Vert middle points rainbow, test mpoints")
#define EFF11_NAME F("Vert slow green rain")
#define EFF12_NAME F("Vert red glowing up, tree on fire?")
#define EFF13_NAME F("Vert lines rainbow")
#define EFF14_NAME F("Horiz rainbow bars")
#define EFF15_NAME F("Horiz rainbow x")
#define EFF16_NAME F("Rainbow clock")
#define EFF17_NAME F("Center color slow")
#define EFF18_NAME F("Rainbow scans. both directions")
#define EFF19_NAME F("Rainbow bounce dot")
#define EFF20_NAME F("Rainbow ASCII chars?")

String EffectName(uint8_t no)
{
  if (no == 0)
    return (EFF1_NAME);
  if (no == 1)
    return (EFF2_NAME);
  if (no == 2)
    return (EFF3_NAME);
  if (no == 3)
    return (EFF4_NAME);
  if (no == 4)
    return (EFF5_NAME);
  if (no == 5)
    return (EFF6_NAME);
  if (no == 6)
    return (EFF7_NAME);
  if (no == 7)
    return (EFF8_NAME);
  if (no == 8)
    return (EFF9_NAME);
  if (no == 9)
    return (EFF10_NAME);
  if (no == 10)
    return (EFF11_NAME);
  if (no == 11)
    return (EFF12_NAME);
  if (no == 12)
    return (EFF13_NAME);
  if (no == 13)
    return (EFF14_NAME);
  if (no == 14)
    return (EFF15_NAME);
  if (no == 15)
    return (EFF16_NAME);
  if (no == 16)
    return (EFF17_NAME);
  if (no == 17)
    return (EFF18_NAME);
  if (no == 18)
    return (EFF19_NAME);
  if (no == 19)
    return (EFF20_NAME);

  return "";
};

#define COL_RED 0x00FF0000     // r
#define COL_GREN 0x0000FF00    // g
#define COL_BLUE 0x000000FF    // b
#define COL_MAGENTA 0x00FF00FF // m
#define COL_YELOW 0x00FFFF00   // y
#define COL_WHITE 0x00FFFFFF   // w
#define COL_BLACK 0x00000000
#define COL_GREY 0x00808080 // half

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
  case EFF1_TESTRGB:
    st16_fase++;
    if (st16_lastled >= PixBuffCount)
    {
      st16_lastled = 0;
      // set for wlna
      if (WlanStatus <= WLAN_SCAN_END)
        tm32_currcol = COL_WHITE;
      else if (WlanStatus <= WLAN_STA_RUN)
        tm32_currcol = COL_BLUE;
      else if (WlanStatus <= WLAN_WPS_END)
        tm32_currcol = COL_YELOW;
      else if (WlanStatus <= WLAN_AP_RUN)
        tm32_currcol = COL_GREN;
      else
        tm32_currcol = 0x00800000; // half red
      PixFillAll(tm32_currcol);
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
    break; // end EFF0_TESTRGB
  case EFF2_RAINBOW:
    if (st32_lastcolor > HUE_MAX)
      st32_lastcolor = 0;
    PixScrollDown();
    PixSet(PixBuffCount - 1, HueToRGB32(st32_lastcolor, 255, 255));
    st32_lastcolor++;
    break; // end EFF_RAINBOW
  case EFF3_BLUE_SP:
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

    break; // end EFF_BLUE_SP
  case EFF4_RED_SP:
    st16_fase = random(PixBuffCount / 10);
    PixFadeOutAll(st16_fase / 3);
    tm32_currcol = PixColor32(random(255), 0, 0);
    PixSparkle(st16_fase, tm32_currcol);
    if (!(st16_fase % 4))
      PixSparkle(1, COL_YELOW);
    break; // end EFF_RED_SP
  case EFF5_RAINBOW_SP:
    if (st32_lastcolor >= HUE_MAX)
      st32_lastcolor = 0;
    PixFadeOutAll(2);
    PixSparkle(random(PixBuffCount / 10), HueToRGB32(st32_lastcolor, 255, 200));
    st32_lastcolor++;
    break; // end EFF_RAINBOW_SP
  case EFF6_RIB_RAND:
    if (st8_direction < 1)
      st8_direction = 1;
    st16_lastled += st8_direction;
    if (st16_lastled > PixBuffCount)
    {
      st32_lastcolor = random(HUE_MAX);
      st8_direction = random(30);
      PixFadeOutAll(2);
    }
    PixSet(st16_lastled, HueToRGB32(st32_lastcolor, 255, 255));
    break; // end EFF_RIB_RAND
  case EFF7_TWO_RAINBOW:
    PixFadeOutAll(1);
    if (st32_lastcolor > HUE_MAX)
      st32_lastcolor = 0;
    PixSet(PixBuffCount - st16_lastled, HueToRGB32((HUE_MAX - st32_lastcolor), 255, 255));
    PixSet(st16_lastled, HueToRGB32(st32_lastcolor, 255, 255));
    st16_lastled++;
    st32_lastcolor++;
    break; // end EFF_TWO_RAINBOW
  case EFF8_TWO_RG:
    PixFadeOutAll((4 * st16_lastled) / (PixBuffCount + 1));
    PixSet(PixBuffCount - st16_lastled, COL_GREN);
    PixSet(st16_lastled, COL_RED);
    PixSparkle(st16_lastled % 3, COL_WHITE);
    st16_lastled++;
    break; // end EFF_TWO_RG
  case EFF9_COMET_RAND:
    if (st16_lastled == 0)
    {
      st32_lastcolor = HueToRGB32(random(HUE_MAX), 255, 255);
      st8_colval = 255 / (PixBuffCount / 2);
    }
    PixFadeOutAll(st8_colval / 2);
    if (!(st16_lastled % 8))
      PixSparkle(1, COL_WHITE);
    PixSet(PixBuffCount - st16_lastled, st32_lastcolor);
    st16_lastled++;
    break; // end EFF_COMET_RAND
  case EFF10_MPOINTS:

    tm32_currcol = HueToRGB32((random(30) * HUE_RES), 255, 32);
    PixSet(st16_lastled, tm32_currcol);
    if (st16_lastled % 2)
    {
      tm32_currcol = COL_WHITE;
    }

    for (tm16_currled = 0; tm16_currled < MAX_MIDDLEPOINTS; tm16_currled++)
    {
      PixSet(Xmas.Stripe1.MiddlePoints[tm16_currled], tm32_currcol);
    }
    st16_lastled++;
    break; // end EFF_MPOINTS
  case EFF11_MP_MATRIX:
    if (st8_direction < -(MAX_MIDDLEPOINTS / 2))
    {
      st8_direction = (MAX_MIDDLEPOINTS / 2);
    }
    if (st8_direction < 0)
      PixFadeOutAll(1);
    for (int8_t column = -30; column < 30; column++)
    {
      if (st8_direction > 0) // upper half chaos
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
    break; // end EFF_MP_MATRIX
  case EFF12_MP_REDF:
    /*
      PixFadeOutAll(st16_fase);
      for (st8_colsat = MAX_MIDDLEPOINTS - 1; st8_colsat > 0; st8_colsat--) // allmiddlepoints from top
      {
        tm16_currled = Xmas.Stripe1.MiddlePoints[st8_colsat];
        st16_lastled = Xmas.Stripe1.MiddlePoints[st8_colsat - 1];
        for (st8_colval = 0; st8_colval < MAX_MIDDLEPOINTS; st8_colval++) // horizont
        {
          PixSet(tm16_currled + st8_colval, PixGet(st16_lastled + st8_colval).c32);
          PixSet(tm16_currled - st8_colval, PixGet(st16_lastled - st8_colval).c32);
        }
      }

      PixSparkle(1, 0x00808080);
      PixSparkle(1, COL_YELOW);
      if (st16_fase > MAX_MIDDLEPOINTS)
      {
        st16_fase = 0;
        PixSparkle(1, COL_WHITE);
        for (st8_direction = 0; st8_direction < MAX_MIDDLEPOINTS; st8_direction++) // draw 0
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
      */

    // draw pixel on fire
    tm16_currled = random(25);
    st8_colval = 100 + random(155);
    tm32_currcol = HueToRGB32((26 - tm16_currled) * HUE_RES, 255, st8_colval);

    if ((tm32_currcol % 13) == 0)
      PixSparkle(1, COL_GREY);

    PixDrawMpXY(tm16_currled, (-MAX_MIDDLEPOINTS / 2), tm32_currcol);
    PixDrawMpXY(-tm16_currled, (-MAX_MIDDLEPOINTS / 2), tm32_currcol);

    st16_fase++;
    if (!(st16_fase % 5))
    {
      PixFadeOutAll(1);
      // break;
    }

    for (int8_t y = -(MAX_MIDDLEPOINTS / 2); y < (MAX_MIDDLEPOINTS / 2); y++)
    {
      st8_colval = random(MAX_MIDDLEPOINTS);
      for (int8_t x = 0; x < 25; x++)
      {
        tm32_currcol = PixGetMpXY(x, y, COL_RED).c32;
        tm32_currcol = PixFadeTo32(PixGetMpXY(x, y + 1, COL_BLACK).c32, tm32_currcol);
        tm32_currcol = PixFadeOut32(tm32_currcol, abs(x) + abs(y + st8_colval));
        if (y > 0)
          tm32_currcol = PixFadeOut32(tm32_currcol, random(x));
        PixDrawMpXY(x, y + 1, tm32_currcol);
        PixDrawMpXY(-x, y + 1, tm32_currcol);
      }
    }

    break; // end EFF_MP_RED
  case EFF13_MP_LINE:
    // random width, color
    if (st8_colsat > 0) // pause and fade
    {
      st8_colsat--;
      PixFadeOutAll(1);
      break;
    }

    if (st8_direction > (MAX_MIDDLEPOINTS / 2))
    {
      st16_lastled = 50 - random(50);
      st8_direction = -(MAX_MIDDLEPOINTS / 2);
      st32_lastcolor = st32_lastcolor + random(360);
      if (st32_lastcolor > HUE_MAX)
        st32_lastcolor = st32_lastcolor - HUE_MAX;
      st8_colsat = st16_lastled / 2;
      break;
    }
    PixDrawMpXY(25 - st16_lastled, st8_direction, HueToRGB32(st32_lastcolor, 255, 255));
    st8_direction++;
    st32_lastcolor++;
    break; // end EFF_MP_LINE
  case EFF14_MP_BARS:
    PixSmooth();
    if (st8_direction > 5)
    {
      PixFadeOutAll(1);
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
  case EFF15_MP_HORX:
    if (st16_fase > MAX_MIDDLEPOINTS) // thru mid point
    {
      st16_fase = 0;
      st16_lastled = 0;
      PixFadeOutAll(1);
      break;
    }
    PixDrawMpXY(st16_lastled, st16_fase - (MAX_MIDDLEPOINTS / 2), st32_lastcolor);
    PixDrawMpXY(-st16_lastled, st16_fase - (MAX_MIDDLEPOINTS / 2), st32_lastcolor);

    st16_lastled++;
    if (st16_lastled > MAX_MIDDLEPOINTS) // run thru x
    {
      st16_lastled = 0;
      st32_lastcolor = HueToRGB32(random(HUE_MAX), 255, 255);
      st16_fase++;
    }
    break; // END EFF_MP_HORX
  case EFF16_MP_CLOCK:
    PixFadeOutAll(1);
    // PixSparkle(1, COL_WHITE);
    st32_lastcolor++;
    if (st32_lastcolor > 360)
    {
      st32_lastcolor = 0;
      st16_fase++;
    }
    if (st16_fase > HUE_RES)
      st16_fase = 0;

    tm32_currcol = HueToRGB32(st32_lastcolor * st16_fase, 255, 255);
    if (1)
    {
      double dx = cos(st32_lastcolor * 0.017453); // calc steep per tick middlepoints
      double dy = sin(st32_lastcolor * 0.017453); // radians from degress

      for (int8_t i = 1; i < MAX_MIDDLEPOINTS; i++)
      {
        PixDrawMpXY((dx * i), (dy * i), tm32_currcol);
      }
    }

    break; // end EFF_MP_CLOCK
  case EFF17_MP_CIRCLE:
    // splash
    if (st16_fase > 0)
    {
      st16_fase--;
      PixFadeOutAll(1);
      break;
    }

    if (st16_lastled > 10)
    {
      st16_lastled = 0;
      PixFillAll(st32_lastcolor);
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
    break; // END EFF_MP_CIRCLE
  case EFF18_MP_SCANS:
    if (st8_direction == 0)
      st8_direction = 1;

    st16_lastled = st16_lastled + st8_direction;

    if (st16_lastled > 50)
    {
      st8_direction = -1;
    }
    else if (st16_lastled == 0)
    {
      st8_direction = 1;
      st8_colsat++;
    }
    st16_fase++;
    if (st16_fase > 2000)
      st16_fase = 0;

    if (st32_lastcolor > HUE_MAX)
      st32_lastcolor = 0;
    if (st8_colsat < 128)
      st8_colsat = 128;

    PixFadeOutAll(3);
    for (int8_t i = 0; i < 50; i++)
    {
      st32_lastcolor++;
      tm32_currcol = HueToRGB32(st32_lastcolor, st8_colsat, 160);
      if (st16_fase > 1000)
        PixDrawMpXY((i - 25), 25 - st16_lastled, tm32_currcol);
      else
        PixDrawMpXY(25 - st16_lastled, (i - 25), tm32_currcol);
    }

    break; // end EFF_MP_SCANS
  case EFF19_EFF_MP_BALL:
    static float_t px = 0, py = 0;
    static int8_t dx = 1, dy = 1;

    if (st32_lastcolor > HUE_MAX)
    {
      st32_lastcolor = 0;
    }
    else
      st32_lastcolor++;

    if (st16_fase > 128)
      st16_fase = 128;
    else
      st16_fase++;

    // borders
    if (px > 15)
    {
      dx = -1;
      st16_fase = st16_fase * 2;
    }
    if (px < -15)
    {
      dx = 1;
      st16_fase = st16_fase * 2;
    }
    if (py > (MAX_MIDDLEPOINTS / 2))
    {
      dy = -1;
      st16_fase = st16_fase / 2;
    }
    if (py < (-MAX_MIDDLEPOINTS / 2))
    {
      dy = 1;
      st16_fase = st16_fase / 2;
    }

    if (st16_fase < 1)
      st16_fase = 1;

    px = px + (dx * (128.0 / st16_fase) / 10.0);
    py = py + (dy * (128.0 / st16_fase) / 10.0);

    PixFadeToAll(1, HueToRGB32(st32_lastcolor + HUE_MAX / 2, 255, 32));

    tm32_currcol = HueToRGB32(st32_lastcolor, 255, 255);
    PixDrawMpXY(px, py, tm32_currcol);
    PixDrawMpXY(px + 1, py, tm32_currcol);
    PixDrawMpXY(px - 1, py, tm32_currcol);
    PixDrawMpXY(px, py - 1, tm32_currcol);
    PixDrawMpXY(px, py + 1, tm32_currcol);

    break; // end EFF_MP_BALL

  case EFF20_MP_ANIM1:
    st32_lastcolor++;
    if (st32_lastcolor > HUE_MAX)
      st32_lastcolor = 0;

    st8_colsat++; // shift x
    if (st8_colsat > 90)
    {
      st8_colsat = 0;
      st8_colval++;
      st8_direction = -(MAX_MIDDLEPOINTS - FONT8x8_H) + random((MAX_MIDDLEPOINTS - FONT8x8_H) * 2);
    }
    // PixFadeToAll(1, HueToRGB32(st32_lastcolor + (HUE_MAX / 2), 255, 32));
    PixFillAll(HueToRGB32(st32_lastcolor + (HUE_MAX / 2), 255, 32));
    if (st8_colval >= FONT8x8_COUNT)
      st8_colval = 0; // kolejny znak 33-127

    for (int8_t yy = 0; yy < FONT8x8_H; yy++)
    {
      int8_t bb = pgm_read_byte(font8x8_basic + (st8_colval * FONT8x8_H) + (yy));
      for (int8_t xx = 0; xx < FONT8x8_W; xx++)
      {
        if (bb & (1 << xx))
        {
          tm32_currcol = HueToRGB32(st32_lastcolor, 255, 255);
          PixDrawMpXY((-FONT8x8_W / 2) + xx + (-15 + st8_colsat / 3), (FONT8x8_H / 2) - yy + st8_direction, tm32_currcol);
        }
      }
    }

    /*
        for (int8_t yy = 0; yy < EFF20_BITM1_HEIGHT; yy++)
        {
          for (int16_t xx = 0; xx < EFF20_BITM1_WIDTH; xx++)
          {
            char t = pgm_read_byte(EFF20_BITM1 + (yy * EFF20_BITM1_WIDTH) + xx);
            switch (t)
            {
            case 'b':
              tm32_currcol = COL_BLUE;
              break;
            case 'g':
              tm32_currcol = COL_GREN;
              break;
            case 'm':
              tm32_currcol = COL_MAGENTA;
              break;
            case 'r':
              tm32_currcol = COL_RED;
              break;
            case 'y':
              tm32_currcol = COL_YELOW;
              break;
            case 'w':
              tm32_currcol = COL_WHITE;
              break;
            default:
              tm32_currcol = COL_BLACK;
            }
            PixDrawMpXY((-EFF20_BITM1_WIDTH / 2) + xx + px, py + yy - (EFF20_BITM1_HEIGHT / 2), tm32_currcol);
          }
        }
    */

    break; // end EFF_MP_ANIM1
  }
}
//-------------------------------------------------------
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
//--------darken color
uint32_t PixFadeOut32(uint32_t c, uint8_t steps)
{ // darken
  if (steps == 255)
    return COL_BLACK;
  if (steps == 0)
    return c;
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
//--------darken all pixels
void PixFadeOutAll(uint8_t steps)
{
  if (steps == 0)
    return;
  for (uint16_t i = 0; i < PixBuffCount; i++)
  {
    PixSet(i, PixFadeOut32(PixGet(i).c32, steps));
    // yield();
  }
}
//---------lighten all pixels
void PixFadeInAll(uint8_t steps)
{
  if (steps == 0)
    return;
  for (uint16_t i = 0; i < PixBuffCount; i++)
  {
    PixSet(i, PixFadeIn32(PixGet(i).c32, steps));
    // yield();
  }
}
//--------mean from two collors + lighten
void PixFadeToAll(uint8_t steps, uint32_t dcol)
{
  if (steps == 0)
    return;
  for (uint16_t i = 0; i < PixBuffCount; i++)
  {
    un_color32 tc;
    tc.c32 = PixFadeIn32(PixFadeTo32(PixGet(i).c32, dcol), steps);

    PixSet(i, tc.c32);
    // yield();
  }
}
//--------lighten color
uint32_t PixFadeIn32(uint32_t c, uint8_t steps)
{ // lighten
  if (steps == 255)
    return COL_WHITE;
  if (steps == 0)
    return c;
  un_color32 ret;
  ret.c32 = c;
  uint16_t tret;

  tret = ret.c8.r + steps;
  if (tret < 255)
    ret.c8.r = tret;
  else
    ret.c8.r = 255;

  tret = ret.c8.g + steps;
  if (tret < 255)
    ret.c8.g = tret;
  else
    ret.c8.g = 255;

  tret = ret.c8.b + steps;
  if (tret < 255)
    ret.c8.b = tret;
  else
    ret.c8.b = 255;

  return ret.c32;
}
//--------------
uint32_t PixFadeTo32(uint32_t s, uint32_t d)
{
  if (s == d)
    return d;

  un_color32 scol, dcol;
  scol.c32 = s;
  dcol.c32 = d;

  int16_t tc;

  tc = (dcol.c8.r + scol.c8.r) / 2;
  dcol.c8.r = tc;

  tc = (dcol.c8.g + scol.c8.g) / 2;
  dcol.c8.g = tc;

  tc = (dcol.c8.b + scol.c8.b) / 2;
  dcol.c8.b = tc;

  return dcol.c32;
}
//--------------
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
    // yield();
  }
}
//--------adds glitter
void PixSparkle(uint16_t count, uint32_t col32)
{
  while (count > 0)
  {
    PixSet(random(PixBuffCount), col32);
    count--;
    // yield();
  };
}
//--------
un_color32 IRAM_ATTR PixGet(uint16_t pixno)
{
  un_color32 ret;
  if (pixno >= PixBuffCount)
    ret.c32 = COL_BLACK;
  else
    ret.c32 = PixBuffer[pixno].c32;
  return ret;
}
//--------
void IRAM_ATTR PixSet(uint16_t pixno, uint32_t col32) // TODO: protect id
{
  if (pixno >= PixBuffCount)
    return;
  PixBuffer[pixno].c32 = col32;
}
//---------
void IRAM_ATTR PixSet(uint16_t pixno, uint8_t r, uint8_t g, uint8_t b) // TODO: protect id
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
  }
}
//----------
void PixScrollDown()
{
  for (uint16_t i = 1; i < PixBuffCount; i++)
  {
    PixSet(i - 1, PixGet(i).c32);
  }
}
//----------
void PixFillAll(uint32_t col32)
{
  for (uint16_t i = 0; i < PixBuffCount; i++)
  {
    PixSet(i, col32);
  }
}
//-------
// draws pixel,  x0 y0 are in center of Xmas.Stripe1.MiddlePoints
void PixDrawMpXY(int8_t x, int8_t y, uint32_t col32)
{
  int8_t cy = MAX_MIDDLEPOINTS / 2;
  cy = y + cy;
  if ((cy < 0) || (cy >= MAX_MIDDLEPOINTS))
    return;
  // left & right margin
  uint16_t lm;
  if (cy > 0)
  {
    lm = (Xmas.Stripe1.MiddlePoints[cy] - Xmas.Stripe1.MiddlePoints[cy - 1]) / 2; // middle first
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
  // left & right margin
  uint16_t lm;
  if (cy > 0)
  {
    lm = (Xmas.Stripe1.MiddlePoints[cy] - Xmas.Stripe1.MiddlePoints[cy - 1]) / 2; // middle first
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
void PixLineMpDegr(int16_t d, int8_t len)
{
  if (len == 0)
    return;
  // deg 0 top, 90 right, 180 down, 270 left
};

//------------
uint32_t ICACHE_RAM_ATTR HueToRGB32(uint32_t h, uint8_t sat, uint8_t val)
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

  /*
    //color tint
    if ((h>(HUE_RES*165)) && (h < (HUE_RES*330)))
    {//cold color, make it cool white
      h= h-(165*HUE_RES);
      sat= 16;
    }
  */
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
    // q = (256 * 255 * (60 * HUE_RES) + h * 255 * 255 - (60 * HUE_RES) * 255 * 255 * (i + 1)) / (256 * (60 * HUE_RES));
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