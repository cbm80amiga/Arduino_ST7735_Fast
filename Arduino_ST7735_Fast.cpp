// Fast ST7735 128x160 1.8" display library
// (c) 2019 by Pawel A. Hernik

#include "Arduino_ST7735_Fast.h"
#include <limits.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>

// Initialization commands for ST7735 128x160 1.8" IPS
// taken from Adafruit
static const uint8_t PROGMEM
Rcmd1[] = {                       // 7735R init, part 1 (red or green tab)
  15,                             // 15 commands in list:
  ST7735_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
  150,                          //     150 ms delay
  ST7735_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
  255,                          //     500 ms delay
  ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
  0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
  ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
  0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
  ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
  0x01, 0x2C, 0x2D,             //     Dot inversion mode
  0x01, 0x2C, 0x2D,             //     Line inversion mode
  ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
  0x07,                         //     No inversion
  ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
  0xA2,
  0x02,                         //     -4.6V
  0x84,                         //     AUTO mode
  ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
  0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
  ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
  0x0A,                         //     Opamp current small
  0x00,                         //     Boost frequency
  ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
  0x8A,                         //     BCLK/2,
  0x2A,                         //     opamp current small & medium low
  ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
  0x8A, 0xEE,
  ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
  0x0E,
  ST7735_INVOFF,  0,              // 13: Don't invert display, no args
  ST7735_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
  0xC8,                         //     row/col addr, bottom-top refresh
  ST7735_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
  0x05 },
      
Rcmd2red[] = {                    // 7735R init, part 2 (red tab only)
  2,                              //  2 commands in list:
  ST7735_CASET,   4,              //  1: Column addr set, 4 args, no delay:
  0x00, 0x00,                   //     XSTART = 0
  0x00, 0x7F,                   //     XEND = 127
  ST7735_RASET,   4,              //  2: Row addr set, 4 args, no delay:
  0x00, 0x00,                   //     XSTART = 0
  0x00, 0x9F },
      
Rcmd3[] = {                       // 7735R init, part 3 (red or green tab)
  4,                              //  4 commands in list:
  ST7735_GMCTRP1, 16      ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
  0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
  0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
  0x29, 0x25, 0x2B, 0x39,
  0x00, 0x01, 0x03, 0x10,
  ST7735_GMCTRN1, 16      ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
  0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
  0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
  0x2E, 0x2E, 0x37, 0x3F,
  0x00, 0x00, 0x02, 0x10,
  ST7735_NORON,     ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
  10,                           //     10 ms delay
  ST7735_DISPON,    ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
  100 };         

#ifdef COMPATIBILITY_MODE
static SPISettings spiSettings;
#define SPI_START  SPI.beginTransaction(spiSettings)
#define SPI_END    SPI.endTransaction()
#else
#define SPI_START
#define SPI_END
#endif

// macros for fast DC and CS state changes
#ifdef COMPATIBILITY_MODE
#define DC_DATA     digitalWrite(dcPin, HIGH)
#define DC_COMMAND  digitalWrite(dcPin, LOW)
#define CS_IDLE     digitalWrite(csPin, HIGH)
#define CS_ACTIVE   digitalWrite(csPin, LOW)
#else
#define DC_DATA    *dcPort |= dcMask
#define DC_COMMAND *dcPort &= ~dcMask
#define CS_IDLE    *csPort |= csMask
#define CS_ACTIVE  *csPort &= ~csMask
#endif

// if CS always connected to the ground then don't do anything for better performance
#ifdef CS_ALWAYS_LOW
#define CS_IDLE
#define CS_ACTIVE
#endif

// ----------------------------------------------------------
// speed test results:
// in AVR best performance mode -> about 6.9 Mbps
// in compatibility mode (SPI.transfer(c)) -> about 4 Mbps
inline void Arduino_ST7735::writeSPI(uint8_t c) 
{
#ifdef COMPATIBILITY_MODE
    SPI.transfer(c);
#else
    SPDR = c;
    asm volatile("nop"); // 8 NOPs seem to be enough for 16MHz AVR @ DIV2 to avoid using while loop
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    //while(!(SPSR & _BV(SPIF))) ;
#endif
}

// ----------------------------------------------------------
Arduino_ST7735::Arduino_ST7735(int8_t dc, int8_t rst, int8_t cs) : Adafruit_GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT) 
{
  csPin = cs;
  dcPin = dc;
  rstPin = rst;
}

// ----------------------------------------------------------
void Arduino_ST7735::init() 
{
  pinMode(dcPin, OUTPUT);
#ifndef CS_ALWAYS_LOW
	pinMode(csPin, OUTPUT);
#endif

#ifndef COMPATIBILITY_MODE
  dcPort = portOutputRegister(digitalPinToPort(dcPin));
  dcMask = digitalPinToBitMask(dcPin);
#ifndef CS_ALWAYS_LOW
	csPort = portOutputRegister(digitalPinToPort(csPin));
	csMask = digitalPinToBitMask(csPin);
#endif
#endif

  SPI.begin();
#ifdef COMPATIBILITY_MODE
  spiSettings = SPISettings(16000000, MSBFIRST, SPI_MODE3);  // 8000000 gives max speed on AVR 16MHz
#else
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE3);
#endif

  CS_ACTIVE;
  if(rstPin != -1) {
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(50);
    digitalWrite(rstPin, LOW);
    delay(50);
    digitalWrite(rstPin, HIGH);
    delay(50);
  }

  _colstart = 0;
  _rowstart = 0;
  _width  = ST7735_TFTWIDTH;
  _height = ST7735_TFTHEIGHT;
  displayInit(Rcmd1);
  displayInit(Rcmd2red);
  displayInit(Rcmd3);
  setRotation(2);
}

// ----------------------------------------------------------
void Arduino_ST7735::writeCmd(uint8_t c) 
{
  DC_COMMAND;
  CS_ACTIVE;
  SPI_START;

  writeSPI(c);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void Arduino_ST7735::writeData(uint8_t c) 
{
  DC_DATA;
  CS_ACTIVE;
  SPI_START;
    
  writeSPI(c);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void Arduino_ST7735::displayInit(const uint8_t *addr) 
{
  uint8_t  numCommands, numArgs;
  uint16_t ms;
  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writeCmd(pgm_read_byte(addr++));     //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & ST_CMD_DELAY;   //   If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;            //   Mask out delay bit
    while(numArgs--) writeData(pgm_read_byte(addr++));

    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
}


// ----------------------------------------------------------
void Arduino_ST7735::setRotation(uint8_t m) 
{
  writeCmd(ST7735_MADCTL);
  rotation = m & 3;
  switch (rotation) {
   case 0:
     writeData(ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB);
     _xstart = _colstart;
     _ystart = _rowstart;
     _height = ST7735_TFTHEIGHT;
     _width  = ST7735_TFTWIDTH;
     break;
   case 1:
     writeData(ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB);
     _ystart = _colstart;
     _xstart = _rowstart;
     _width  = ST7735_TFTHEIGHT;
     _height = ST7735_TFTWIDTH;
     break;
  case 2:
     writeData(ST7735_MADCTL_RGB);
     _xstart = _colstart;
     _ystart = _rowstart;
     _height = ST7735_TFTHEIGHT;
     _width  = ST7735_TFTWIDTH;
     break;
   case 3:
     writeData(ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB);
     _ystart = _colstart;
     _xstart = _rowstart;
     _width  = ST7735_TFTHEIGHT;
     _height = ST7735_TFTWIDTH;
     break;
  }
}

// ----------------------------------------------------------
void Arduino_ST7735::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) 
{
  uint16_t xs = x0 + _xstart, xe = x1 + _xstart;
  uint16_t ys = y0 + _ystart, ye = y1 + _ystart;
  // optimized version
  CS_ACTIVE;
  SPI_START;
  
  DC_COMMAND; writeSPI(ST7735_CASET);
  DC_DATA;
  writeSPI(xs >> 8); writeSPI(xs & 0xFF);
  writeSPI(xe >> 8); writeSPI(xe & 0xFF);

  DC_COMMAND; writeSPI(ST7735_RASET);
  DC_DATA;
  writeSPI(ys >> 8); writeSPI(ys & 0xFF);
  writeSPI(ye >> 8); writeSPI(ye & 0xFF);

  DC_COMMAND; writeSPI(ST7735_RAMWR);
  
  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void Arduino_ST7735::pushColor(uint16_t color) 
{
  SPI_START;
  DC_DATA;
  CS_ACTIVE;

  writeSPI(color >> 8); writeSPI(color);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void Arduino_ST7735::drawPixel(int16_t x, int16_t y, uint16_t color) 
{
  if(x<0 ||x>=_width || y<0 || y>=_height) return;
  setAddrWindow(x,y,x+1,y+1);

  SPI_START;
  DC_DATA;
  CS_ACTIVE;

  writeSPI(color >> 8); writeSPI(color);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void Arduino_ST7735::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
  if(x>=_width || y>=_height || h<=0) return;
  if(y+h-1>=_height) h=_height-y;
  setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;
    
  SPI_START;
  DC_DATA;
  CS_ACTIVE;

  uint8_t num8 = h>>3;
  while(num8--) {
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
  }
  num8 = (uint8_t)h & 7;
  while(num8--) { writeSPI(hi); writeSPI(lo); }

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void Arduino_ST7735::drawFastHLine(int16_t x, int16_t y, int16_t w,  uint16_t color) 
{
  if(x>=_width || y>=_height || w<=0) return;
  if(x+w-1>=_width)  w=_width-x;
  setAddrWindow(x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;

  SPI_START;
  DC_DATA;
  CS_ACTIVE;

  uint8_t num8 = w>>3;
  while(num8--) {
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
  }
  num8 = (uint8_t)w & 7;
  while(num8--) { writeSPI(hi); writeSPI(lo); }

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void Arduino_ST7735::fillScreen(uint16_t color) 
{
  fillRect(0, 0,  _width, _height, color);
}

// ----------------------------------------------------------
void Arduino_ST7735::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
  if(x>=_width || y>=_height || w<=0 || h<=0) return;
  if(x+w-1>=_width)  w=_width -x;
  if(y+h-1>=_height) h=_height-y;
  setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;
    
  SPI_START;
  DC_DATA;
  CS_ACTIVE;

  uint32_t num = (uint32_t)w*h;
  uint16_t num16 = num>>4;
  while(num16--) {
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
    writeSPI(hi); writeSPI(lo);
  }
  uint8_t num8 = num & 0xf;
  while(num8--) { writeSPI(hi); writeSPI(lo); }

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
// draws image from RAM
void Arduino_ST7735::drawImage(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *img16) 
{
  if(x>=_width || y>=_height || w<=0 || h<=0) return;
  setAddrWindow(x, y, x+w-1, y+h-1);

  SPI_START;
  DC_DATA;
  CS_ACTIVE;

  uint32_t num = (uint32_t)w*h;
  uint16_t num16 = num>>3;
  uint8_t *img = (uint8_t *)img16;
  while(num16--) {
    writeSPI(*(img+1)); writeSPI(*(img+0)); img+=2;
    writeSPI(*(img+1)); writeSPI(*(img+0)); img+=2;
    writeSPI(*(img+1)); writeSPI(*(img+0)); img+=2;
    writeSPI(*(img+1)); writeSPI(*(img+0)); img+=2;
    writeSPI(*(img+1)); writeSPI(*(img+0)); img+=2;
    writeSPI(*(img+1)); writeSPI(*(img+0)); img+=2;
    writeSPI(*(img+1)); writeSPI(*(img+0)); img+=2;
    writeSPI(*(img+1)); writeSPI(*(img+0)); img+=2;
  }
  uint8_t num8 = num & 0x7;
  while(num8--) { writeSPI(*(img+1)); writeSPI(*(img+0)); img+=2; }

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
// draws image from flash (PROGMEM)
void Arduino_ST7735::drawImageF(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *img16) 
{
  if(x>=_width || y>=_height || w<=0 || h<=0) return;
  setAddrWindow(x, y, x+w-1, y+h-1);

  SPI_START;
  DC_DATA;
  CS_ACTIVE;

  uint32_t num = (uint32_t)w*h;
  uint16_t num16 = num>>3;
  uint8_t *img = (uint8_t *)img16;
  while(num16--) {
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
  }
  uint8_t num8 = num & 0x7;
  while(num8--) { writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2; }

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Arduino_ST7735::Color565(uint8_t r, uint8_t g, uint8_t b) 
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// ----------------------------------------------------------
void Arduino_ST7735::invertDisplay(boolean mode) 
{
  writeCmd(mode ? ST7735_INVON : ST7735_INVOFF);
}

// ----------------------------------------------------------
void Arduino_ST7735::partialDisplay(boolean mode) 
{
  writeCmd(mode ? ST7735_PTLON : ST7735_NORON);
}

// ----------------------------------------------------------
void Arduino_ST7735::sleepDisplay(boolean mode) 
{
  writeCmd(mode ? ST7735_SLPIN : ST7735_SLPOUT);
  delay(5);
}

// ----------------------------------------------------------
void Arduino_ST7735::enableDisplay(boolean mode) 
{
  writeCmd(mode ? ST7735_DISPON : ST7735_DISPOFF);
}

// ----------------------------------------------------------
void Arduino_ST7735::idleDisplay(boolean mode) 
{
  writeCmd(mode ? ST7735_IDMON : ST7735_IDMOFF);
}

// ----------------------------------------------------------
void Arduino_ST7735::resetDisplay() 
{
  writeCmd(ST7735_SWRESET);
  delay(5);
}

// ----------------------------------------------------------
void Arduino_ST7735::setScrollArea(uint16_t tfa, uint16_t bfa) 
{
  uint16_t vsa = 160-tfa-bfa;
  writeCmd(ST7735_VSCRDEF);
  writeData(tfa >> 8); writeData(tfa);
  writeData(vsa >> 8); writeData(vsa);
  writeData(bfa >> 8); writeData(bfa);
}

// ----------------------------------------------------------
void Arduino_ST7735::setScroll(uint16_t vsp) 
{
  writeCmd(ST7735_VSCRSADD);
  writeData(vsp >> 8); writeData(vsp);
}

// ----------------------------------------------------------
void Arduino_ST7735::setPartArea(uint16_t sr, uint16_t er) 
{
  writeCmd(ST7735_PTLAR);
  writeData(sr >> 8); writeData(sr);
  writeData(er >> 8); writeData(er);
}

// ----------------------------------------------------------
// doesn't work
void Arduino_ST7735::setBrightness(uint8_t br) 
{
}

// ----------------------------------------------------------
// 0 - off
// 1 - idle
// 2 - normal
// 4 - display off
void Arduino_ST7735::powerSave(uint8_t mode) 
{
  if(mode==0) {
    writeCmd(ST7735_POWSAVE);
    writeData(0xec|3);
    writeCmd(ST7735_DLPOFFSAVE);
    writeData(0xff);
    return;
  }
  int is = (mode&1) ? 0 : 1;
  int ns = (mode&2) ? 0 : 2;
  writeCmd(ST7735_POWSAVE);
  writeData(0xec|ns|is);
  if(mode&4) {
    writeCmd(ST7735_DLPOFFSAVE);
    writeData(0xfe);
  }
}

// ------------------------------------------------
// Input a value 0 to 511 (85*6) to get a color value.
// The colours are a transition R - Y - G - C - B - M - R.
void Arduino_ST7735::rgbWheel(int idx, uint8_t *_r, uint8_t *_g, uint8_t *_b)
{
  idx &= 0x1ff;
  if(idx < 85) { // R->Y  
    *_r = 255; *_g = idx * 3; *_b = 0;
    return;
  } else if(idx < 85*2) { // Y->G
    idx -= 85*1;
    *_r = 255 - idx * 3; *_g = 255; *_b = 0;
    return;
  } else if(idx < 85*3) { // G->C
    idx -= 85*2;
    *_r = 0; *_g = 255; *_b = idx * 3;
    return;  
  } else if(idx < 85*4) { // C->B
    idx -= 85*3;
    *_r = 0; *_g = 255 - idx * 3; *_b = 255;
    return;    
  } else if(idx < 85*5) { // B->M
    idx -= 85*4;
    *_r = idx * 3; *_g = 0; *_b = 255;
    return;    
  } else { // M->R
    idx -= 85*5;
    *_r = 255; *_g = 0; *_b = 255 - idx * 3;
   return;
  }
} 

uint16_t Arduino_ST7735::rgbWheel(int idx)
{
  uint8_t r,g,b;
  rgbWheel(idx, &r,&g,&b);
  return RGBto565(r,g,b);
}

// ------------------------------------------------