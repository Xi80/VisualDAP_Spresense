#include <Arduino.h>
#include <MP.h>
#include <string.h>
#if (SUBCORE != 1)
#error "Core selection is wrong!!"
#endif

/*
    SubCore1のタスク

    - 時間管理(再生してからの経過時間)
    - ボタン入力管理
*/

/*
    ボタン入力書式
    メッセージID:1
    D7  D6  D5  D4  D3  D2  D1  D0
                UP  DN  RT  LT  BT
*/

/*
    1秒タイマ書式
    メッセージID:2
    (millis)
*/

/*
    1秒タイマリセット
    メッセージID:2
*/
#define _UP PIN_D03
#define _DN PIN_D04
#define _RT PIN_D05
#define _LT PIN_D06
#define _BT PIN_D02

uint32_t cur = 0, prv = 0;

uint8_t bts[5];

void setup()
{
  // put your setup code here, to run once:
  MP.begin();

  MP.RecvTimeout(MP_RECV_POLLING);

  pinMode(_UP, INPUT);
  pinMode(_DN, INPUT);
  pinMode(_RT, INPUT);
  pinMode(_LT, INPUT);
  pinMode(_BT, INPUT);

  prv = millis();

  memset(bts, 0x01, 5);
}

void loop()
{
  // put your main code here, to run repeatedly:
  cur = millis();

  if (cur - prv >= 1000)
  {
    MP.Send(2, cur);
    prv = cur;
  }
  uint32_t dat = 0;

  if (digitalRead(_UP) != bts[0])
  {

    if (bts[0] == 1)
      dat |= 0x10;

    bts[0] = digitalRead(_UP);
  }

  if (digitalRead(_DN) != bts[1])
  {
    if (bts[1] == 1)
      dat |= 0x08;

    bts[1] = digitalRead(_DN);
  }

  if (digitalRead(_RT) != bts[2])
  {
    if (bts[2] == 1)
      dat |= 0x04;
    bts[2] = digitalRead(_RT);
  }

  if (digitalRead(_LT) != bts[3])
  {
    if (bts[3] == 1)
      dat |= 0x02;
    bts[3] = digitalRead(_LT);
  }

  if (digitalRead(_BT) != bts[4])
  {
    if (bts[4] == 1)
      dat |= 0x01;
    bts[4] = digitalRead(_BT);
  }

  if (dat)
    MP.Send(1, dat);

  int8_t msgid;
  uint32_t d;

  if(MP.Recv(&msgid,&d) > 0){
    if(msgid == 2){
      prv = millis();
    }
  }

}