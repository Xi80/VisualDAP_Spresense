#include <Arduino.h>
#include <SPI.h>
#include <string.h>

class AQM1248A{
    public:
        uint8_t _gfxBuffer[6][128];

        AQM1248A(SPIClass SPI,uint8_t res,uint8_t rs);

        void clear(void);
        void init(void);

        void xferAll(void);
        void xferPage(uint8_t page);

        void setPixel(uint8_t x,uint8_t y,uint8_t state);

        void showState(uint8_t);

        void showVolume(uint8_t);

        void showSplash(void);
    private:

        void writeCommand(uint8_t cmd);
        void writeData(uint8_t dat);

        uint8_t _res,_rs,_cs;
        SPIClass _spi;
        SPISettings _settings;
};
