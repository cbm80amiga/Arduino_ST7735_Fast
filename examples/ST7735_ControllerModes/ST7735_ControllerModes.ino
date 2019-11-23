// ST7735 library example
// (c) 2019 Pawel A. Hernik

// requires RRE Font library:
// https://github.com/cbm80amiga/RREFont

/*
 ST7735 128x160 1.8" LCD pinout (header at the top, from left):
 #1 LED   -> 3.3V
 #2 SCK   -> SCL/D13/PA5
 #3 SDA   -> MOSI/D11/PA7
 #4 A0/DC -> D8/PA1  or any digital
 #5 RESET -> D9/PA0  or any digital
 #6 CS    -> D10/PA2 or any digital
 #7 GND   -> GND
 #8 VCC   -> 3.3V
*/

#define SCR_WD   128
#define SCR_HT   160
#include <SPI.h>
#include <Adafruit_GFX.h>

#if (__STM32F1__) // bluepill
#define TFT_CS  PA2
#define TFT_DC  PA1
#define TFT_RST PA0
//#include <Arduino_ST7735_STM.h>
#else
#define TFT_CS 10
#define TFT_DC  8
#define TFT_RST 9
#include <Arduino_ST7735_Fast.h>
#endif

Arduino_ST7735 lcd = Arduino_ST7735(TFT_DC, TFT_RST, TFT_CS);

#include "RREFont.h"
#include "rre_fjg_8x16.h"

RREFont font;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }

void setup() 
{
  Serial.begin(9600);
  lcd.init();
  lcd.fillScreen(BLACK);
  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
  font.setFont(&rre_fjg_8x16);
  font.setScale(1); font.setSpacing(3);
  font.setColor(WHITE);
}

void rainbow()
{
  for(int i=0;i<128;i+=4) {
    uint8_t r,g,b;
    lcd.rgbWheel(i*512L/128,&r,&g,&b);
    lcd.fillRect(0,i,128,4,RGBto565(r,g,b));
  }
}

void loop()
{
  lcd.fillScreen(RGBto565(120,60,30));
  font.printStr(ALIGN_CENTER,SCR_HT/2-8-20,"ST7735");
  font.printStr(ALIGN_CENTER,SCR_HT/2-8+20,"modes");
  delay(2000);

  lcd.fillScreen(BLACK);
  rainbow();
  font.setColor(BLACK);
  font.printStr(ALIGN_CENTER,50,"Idle mode OFF");
  lcd.idleDisplay(false); delay(2000);
  rainbow();
  font.printStr(ALIGN_CENTER,50,"Idle mode ON");
  lcd.idleDisplay(true); delay(4000);
  rainbow();
  font.printStr(ALIGN_CENTER,50,"Idle mode OFF");
  lcd.idleDisplay(false); delay(2000);

  rainbow();
  font.setColor(WHITE,BLACK);
  lcd.fillRect(10,46,SCR_WD-20,22,BLACK);
  font.printStr(ALIGN_CENTER,50,"Invert OFF");
  lcd.invertDisplay(false); delay(2000);
  font.printStr(ALIGN_CENTER,50," Invert ON ");
  lcd.invertDisplay(true); delay(4000);
  font.printStr(ALIGN_CENTER,50,"Invert OFF");
  lcd.invertDisplay(false); delay(2000);

  font.setColor(WHITE);
  lcd.fillScreen(RGBto565(180,0,180));
  font.printStr(ALIGN_CENTER,SCR_HT/2-8,"Sleep mode 2s");
  delay(2000);
  //lcd.enableDisplay(false); 
  lcd.sleepDisplay(true); delay(4000);
  lcd.sleepDisplay(false); 
  //lcd.enableDisplay(true);

  lcd.fillScreen(RGBto565(180,0,180));
  font.printStr(ALIGN_CENTER,SCR_HT/2-8,"Display on/off");
  delay(2000);
  lcd.enableDisplay(false);  delay(4000);
  lcd.enableDisplay(true);  delay(1000);

  lcd.fillScreen(RGBto565(180,0,180));
  font.printStr(ALIGN_CENTER,SCR_HT/2-8,"Partial display");
  font.setColor(YELLOW);
  font.printStr(ALIGN_CENTER,4,"Top");
  font.printStr(ALIGN_CENTER,SCR_HT-18,"Bottom");
  font.setColor(WHITE);
  delay(2000);
  lcd.setPartArea(SCR_HT/4, SCR_HT-SCR_HT/4); lcd.partialDisplay(true); delay(4000);
  lcd.setPartArea(SCR_HT-SCR_HT/4, SCR_HT/4); lcd.partialDisplay(true); delay(4000);
  lcd.partialDisplay(false);
  delay(1000);
  
  lcd.fillScreen(RGBto565(180,0,0));
  font.printStr(ALIGN_CENTER,SCR_HT/2-8,"Sw reset ...");
  delay(2000);
  lcd.resetDisplay();  delay(2000);
  lcd.init();
  lcd.fillScreen(RGBto565(0,0,180));
  font.printStr(0,0,"After reset"); delay(2000);
}

