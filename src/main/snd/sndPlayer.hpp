#include <Arduino.h>
#include <Audio.h>
#include <File.h>
#include <SDHCI.h>


class SoundPlayer{
    public:
        SoundPlayer(SDClass sd);

        void init(void);

        void play(char* filePath);

        void pause(void);

        void stop(void);

        void setVolume(int vol);

        void process(void);

        int isPlaying;

        bool isEnd;

        uint32_t playTime;
    private:
        uint32_t totalSize;
        SDClass _sd;
};