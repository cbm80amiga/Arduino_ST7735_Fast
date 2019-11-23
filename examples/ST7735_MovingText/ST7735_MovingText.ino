// ST7735 library example
// (C)2019 Pawel A. Hernik

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
#include "rre_chicago_20x24.h"

RREFont font;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }

void setup() 
{
  Serial.begin(9600);
  lcd.init();
  lcd.fillScreen(BLACK);

  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
  font.setFont(&rre_chicago_20x24);
  int i;
  for(i=0;i<10;i++) {
    font.setColor(RGBto565(i*25,i*25,i*25));
    font.printStr(30+i,20+i,"Hello");
  }
  font.setColor(WHITE);
  font.printStr(30+i,20+i,"Hello");

  for(i=0;i<10;i++) {
    font.setColor(lcd.rgbWheel(0+i*8));
    font.printStr(25+i,60+i,"World");
  }
  font.setColor(WHITE);
  font.printStr(25+i,60+i,"World");
  delay(4000);
  lcd.fillScreen();
}

#define MAX_TXT 32
byte tx[MAX_TXT];
byte ty[MAX_TXT];
byte cur=0;
int numTxt=16;
int x=0,y=0;
int dx=6,dy=5;
int i,ii,cc=0;

void loop()
{
  x+=dx;
  y+=dy;
  if(x>SCR_WD-20) { dx=-dx; x=SCR_WD-20; }
  if(x<1)   { dx=-dx; x=0; }
  if(y>SCR_HT-20) { dy=-dy; y=SCR_HT-20; }
  if(y<1)   { dy=-dy; y=0; }
  int i=cur;
  //font.setColor(BLACK);
  //font.printStr(tx[i],ty[i],"Hello!");
  tx[cur]=x;
  ty[cur]=y;
  if(++cur>=numTxt) cur=0;
  for(i=0;i<numTxt;i++) {
    ii=i+cur;
    if(ii>=numTxt) ii-=numTxt;
    font.setColor(RGBto565(i*15,i*5,0));
    if(i==numTxt-1) font.setColor(WHITE);
    font.printStr(tx[ii],ty[ii],"Hello!");
    cc++;
  }
  delay(20);
}

