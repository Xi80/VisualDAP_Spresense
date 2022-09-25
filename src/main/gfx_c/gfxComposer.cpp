#include "gfxComposer.hpp"

graphicsComposer::graphicsComposer(fontReader reader) : _reader(reader){

}

void graphicsComposer::createTextGraphics(uint16_t *txt,uint8_t length,uint8_t *out){
    memset(tmpBuffer,0x00,64*16);

    uint8_t idx = 0;
    for(uint8_t k = 0;k < length;k++){
        uint8_t character[32];
        _reader.get_font(txt[k],character);
        if(txt[k] > 0x100){
            /*全角*/
            for(uint8_t y = 0;y < 16;y++){
                memcpy(&tmpBuffer[64 * y + idx],&character[y << 1],2);
            }
            idx += 2;
        } else {
            /*半角*/
            for(uint8_t y = 0;y < 16;y++){
                memcpy(&tmpBuffer[64 * y + idx],&character[y],1);
            }
            idx += 1;
        }
    }
    memset(out,0x00,64*16);
    memcpy(out,tmpBuffer,64*16);
}