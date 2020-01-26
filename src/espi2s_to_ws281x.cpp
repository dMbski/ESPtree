#ifndef _ESPI2S_TO_WS_CPP_
#define _ESPI2S_TO_WS_CPP_
//its all copied (some changes to work with dynamically changed type and count leds)
//  from NeoPixel library - (see https://github.com/Makuna/NeoPixelBus)
//  MUUch thx for Creator of NeoPixelBus library
#include "Arduino.h"
#include "osapi.h"
#include "ets_sys.h"

#include "i2s_reg.h"
#include "i2s.h"
#include "eagle_soc.h"
#include "esp8266_peri.h"
#include "slc_register.h"

#include "osapi.h"
#include "ets_sys.h"
#include "user_interface.h"
#include "espi2s_to_ws281x.h"

#define BytesPerPixel 3

const static uint32_t I2sClockDivisor = 7; //ws2812 3  ws2811  5
const static uint32_t I2sBaseClockDivisor = 8; //ws2812 16  ws2811 16

const uint16_t c_maxDmaBlockSize = 4095;
const uint16_t c_dmaBytesPerPixelBytes = 4;

//buffer for i2s cache
uint8_t *_i2sBuffer;
uint32_t _i2sBufferSize;
uint8_t _i2sZeroes[24L / 6L];

struct slc_queue_item
{
  uint32 blocksize : 12;
  uint32 datalen : 12;
  uint32 unused : 5;
  uint32 sub_sof : 1;
  uint32 eof : 1;
  uint32 owner : 1;
  uint32 buf_ptr;
  uint32 next_link_ptr;
};

slc_queue_item *_i2sBufDesc;
uint16_t _i2sBufDescCount;

enum NeoDmaState
{
  NeoDmaState_Idle,
  NeoDmaState_Pending,
  NeoDmaState_Sending,
  NeoDmaState_Zeroing,
};

volatile NeoDmaState _dmaState;

// This routine is called as soon as the DMA routine has something to tell us. All we
// handle here is the RX_EOF_INT status, which indicate the DMA has sent a buffer whose
// descriptor has the 'EOF' field set to 1.
// in the case of this code, the second to last state descriptor
volatile static void ICACHE_RAM_ATTR i2s_slc_isr(void)
{
  ETS_SLC_INTR_DISABLE();

  uint32_t slc_intr_status = SLCIS;

  SLCIC = 0xFFFFFFFF;

  if ((slc_intr_status & SLCIRXEOF))
  {
    switch (_dmaState)
    {
    case NeoDmaState_Idle:
      break;

    case NeoDmaState_Pending:
    {
      slc_queue_item *finished_item = (slc_queue_item *)SLCRXEDA;

      // data block has pending data waiting to send, prepare it
      // point last state block to top
      (finished_item + 1)->next_link_ptr = (uint32_t)(_i2sBufDesc);

      _dmaState = NeoDmaState_Sending;
    }
    break;

    case NeoDmaState_Sending:
    {
      slc_queue_item *finished_item = (slc_queue_item *)SLCRXEDA;

      // the data block had actual data sent
      // point last state block to first state block thus
      // just looping and not sending the data blocks
      (finished_item + 1)->next_link_ptr = (uint32_t)(finished_item);

      _dmaState = NeoDmaState_Zeroing;
    }
    break;

    case NeoDmaState_Zeroing:
      _dmaState = NeoDmaState_Idle;
      break;
    }
  }

  ETS_SLC_INTR_ENABLE();
}

void StopDma()
{
  ETS_SLC_INTR_DISABLE();

  // Disable any I2S send or receive
  I2SC &= ~(I2STXS | I2SRXS);

  // Reset I2S
  I2SC &= ~(I2SRST);
  I2SC |= I2SRST;
  I2SC &= ~(I2SRST);

  SLCIC = 0xFFFFFFFF;
  SLCIE = 0;
  SLCTXL &= ~(SLCTXLAM << SLCTXLA); // clear TX descriptor address
  SLCRXL &= ~(SLCRXLAM << SLCRXLA); // clear RX descriptor address
}

bool IsReadyToUpdate()
{
  return (_dmaState == NeoDmaState_Idle);
}

void prepareI2S(uint32_t pixelcount)
{
  StopDma();
  pinMode(3, FUNCTION_1); // I2S0_DATA

  uint16_t dmaPixelSize = c_dmaBytesPerPixelBytes * BytesPerPixel;

  _i2sBufferSize = pixelcount * dmaPixelSize;
  _i2sBuffer = (uint8_t *)malloc(_i2sBufferSize);
  memset(_i2sBuffer, 0, _i2sBufferSize);
  memset(_i2sZeroes, 0, sizeof(_i2sZeroes));

  uint16_t _is2BufMaxBlockSize = (c_maxDmaBlockSize / dmaPixelSize) * dmaPixelSize;

  _i2sBufDescCount = (_i2sBufferSize / _is2BufMaxBlockSize) + 1 + 2; // need two more for state/latch blocks
  _i2sBufDesc = (slc_queue_item *)malloc(_i2sBufDescCount * sizeof(slc_queue_item));

  uint8_t *is2Buffer = _i2sBuffer;
  uint32_t is2BufferSize = _i2sBufferSize;
  uint16_t indexDesc;
  // prepare main data block decriptors that point into our one static dma buffer
  for (indexDesc = 0; indexDesc < (_i2sBufDescCount - 2); indexDesc++)
  {
    uint32_t blockSize = (is2BufferSize > _is2BufMaxBlockSize) ? _is2BufMaxBlockSize : is2BufferSize;

    _i2sBufDesc[indexDesc].owner = 1;
    _i2sBufDesc[indexDesc].eof = 0; // no need to trigger interrupt generally
    _i2sBufDesc[indexDesc].sub_sof = 0;
    _i2sBufDesc[indexDesc].datalen = blockSize;
    _i2sBufDesc[indexDesc].blocksize = blockSize;
    _i2sBufDesc[indexDesc].buf_ptr = (uint32_t)is2Buffer;
    _i2sBufDesc[indexDesc].unused = 0;
    _i2sBufDesc[indexDesc].next_link_ptr = (uint32_t) & (_i2sBufDesc[indexDesc + 1]);

    is2Buffer += blockSize;
    is2BufferSize -= blockSize;
  }
  for (; indexDesc < _i2sBufDescCount; indexDesc++)
  {
    _i2sBufDesc[indexDesc].owner = 1;
    _i2sBufDesc[indexDesc].eof = 0; // no need to trigger interrupt generally
    _i2sBufDesc[indexDesc].sub_sof = 0;
    _i2sBufDesc[indexDesc].datalen = sizeof(_i2sZeroes);
    _i2sBufDesc[indexDesc].blocksize = sizeof(_i2sZeroes);
    _i2sBufDesc[indexDesc].buf_ptr = (uint32_t)_i2sZeroes;
    _i2sBufDesc[indexDesc].unused = 0;
    _i2sBufDesc[indexDesc].next_link_ptr = (uint32_t) & (_i2sBufDesc[indexDesc + 1]);
  }
  // the first state block will trigger the interrupt
  _i2sBufDesc[indexDesc - 2].eof = 1;
  // the last state block will loop to the first state block by defualt
  _i2sBufDesc[indexDesc - 1].next_link_ptr = (uint32_t) & (_i2sBufDesc[indexDesc - 2]);

  // setup the rest of i2s DMA
  //
  ETS_SLC_INTR_DISABLE();
  // start off in sending state as that is what it will be all setup to be
  // for the interrupt
  _dmaState = NeoDmaState_Sending;

  SLCC0 |= SLCRXLR | SLCTXLR;
  SLCC0 &= ~(SLCRXLR | SLCTXLR);
  SLCIC = 0xFFFFFFFF;

  // Configure DMA
  SLCC0 &= ~(SLCMM << SLCM);                    // clear DMA MODE
  SLCC0 |= (1 << SLCM);                         // set DMA MODE to 1
  SLCRXDC |= SLCBINR | SLCBTNR;                 // enable INFOR_NO_REPLACE and TOKEN_NO_REPLACE
  SLCRXDC &= ~(SLCBRXFE | SLCBRXEM | SLCBRXFM); // disable RX_FILL, RX_EOF_MODE and RX_FILL_MODE

  // Feed DMA the 1st buffer desc addr
  // To send data to the I2S subsystem, counter-intuitively we use the RXLINK part, not the TXLINK as you might
  // expect. The TXLINK part still needs a valid DMA descriptor, even if it's unused: the DMA engine will throw
  // an error at us otherwise. Just feed it any random descriptor.
  SLCTXL &= ~(SLCTXLAM << SLCTXLA); // clear TX descriptor address
  // set TX descriptor address. any random desc is OK, we don't use TX but it needs to be valid
  SLCTXL |= (uint32) & (_i2sBufDesc[_i2sBufDescCount - 1]) << SLCTXLA;
  SLCRXL &= ~(SLCRXLAM << SLCRXLA); // clear RX descriptor address
  // set RX descriptor address.  use first of the data addresses
  SLCRXL |= (uint32) & (_i2sBufDesc[0]) << SLCRXLA;

  ETS_SLC_INTR_ATTACH(i2s_slc_isr, NULL);
  SLCIE = SLCIRXEOF; // Enable only for RX EOF interrupt

  ETS_SLC_INTR_ENABLE();

  //Start transmission
  SLCTXL |= SLCTXLS;
  SLCRXL |= SLCRXLS;

  I2S_CLK_ENABLE();
  I2SIC = 0x3F;
  I2SIE = 0;

  //Reset I2S
  I2SC &= ~(I2SRST);
  I2SC |= I2SRST;
  I2SC &= ~(I2SRST);

  // Set RX/TX FIFO_MOD=0 and disable DMA (FIFO only)
  I2SFC &= ~(I2SDE | (I2STXFMM << I2STXFM) | (I2SRXFMM << I2SRXFM));
  I2SFC |= I2SDE; //Enable DMA
  // Set RX/TX CHAN_MOD=0
  I2SCC &= ~((I2STXCMM << I2STXCM) | (I2SRXCMM << I2SRXCM));

  // set the rate
  uint32_t i2s_clock_div = I2sClockDivisor & I2SCDM;
  uint8_t i2s_bck_div = I2sBaseClockDivisor & I2SBDM;

  //!trans master, !bits mod, rece slave mod, rece msb shift, right first, msb right
  I2SC &= ~(I2STSM | I2SRSM | (I2SBMM << I2SBM) | (I2SBDM << I2SBD) | (I2SCDM << I2SCD));
  I2SC |= I2SRF | I2SMR | I2SRSM | I2SRMS | (i2s_bck_div << I2SBD) | (i2s_clock_div << I2SCD);

  I2SC |= I2STXS; // Start transmission
}

//sets nibble (4bits) no nonibb (0-7) in innumber (32bits)
inline uint32_t ICACHE_RAM_ATTR setNibble(uint32_t innumber, uint32_t nibble, uint8_t nonibb)
{
  nibble = (nibble & 0xF) << (4 * nonibb);
  innumber = innumber & ~(0xF << (4 * nonibb));
  return (innumber | nibble);
}
//---------------------------------
uint32_t ICACHE_RAM_ATTR byteToWSData(uint8_t val)
{
  uint32_t ret = 0;
  for (uint8_t i = 0; i < 8; i++)
  {
    /*
    if (bitRead(val, i))
      ret = setNibble(ret, 0b1100, i); //ws 1
    else
      ret = setNibble(ret, 0b1000, i); //ws 0
      */

    ret = setNibble(ret, bitRead(val, i) ? 0b1100 : 0b1000, i);
  }

  return ret;
}

//// version for un_color32

inline void prepareI2Spacket(un_color32 *ledcolor, uint32_t *wspack, uint8_t neo_type, uint32_t powerfactor)
{
  if (powerfactor < 100)
  {
    ledcolor->c8.r = (ledcolor->c8.r * powerfactor) / 100;
    ledcolor->c8.g = (ledcolor->c8.g * powerfactor) / 100;
    ledcolor->c8.b = (ledcolor->c8.b * powerfactor) / 100;
  };
  switch (neo_type)
  {
  case NEO_BGR:
    wspack[0] = byteToWSData(ledcolor->c8.b);
    wspack[1] = byteToWSData(ledcolor->c8.g);
    wspack[2] = byteToWSData(ledcolor->c8.r);
    break;
  case NEO_BRG:
    wspack[0] = byteToWSData(ledcolor->c8.b);
    wspack[1] = byteToWSData(ledcolor->c8.r);
    wspack[2] = byteToWSData(ledcolor->c8.g);
    break;
  case NEO_GBR:
    wspack[0] = byteToWSData(ledcolor->c8.g);
    wspack[1] = byteToWSData(ledcolor->c8.b);
    wspack[2] = byteToWSData(ledcolor->c8.r);
    break;
  case NEO_GRB:
    wspack[0] = byteToWSData(ledcolor->c8.g);
    wspack[1] = byteToWSData(ledcolor->c8.r);
    wspack[2] = byteToWSData(ledcolor->c8.b);
    break;
  case NEO_RBG:
    wspack[0] = byteToWSData(ledcolor->c8.r);
    wspack[1] = byteToWSData(ledcolor->c8.b);
    wspack[2] = byteToWSData(ledcolor->c8.g);
    break;
  default:
    wspack[0] = byteToWSData(ledcolor->c8.r);
    wspack[1] = byteToWSData(ledcolor->c8.g);
    wspack[2] = byteToWSData(ledcolor->c8.b);
  }
}

void sendI2S(un_color32 *rgbdata32, uint32_t pixelcount, uint8_t neo_type, uint8_t powerfactor)
{
  while (!IsReadyToUpdate())
  {
    yield();
  }
  uint32_t *pDma = (uint32_t *)_i2sBuffer;

  for (uint16_t i = 0; i < pixelcount; i++)
  {
    prepareI2Spacket(&rgbdata32[i], &pDma[i * 3], neo_type, powerfactor);
    if (!(i % 100))
      yield();
  }

  // toggle state so the ISR reacts
  _dmaState = NeoDmaState_Pending;
}

#endif
