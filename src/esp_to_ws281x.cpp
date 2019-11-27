#ifndef _ESP_TO_WS_CPP_
#define _ESP_TO_WS_CPP_

#include "esp_to_ws281x.h"

#define LEDPACK_MAX 5
#define LEDBITSTOSEND(x) (x * 3 * sizeof(uint32_t) * 8) //calc bits to send per led (3xuint32)

#if (CPU_CLK_FREQ == 160000000) || (F_CPU == 160000000)
//for turbo ESP 160MHz
#define SPI_PREDIV 8 //10
#define SPI_CNTDIV 5
#warning "SPI set for 160MHz"
#else
//for standard ESP8266 80MHz clock
#define SPI_PREDIV 5  //5
#define SPI_CNTDIV 5
#warning "SPI set for 80MHz"
#endif

void prepareHSPI()
{
  //init GPIO
  //PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); //GPIO12 is HSPI MISO pin (Master Data In)
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); //GPIO13 is HSPI MOSI pin (Master Data Out) - this pin generate WS281x signal
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); //GPIO14 is HSPI CLK pin (Clock)
  //PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2); //GPIO15 is HSPI CS pin (Chip Select / Slave Select)
  //HSPI mode
  CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_IDLE_EDGE);
  CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_CK_OUT_EDGE);
  //set HSPI clock
  WRITE_PERI_REG(SPI_CLOCK(HSPI),
                 (((SPI_PREDIV - 1) & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) |
                     (((SPI_CNTDIV - 1) & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) |
                     (((SPI_CNTDIV >> 1) & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) |
                     ((0 & SPI_CLKCNT_L) << SPI_CLKCNT_L_S));
  //MSB
  SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_WR_BYTE_ORDER);
  //SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_CS_SETUP | SPI_CS_HOLD);
  CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_FLASH_MODE);
}
//---------------------------------
inline uint32_t setNibble(uint32_t innumber, uint32_t nibble, uint8_t nonibb)
{
  nibble = (nibble & 0xF) << (4 * nonibb);
  innumber = innumber & ~(0xF << (4 * nonibb));
  return (innumber | nibble);
}
//---------------------------------
uint32_t byteToSPIData(uint8_t val)
{
  uint32_t ret = 0;

  for (uint8_t i = 0; i < 8; i++)
  {
    if (bitRead(val, i))
      ret = setNibble(ret, 0b1100, i); //ws 1
    else
      ret = setNibble(ret, 0b1000, i); //ws 0
  }
  return ret;
}
//----------------------------------
void prepareSPIpacket(un_color24 *ledcolor, uint32_t *wspack, uint8_t neo_type, uint32_t powerfactor)
{
  if (powerfactor < 100)
  {
    ledcolor->r = (ledcolor->r * powerfactor) / 100;
    ledcolor->g = (ledcolor->g * powerfactor) / 100;
    ledcolor->b = (ledcolor->b * powerfactor) / 100;
  };
  switch (neo_type)
  {
  case NEO_BGR:
    wspack[0] = byteToSPIData(ledcolor->b);
    wspack[1] = byteToSPIData(ledcolor->g);
    wspack[2] = byteToSPIData(ledcolor->r);
    break;
  case NEO_BRG:
    wspack[0] = byteToSPIData(ledcolor->b);
    wspack[1] = byteToSPIData(ledcolor->r);
    wspack[2] = byteToSPIData(ledcolor->g);
    break;
  case NEO_GBR:
    wspack[0] = byteToSPIData(ledcolor->g);
    wspack[1] = byteToSPIData(ledcolor->b);
    wspack[2] = byteToSPIData(ledcolor->r);
    break;
  case NEO_GRB:
    wspack[0] = byteToSPIData(ledcolor->g);
    wspack[1] = byteToSPIData(ledcolor->r);
    wspack[2] = byteToSPIData(ledcolor->b);
    break;
  case NEO_RBG:
    wspack[0] = byteToSPIData(ledcolor->r);
    wspack[1] = byteToSPIData(ledcolor->b);
    wspack[2] = byteToSPIData(ledcolor->g);
    break;
  default:
    wspack[0] = byteToSPIData(ledcolor->r);
    wspack[1] = byteToSPIData(ledcolor->g);
    wspack[2] = byteToSPIData(ledcolor->b);
  }
}

//----------------------------------
void sendSPI(un_color24 *rgbdata24, uint32_t pixelcount, uint8_t neo_type, uint8_t powerfactor)
{
  uint32_t wspack[3 * LEDPACK_MAX]; //use 60 from fifo spi 64bytes 1color (1byte)= 1 uint32 (4byte)
  uint32_t i = 0;
  //wait for spi and setup
  while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR)
  {
  };
  CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI | SPI_USR_MISO | SPI_USR_COMMAND | SPI_USR_ADDR | SPI_USR_DUMMY);
  SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI);

  while (i < pixelcount)
  {
    uint8_t ledinpack = 0;
    if ((i + LEDPACK_MAX) < pixelcount)
    {
      while (ledinpack < LEDPACK_MAX)
      {
        prepareSPIpacket(&rgbdata24[i], &wspack[ledinpack * 3], neo_type, powerfactor);
        ledinpack++;
        i++;
      }
    }
    else
    {
      while (i < pixelcount)
      {
        prepareSPIpacket(&rgbdata24[i], &wspack[ledinpack * 3], neo_type, powerfactor);
        ledinpack++;
        i++;
      }
    }
    //wait for hspi
    while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR)
    {
    };
    //Number of bits to send via MOSI
    WRITE_PERI_REG(SPI_USER1(HSPI), (((LEDBITSTOSEND(ledinpack) - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S));
    //copy to HSPI fifo
    for (uint8_t lc = 0; lc < (ledinpack * 3); lc++)
    {
      *(uint32_t *)(SPI_W0(HSPI) + (lc * 4)) = (uint32_t)(wspack[lc]);
    }

    SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR); //toggle spi sending

  } // end while i
}

#ifdef _un_color32_
void sendSPI(un_color32 *rgbdata32, uint32_t pixelcount, uint8_t neo_type, uint8_t powerfactor)
{
  uint32_t wspack[3 * LEDPACK_MAX]; //use 60 from fifo spi 64bytes 1color (1byte)= 1 uint32 (4byte)
  uint32_t i = 0;
  un_color24 c24; //to prepare packet
  un_color32 c32; //local
  //wait for spi and setup
  while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR)
  {
  };
  CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI | SPI_USR_MISO | SPI_USR_COMMAND | SPI_USR_ADDR | SPI_USR_DUMMY);
  SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI);

  while (i < pixelcount)
  {
    uint8_t ledinpack = 0;
    if ((i + LEDPACK_MAX) < pixelcount)
    {
      while (ledinpack < LEDPACK_MAX)
      {
        c32 = rgbdata32[i];
        c24.b = c32.c8.b;
        c24.g = c32.c8.g;
        c24.r = c32.c8.r;
        prepareSPIpacket(&c24, &wspack[ledinpack * 3], neo_type, powerfactor);
        ledinpack++;
        i++;
      }
    }
    else
    {
      while (i < pixelcount)
      {
        prepareSPIpacket(&c24, &wspack[ledinpack * 3], neo_type, powerfactor);
        ledinpack++;
        i++;
      }
    }
    //wait for hspi
    while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR)
    {
    };
    //Number of bits to send via MOSI
    WRITE_PERI_REG(SPI_USER1(HSPI), (((LEDBITSTOSEND(ledinpack) - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S));
    //copy to HSPI fifo
    for (uint8_t lc = 0; lc < (ledinpack * 3); lc++)
    {
      *(uint32_t *)(SPI_W0(HSPI) + (lc * 4)) = (uint32_t)(wspack[lc]);
    }

    SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR); //toggle spi sending

  } // end while i
}
#endif

#endif
