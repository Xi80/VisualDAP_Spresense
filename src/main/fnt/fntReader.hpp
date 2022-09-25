#include <Arduino.h>
#include <SDHCI.h>
#include <File.h>

#define FONT_J  "FONTXJ.FNT"    //全角フォント
#define FONT_A  "FONTXA.FNT"    //半角フォント

class fontReader{
    public:
        fontReader(SDClass sd);

        ulong get_font(uint16_t,uint8_t*);
    private:

        SDClass _sd;
};