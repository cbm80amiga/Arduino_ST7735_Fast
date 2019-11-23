// ST7735 library example
// (c) 2019 Pawel A. Hernik

/*
 ST7735 128x160 LCD pinout (header at the top, from left; better viewing angles):
 #1 LED   -> 3.3V
 #2 SCK   -> D13
 #3 SDA   -> D11/MOSI
 #4 A0/DC -> D8  or any digital
 #5 RESET -> D9  or any digital
 #6 CS    -> D10 or any digital
 #7 GND   -> GND
 #8 VCC   -> 3.3V
*/

#define SCR_WD 128
#define SCR_HT 160

#define TFT_CS 10
#define TFT_DC  8
#define TFT_RST 9
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ST7735_Fast.h>
Arduino_ST7735 lcd = Arduino_ST7735(TFT_DC, TFT_RST, TFT_CS);

void setup(void) 
{
  Serial.begin(9600);
  lcd.init();
  lcd.fillScreen(BLACK);
  lcd.setTextColor(WHITE,BLUE);
  int xt=(SCR_WD-11*6)/2, yt=(SCR_HT-8)/2;
  lcd.setCursor(xt, yt);
  lcd.println("HELLO WORLD");
  lcd.drawRect(xt-10,yt-10,11*6+20,8+20,GREEN);
 }

void loop()
{
}

