#include <Arduino.h>
#include <string.h>

#include "../fnt/fntReader.hpp"

class graphicsComposer{
    public:
        graphicsComposer(fontReader reader);

        void createTextGraphics(uint16_t*,uint8_t,uint8_t*);
    private:
        fontReader _reader;
        uint8_t tmpBuffer[16*64];  /*出力バッファ*/
};