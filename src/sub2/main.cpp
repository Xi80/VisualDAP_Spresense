#include <Arduino.h>
#include <MP.h>
#include <string.h>
#include "gfx/gfxController.hpp"
/*
    SubCore2のタスク

    - 液晶制御

*/
#define _RES PIN_D08
#define _RS PIN_D09

AQM1248A lcd(SPI, _RES, _RS);

uint8_t recvBuf[16 * 64];

/*
    タイトル受信ID:3
*/

struct dspData
{
  uint8_t type;
  uint8_t data[64 * 16];
};

struct dspData *dsp;

void setup()
{
  // put your setup code here, to run once:
  MP.begin();
  MP.RecvTimeout(MP_RECV_POLLING);

  lcd.init();
  lcd.clear();
  lcd.showSplash();

  delay(2000);

  lcd.xferAll();
  pinMode(PIN_D14, OUTPUT);
  pinMode(PIN_D15, OUTPUT);
  digitalWrite(PIN_D14, LOW);
  digitalWrite(PIN_D15, LOW);
}

void drawTime(void)
{
  for (uint8_t y = 32; y < 48; y++)
  {
    for (uint8_t x = 20; x < 60; x++)
    {
      lcd.setPixel(x, y, 0);
    }
  }
  for (uint8_t y = 0; y < 16; y++)
  {
    for (uint8_t x = 0; x < 40; x++)
    {
      uint16_t arrPos = y * 64 + x / 8;
      uint8_t bitPos = x % 8;
      if (dsp->data[arrPos] & (1 << (7 - bitPos)))
      {
        lcd.setPixel(20 + x, 32 + y, 1);
      }
      else
      {
        lcd.setPixel(20 + x, 32 + y, 0);
      }
    }
  }
  lcd.xferAll();
}

void drawTitle(void)
{
  for (uint8_t y = 0; y < 16; y++)
  {
    for (uint8_t x = 0; x < 128; x++)
    {
      lcd.setPixel(x, y, 0);
    }
  }
  for (uint8_t y = 0; y < 16; y++)
  {
    for (uint8_t x = 0; x < 128; x++)
    {
      uint16_t arrPos = y * 64 + x / 8;
      uint8_t bitPos = x % 8;
      if (dsp->data[arrPos] & (1 << (7 - bitPos)))
      {
        lcd.setPixel(x, y, 1);
      }
      else
      {
        lcd.setPixel(x, y, 0);
      }
    }
  }
  lcd.xferAll();
}

void drawState(void)
{
  if (dsp->type == 1)
  {
    lcd.showState(1);
  }
  else
  {
    lcd.showState(2);
  }
}

void drawArtist(void)
{
  for (uint8_t y = 0; y < 16; y++)
  {
    for (uint8_t x = 0; x < 128; x++)
    {
      lcd.setPixel(x, 16 + y, 0);
    }
  }
  for (uint8_t y = 0; y < 16; y++)
  {
    for (uint8_t x = 0; x < 128; x++)
    {
      uint16_t arrPos = y * 64 + x / 8;
      uint8_t bitPos = x % 8;
      if (dsp->data[arrPos] & (1 << (7 - bitPos)))
      {
        lcd.setPixel(x, 16 + y, 1);
      }
      else
      {
        lcd.setPixel(x, 16 + y, 0);
      }
    }
  }
}
void loop()
{
  // put your main code here, to run repeatedly:
  int8_t msgid;

  if (MP.Recv(&msgid, &dsp) > 0)
  {
    if (msgid == 3)
    {
      //時間
      drawTime();

      MP.Send(100,0x80);
    }
    else if (msgid == 2)
    {
      //タイトル
      drawTitle();

      MP.Send(100,0x80);
    }
    else if (msgid == 1)
    {
      //状態
      drawState();
    }
    else if (msgid == 6)
    {
      //アーティスト
      drawArtist();

      MP.Send(100,0x80);
    }
    else if (msgid == 5)
    {
      //ボリューム
      lcd.showVolume(dsp->type);
    }
    else if (msgid == 7)
    {
      //初期化完了
      lcd.clear();
      lcd.xferAll();
      lcd.showVolume(6);
    }
  }
}