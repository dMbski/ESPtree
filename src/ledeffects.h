#ifndef LEDEFFECTS_H
#define LEDEFFECTS_H
#include "helpers.cpp"
//functions
uint32_t HueToRGB32(uint32_t h, uint8_t sat, uint8_t val);
void PixFill(uint32_t col32);
void PixScrollUp();
void PixScrollDown();
void PixSparkle(uint16_t count, uint32_t col32);
void PixFade(uint8_t steps);
void PixSmooth();
inline uint16_t PixNext(uint16_t currpix);
void PixSet(uint16_t pixno, uint32_t col32);
void PixSet(uint16_t pixno, uint8_t r, uint8_t g, uint8_t b);
un_color32 PixGet(uint16_t pixno);
inline uint32_t PixColor32(uint8_t r, uint8_t g, uint8_t b);
void PixEffect(u8_t effno); //processing effects

//external declarations
extern un_color32 *PixBuffer;
extern uint16_t PixBuffCount;

// definitions
#define HUE_RES 3 //resolution for huetorgb conversion 0=<hue<(360*hue_res)
#define HUE_MAX (HUE_RES * 360)

#define EFFECTS_COUNT 10
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

#define EFF0_NAME F("Slow Color check (R R R G G B)")
#define EFF1_NAME F("Scrolled down rainbow")
#define EFF2_NAME F("Wet blue commet with sparkles")
#define EFF3_NAME F("Red glowing heat")
#define EFF4_NAME F("Random sparkle rainbow")
#define EFF5_NAME F("Scrolled random ribbon")
#define EFF6_NAME F("Two point rainbow")
#define EFF7_NAME F("Two point (Red Green)")
#define EFF8_NAME F("Two comets")
#define EFF9_NAME F("Random comet")

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
  return "";
};

#define COL_RED 0x00FF0000
#define COL_GREN 0x0000FF00
#define COL_BLUE 0x000000FF
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
  static uint16_t lastled;
  static uint32_t lastcol32;
  static uint8_t colsat;
  static uint8_t colval;
  static uint16_t fase;
  static int8_t direction;
  static u8_t lasteffect; //to detect effect change

  if (lastled > PixBuffCount)
    lastled = 0;

  switch (effno)
  {
  case EFF_TESTRGB:
    if (colval > 253)
    {
      direction = -1;
    }
    if (colval < 2)
    {
      direction = 1;
    }
    currcol32 = HueToRGB32(0, 255, colval);
    PixSet(lastled++, currcol32);
    PixSet(lastled++, currcol32);
    PixSet(lastled++, currcol32);
    currcol32 = HueToRGB32(120 * HUE_RES, 255, colval);
    PixSet(lastled++, currcol32);
    PixSet(lastled++, currcol32);
    PixSet(lastled++, HueToRGB32(240 * HUE_RES, 255, colval));
    colval = colval + direction;
    break; //end EFF_TESTRGB
  case EFF_RAINBOW:
    if (lastcol32 > HUE_MAX)
      lastcol32 = 0;
    PixScrollDown();
    PixSet(PixBuffCount - 1, HueToRGB32(lastcol32, 255, 255));
    lastcol32++;
    break; //end EFF_RAINBOW
  case EFF_BLUE_SP:
    fase = random(25);
    PixFade(fase / 2);
    currcol32 = PixColor32(0, 0, 50 + random(180));
    for (int i = 0; i < fase; i++)
    {
      PixSet(lastled + random(fase), currcol32);
      lastled++;
    }
    if ((lastled % 8))
      PixSparkle(1, COL_WHITE);
    break; //end EFF_BLUE_SP
  case EFF_RED_SP:
    fase= random(PixBuffCount/4);
    PixFade(fase/3);
    currcol32 = PixColor32(random(255), 0, 0);
    PixSparkle(fase, currcol32);
    if (!(fase % 4)) PixSparkle(1, COL_YELOW);
    break; //end EFF_RED_SP
  case EFF_RAINBOW_SP:
    if (lastcol32 >= HUE_MAX)
      lastcol32 = 0;
    fase = 2 + random(10);
    PixFade(fase / 2);
    PixSparkle(fase, HueToRGB32(lastcol32++, 255, 200));
    break; //end EFF_RAINBOW_SP
  case EFF_RIB_RAND:
    if (lastled == 0)
    {
      fase = 4 + random(PixBuffCount / 10);
      lastcol32 = HueToRGB32(random(HUE_MAX), 255, 200);
      PixSmooth();
    }
    for (currled = 0; currled < 3; currled++)
    {
      PixSet(lastled + currled *fase, lastcol32);
      lastled++;
    }
    break; //end EFF_RIB_RAND
  case EFF_TWO_RAINBOW:
    PixFade(lastled / 20);
    if (lastcol32 > HUE_MAX)
      lastcol32 = 0;
    PixSet(PixBuffCount - lastled, HueToRGB32(lastcol32 + (180 * HUE_RES), 255, 255));
    PixSet(lastled, HueToRGB32(lastcol32, 255, 255));
    lastled++;
    lastcol32++;
    break; //end EFF_TWO_RAINBOW
  case EFF_TWO_RG:
    PixFade(lastled / 20);
    PixSet(PixBuffCount - lastled, COL_GREN);
    PixSet(lastled, COL_RED);
    lastled++;
    break; //end EFF_TWO_RG
  case EFF_TWO_COMETS:
    if (lastled > (PixBuffCount / 2))
    {
      lastled = 0;
      lastcol32 = random(HUE_MAX);
    }
    lastled++;
    PixFade(8);
    if (lastled % 2)
      break;
    for (currled = 0; currled < 8; currled++)
    {
      currcol32 = HueToRGB32(lastcol32, 255, 255 - currled * 16);
      PixSet(lastled - currled, currcol32);
      PixSet(lastled + PixBuffCount / 2 - currled, currcol32);
    }
    break; //end EFF_TWO_COMETS
  case EFF_COMET_RAND:
    if (lastled == 0)
    {
      lastcol32 = random(HUE_MAX);
      lastled = PixBuffCount - 1;
      colsat = 255 / (PixBuffCount / 10);
      PixSparkle((PixBuffCount / 10), HueToRGB32(lastcol32 + 180 * HUE_RES, 255, 255));
    }
    PixFade(2);
    colval = 0;
    for (currled = PixBuffCount / 10; currled > 0; currled--)
    {
      currcol32 = HueToRGB32(lastcol32, 255, colval);
      PixSet(lastled + currled, currcol32);
      colval += colsat;
    }
    lastled--;
    break; //end EFF_COMET_RAND
  }
  lasteffect = effno;
}
//-------------------------------------------------------
inline uint32_t PixColor32(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}
//--------
void PixFade(uint8_t steps)
{
  int32_t r, g, b;
  un_color32 cc;
  if (steps == 0)
    return;
  for (uint16_t i = 0; i < PixBuffCount; i++)
  {
    cc = PixGet(i);
    r = cc.c8.r;
    g = cc.c8.g;
    b = cc.c8.b;
    r = r - steps;
    g = g - steps;
    b = b - steps;
    if (r < 0)
      r = 0;
    if (g < 0)
      g = 0;
    if (b < 0)
      b = 0;
    PixSet(i, r, g, b);
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