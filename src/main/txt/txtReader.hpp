#include <Arduino.h>
#include <SDHCI.h>
#include <File.h>
#include <string.h>

struct trackData
{
    char fileName[16];
    size_t fileNameLength;

    uint16_t title[32];
    size_t titleLength;

    uint16_t artist[32];
    size_t artistLength;
};

class textReader
{
public:
    textReader(SDClass sd);

    uint16_t getText16(char *path, uint16_t *dst);

    trackData getTrack(char* path, uint8_t index);

    uint16_t getTrackCount(char* path);
private:
    SDClass _sd;
};