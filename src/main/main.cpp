#include <Arduino.h>
#include <string.h>
#include <MP.h>
#include <dirent.h>
#include <Audio.h>
#include <SDHCI.h>

#include "gfx_c/gfxComposer.hpp"
#include "txt/txtReader.hpp"
#include "snd/sndPlayer.hpp"

#define SERIAL_BAUD 115200

#define MUSIC_PATH "/mnt/sd0/wav/"

/*
    MainCoreのタスク

    - SubCoreの起動
    - SD関係(音楽、フォント・曲名等読み出し)
    - 各種文字列画像生成
    - 音楽再生・制御

*/

SDClass sd;
fontReader reader(sd);
graphicsComposer composer(reader);
SoundPlayer player(sd);
textReader txt(sd);

char path[32] = "";

uint8_t countTrack = 0;
uint8_t currentTrack = 0;

struct dspData
{
  uint8_t type;
  uint8_t data[64 * 16];
};

int vol = 6;
const int vols[] = {-960, -480, -240, -120, -60, -30, -15, 0};

bool flag = false;
struct dspData data;

uint8_t b[64 * 16];
uint16_t t[1024];
int8_t msgId;
uint32_t dat;

void upVol(void)
{
  vol = (vol < 7) ? vol + 1 : vol;
  player.setVolume(vols[vol]);
  data.type = vol;
  MP.Send(5, &data, 2);
}

void downVol(void)
{
  vol = (vol > 0) ? vol - 1 : vol;
  player.setVolume(vols[vol]);
  data.type = vol;
  MP.Send(5, &data, 2);
}

void xferTitle(uint16_t *title, uint16_t length)
{
  composer.createTextGraphics(title, length, &b[0]);
  memcpy(data.data, b, 64 * 16);
  MP.Send(2, &data, 2);

  while (MP.Recv(&msgId, &dat, 2) < 0)
    ;
}

void xferArtist(uint16_t *artist, uint16_t length)
{
  composer.createTextGraphics(artist, length, &b[0]);
  memcpy(data.data, b, 64 * 16);
  MP.Send(6, &data, 2);

  while (MP.Recv(&msgId, &dat, 2) < 0)
    ;
}

void xferState(uint8_t state)
{
  MP.Send(2, (uint32_t)0, 1);
  data.type = state;
  MP.Send(1, &data, 2);
  return;
}

void createPath(char *fileName, char *p)
{
  memset(p, 0x00, 32);

  strcpy(p, MUSIC_PATH);

  strcat(p, fileName);

  return;
}

void xferPlayTime(uint16_t in)
{
  char buf[6];         //時間表示用バッファ
  uint16_t ch_time[6]; //表示文字列
  int m = 0;           //分
  int s = 0;           //秒

  while (in >= 3600)
  { //時を決定
    return;
  }

  while (in >= 60)
  { //分を決定
    in -= 60;
    m++;
  }

  s = in; //余りが秒

  sprintf(buf, "%02d:%02d", m, s);

  for (uint8_t i = 0; i < 6; i++)
  {
    ch_time[i] = (uint16_t)buf[i];
  }

  composer.createTextGraphics(ch_time, 5, &b[0]);

  memcpy(data.data, b, 64 * 16);

  MP.Send(3, &data, 2);

  while (MP.Recv(&msgId, &dat, 2) < 0)
    ;
}

void setup()
{
  /*ポーリングモード*/
  MP.RecvTimeout(MP_RECV_POLLING);

  /*SubCoreを起動*/

  MP.begin(2);

  while (!sd.begin())
  {
    /* wait until SD card is mounted. */
    delay(100);
  }

  MP.begin(1);

  /*シリアル通信を開始(デバッグ用)*/
  Serial.begin(SERIAL_BAUD);

  printf("%d\n", txt.getTrackCount("hoge.txt"));

  /*トラックデータを取得*/
  trackData trk = txt.getTrack("hoge.txt", currentTrack + 1);

  countTrack = txt.getTrackCount("hoge.txt");

  /*プレイヤー初期化*/
  player.init();
  player.setVolume(vols[6]);

  MP.Send(7, &data, 2); /*初期化OK*/

  /*曲名，歌手情報を転送*/
  xferTitle(trk.title, trk.titleLength);

  xferArtist(trk.artist, trk.artistLength);

  /*再生*/
  xferState(1);
  createPath(trk.fileName, path);
  printf("%s\n", path);
  player.play(path);
}

void loop()
{
  // put your main code here, to run repeatedly:
  player.process();

  if (player.isEnd)
  {
    /*次の曲へ*/

    player.isEnd = false;
    if (player.isPlaying == 1)
      player.stop();
    if (player.isPlaying == 2)
    {
      player.setVolume(-960);
      player.pause();
      delay(500);
      player.pause();
      player.setVolume(vols[vol]);
    }

    xferPlayTime(0);

    /*パス変更*/

    if (currentTrack = countTrack - 1)
    {
      currentTrack = 0;
    }
    else
    {
      currentTrack++;
    }

    trackData trk = txt.getTrack("hoge.txt", currentTrack + 1);
    xferTitle(trk.title, trk.titleLength);
    xferArtist(trk.artist, trk.artistLength);
    createPath(trk.fileName, path);
    player.play(path);
  }

  if (MP.Recv(&msgId, &dat, 1) > 0)
  {
    if (msgId == 2)
    {
      /*再生時間転送*/
      if (player.isPlaying == 1)
      {

        xferPlayTime(player.playTime);
      }
      else if (player.isPlaying == 2)
      {
        /*点滅処理*/

        if (flag)
        {
          memset(data.data, 0x00, 64 * 16);
          MP.Send(3, &data, 2);
          while (MP.Recv(&msgId, &dat, 2) < 0)
            ;
        }
        else
        {
          memcpy(data.data, b, 64 * 16);
          MP.Send(3, &data, 2);
          while (MP.Recv(&msgId, &dat, 2) < 0)
            ;
        }

        flag = !flag;
      }
    }
    else if (msgId == 1)
    {
      /*ユーザー入力*/
      if (dat & 0x01)
      {
        /*ボタン押下*/
        player.pause();

        data.type = player.isPlaying;
        MP.Send(1, &data, 2);
      }
      else if (dat & 0x10)
      {
        /*上*/
        upVol();
      }
      else if (dat & 0x08)
      {
        /*下*/
        downVol();
      }
      else if (dat & 0x04)
      {
        /*右*/
        /*次の曲へ*/
        if (player.isPlaying == 1)
          player.stop();
        if (player.isPlaying == 2)
        {
          player.setVolume(-960);
          player.pause();
          delay(500);
          player.pause();
          player.setVolume(vols[vol]);
        }

        xferPlayTime(0);

        /*パス変更*/

        if (currentTrack == countTrack - 1)
        {
          currentTrack = 0;
        }
        else
        {
          currentTrack++;
        }

        trackData trk = txt.getTrack("hoge.txt", currentTrack + 1);
        xferTitle(trk.title, trk.titleLength);

        xferArtist(trk.artist, trk.artistLength);
        createPath(trk.fileName, path);
        player.play(path);
      }
      else if (dat & 0x02)
      {

        if (player.isPlaying == 1)
          player.stop();
        if (player.isPlaying == 2)
        {
          player.setVolume(-960);
          player.pause();
          delay(500);
          player.pause();
          player.setVolume(vols[vol]);
        }

        xferPlayTime(0);
        if (player.playTime < 3)
        {
          /*パス変更*/

          if (currentTrack == 0)
          {
            currentTrack = countTrack - 1;
          }
          else
          {
            currentTrack--;
          }
          trackData trk = txt.getTrack("hoge.txt", currentTrack + 1);
          xferTitle(trk.title, trk.titleLength);
          xferArtist(trk.artist, trk.artistLength);
          createPath(trk.fileName, path);
        }
        player.play(path);
      }
    }
  }
}