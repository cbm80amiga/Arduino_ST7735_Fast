// ST7735 library example
// (c) 2019 Pawel A. Hernik

// requires RRE Font library:
// https://github.com/cbm80amiga/RREFont

/*
 ST7735 128x160 LCD pinout (header at the top for better viewwing angles, from left):
 #1 LED   -> 3.3V
 #2 SCK   -> SCL/D13/PA5
 #3 SDA   -> MOSI/D11/PA7
 #4 A0/DC -> D8/PA1  or any digital
 #5 RESET -> D9/PA0  or any digital
 #6 CS    -> D10/PA2 or any digital
 #7 GND   -> GND
 #8 VCC   -> 3.3V
*/

#define SCR_WD 128
#define SCR_HT 160

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
#include "rre_arialb_16.h"

RREFont font;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }

void setup() 
{
  Serial.begin(9600);
  lcd.init();
  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values

  for(int i=0;i<SCR_HT;i+=4) {
    uint8_t r,g,b;
    lcd.rgbWheel(i*512L/SCR_HT,&r,&g,&b);
    lcd.fillRect(0,i,SCR_WD,4,RGBto565(r,g,b));
  }

  font.setFont(&rre_arialb_16); font.setSpacing(2);
  font.setScale(2);
  font.setColor(BLACK);
  font.printStr(ALIGN_CENTER,30+2,"SCROLL"); font.printStr(ALIGN_CENTER,70+2,"DEMO");
  font.setColor(WHITE);
  font.printStr(ALIGN_CENTER,30,"SCROLL"); font.printStr(ALIGN_CENTER,70,"DEMO");
  font.setScale(1);
  delay(2000);
}

int bgCols[] = {2,4,6,8,10,8,6,4};
char *scrollTxt[] = {"","This","is","an example","of","super-smooth","scrolling","with regular",
                     "AVR Arduino,","ILI9163C","1280x128 LCD","library","and","RRE Fonts",""};

int c=0,t=0;
int maxy=160;
unsigned long ms;

void loop()
{
  // full screen scrolling
  lcd.setScrollArea(0,0);
  font.setSpacing(2);
  for(int l=0;l<3;l++)
  for(int i=0;i<maxy;i++) {
    ms = millis();
    lcd.setScroll(i);
    int y=i+SCR_HT-16;
    if(y>=maxy) y-=maxy;
    if((i%16)==0) {
      lcd.fillRect(0,y,128,16,RGBto565(0,0,bgCols[c]<<4));
      font.printStr(ALIGN_CENTER,y+1,scrollTxt[t]);
      if(++c>=sizeof(bgCols)/sizeof(bgCols[0])) c=0;
      if(++t>=sizeof(scrollTxt)/sizeof(scrollTxt[0])) t=0;
      //Serial.println(millis()-ms); // less than 25ms per line
    }
    while(millis()-ms<25);
  }

  // scrolling with fixed top area
  lcd.fillRect(0,0,128,3,RGBto565(220,0,220));
  lcd.fillRect(0,3,128,32-6,RGBto565(180,0,180));
  lcd.fillRect(0,32-3,128,3,RGBto565(140,0,140));
  font.setScale(1); font.setSpacing(1);
  font.setColor(YELLOW);
  font.printStr(ALIGN_CENTER,8,"Fixed Top Area");
  font.setColor(WHITE);
  font.setScale(1); font.setSpacing(3);
  lcd.setScrollArea(32, 0);
  for(int l=0;l<3;l++)
  for(int i=32;i<maxy;i++) {
    ms = millis();
    lcd.setScroll(i);
    int y=i+SCR_HT-16-32;
    if(y>=maxy) {y-=maxy; y+=32;}
    if((i%16)==0) {
      lcd.fillRect(0,y,128,16,RGBto565(0,0,bgCols[c]<<4));
      font.printStr(ALIGN_CENTER,y+1,scrollTxt[t]);
      if(++c>=sizeof(bgCols)/sizeof(bgCols[0])) c=0;
      if(++t>=sizeof(scrollTxt)/sizeof(scrollTxt[0])) t=0;
    }
    while(millis()-ms<25);
  }
}

