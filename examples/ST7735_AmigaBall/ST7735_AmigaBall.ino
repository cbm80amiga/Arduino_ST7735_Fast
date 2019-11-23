// ST7735 library example
// Amiga Boing Ball Demo
// (c) 2019 Pawel A. Hernik
// YT video: https://youtu.be/KwtkfmglT-c

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

#include "ball.h"

uint16_t palette[16];
uint16_t line[SCR_WD];
uint16_t bgCol   = RGBto565(160,160,160);
uint16_t lineCol = RGBto565(150,40,150);

#define LINE_YS  10
#define LINE_XS1 19
#define LINE_XS2 2

#define BALL_WD 64
#define BALL_HT 64
#define BALL_SWD 128
#define BALL_SHT 131

void drawBall(int x, int y)
{
  int i,j;
  for(j=0;j<BALL_HT;j++) {
    uint8_t v,*img = (uint8_t*)ball+16*2+6+j*BALL_WD/2;
    int yy=y+j;
    if(yy==LINE_YS      || yy==LINE_YS+1*10 || yy==LINE_YS+2*10 || yy==LINE_YS+3*10 || yy==LINE_YS+4*10 || yy==LINE_YS+5*10 || yy==LINE_YS+6*10 ||
       yy==LINE_YS+7*10 || yy==LINE_YS+8*10 || yy==LINE_YS+9*10 || yy==LINE_YS+10*10 || yy==LINE_YS+11*10 || yy==LINE_YS+12*10) {
      for(i=0;i<LINE_XS1;i++) line[i]=line[SCR_WD-1-i]=bgCol;
      for(i=0;i<=SCR_WD-LINE_XS1*2;i++) line[i+LINE_XS1]=lineCol;
    } else {
      for(i=0;i<SCR_WD;i++) line[i]=bgCol;
      if(yy>LINE_YS) for(i=0;i<10;i++) line[LINE_XS1+i*10]=lineCol;
    }
  
    for(i=0;i<BALL_WD;i+=2) {
      v = pgm_read_byte(img++);
      if(v>>4)  line[x+i+0] = palette[v>>4];
      if(v&0xf) line[x+i+1] = palette[v&0xf];
    }
    lcd.drawImage(0,yy,SCR_WD,1,line);
  }
}

void setup() 
{
  Serial.begin(115200);
  lcd.init();
  lcd.fillScreen(bgCol);
  int i,o;
  uint16_t *pal = (uint16_t*)ball+3;
  for(i=0;i<16;i++) palette[i] = pgm_read_word(&pal[i]);
  for(i=0;i<10;i++) lcd.drawFastVLine(LINE_XS1+i*10, LINE_YS, 12*10, lineCol);
  for(i=0;i<13;i++) lcd.drawFastHLine(LINE_XS1, LINE_YS+i*10, SCR_WD-LINE_XS1*2, lineCol);
  lcd.drawFastHLine(LINE_XS2,SCR_HT-LINE_YS, SCR_WD-LINE_XS2*2, lineCol);
  int dy=SCR_HT-LINE_YS-(LINE_YS+10*12);
  int dx=LINE_XS1-LINE_XS2;
  o=7*dx/dy;
  lcd.drawFastHLine(LINE_XS2+o,SCR_HT-LINE_YS-7, SCR_WD-LINE_XS2*2-o*2, lineCol);
  o=(7+6)*dx/dy;
  lcd.drawFastHLine(LINE_XS2+o,SCR_HT-LINE_YS-7-6, SCR_WD-LINE_XS2*2-o*2, lineCol);
  o=(7+6+4)*dx/dy;
  lcd.drawFastHLine(LINE_XS2+o,SCR_HT-LINE_YS-7-6-4, SCR_WD-LINE_XS2*2-o*2, lineCol);
  for(i=0;i<10;i++) lcd.drawLine(LINE_XS1+i*10, LINE_YS+10*12, LINE_XS2+i*(SCR_WD-LINE_XS2*2)/9, SCR_HT-LINE_YS, lineCol);

  //delay(10000);
}

int anim=0, animd=1;
int x=0, y=0;
int xd=2, yd=1;

void loop()
{
  for(int i=0;i<14;i++) {
    palette[i+1] = ((i+anim)%14)<7 ? WHITE : RED;
    //int c=((i+anim)%14);
    //if(c<6) palette[i+1]=WHITE; else
    //if(c==6 || c==13) palette[i+1]=RGBto565(255,128,128); else palette[i+1]=RED;
  }
  drawBall(x,y);
  anim+=animd;
  if(anim<0) anim+=14;
  x+=xd;
  y+=yd;
  if(x<0) { x=0; xd=-xd; animd=-animd; }
  if(x>=BALL_SWD-BALL_WD) { x=BALL_SWD-BALL_WD; xd=-xd; animd=-animd; }
  if(y<0) { y=0; yd=-yd; }
  if(y>=BALL_SHT-BALL_HT) { y=BALL_SHT-BALL_HT; yd=-yd; }
  //delay(20);
}

