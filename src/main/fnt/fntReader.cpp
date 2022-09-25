#include "fntReader.hpp"

fontReader::fontReader(SDClass sd) : _sd(sd)
{
    /*Do Nothing*/
}

ulong fontReader::get_font(uint16_t code, uint8_t *ret)
{
    unsigned long ccc = 0, ofset = 0;
    File fontx_file;
    if (code < 0x100)
        fontx_file = _sd.open(FONT_A, FILE_READ);
    if (code > 0x100)
        fontx_file = _sd.open(FONT_J, FILE_READ);
    fontx_file.seek(14);
    uint8_t Fw = fontx_file.read(), Fh = fontx_file.read(), Ft = fontx_file.read(), Fb, Fbw;
    uint16_t Fsize = ((Fw + 7) / 8) * Fh;
    if (Ft == 0x00)
    {
        if (code < 0x100)
            ofset = 17 + code * Fsize;
    }
    else
    {
        Fb = Fbw = fontx_file.read();
        while (Fbw--)
        {
            uint16_t Cbs = fontx_file.read() | fontx_file.read() << 8, Cbe = fontx_file.read() | fontx_file.read() << 8;
            if (code >= Cbs && code <= Cbe)
            {
                ccc += code - Cbs;
                ofset = 18 + 4 * Fb + ccc * Fsize;
                break;
            }
            ccc += Cbe - Cbs + 1;
        }
    }
    uint16_t i = Fsize;
    if (ofset)
    {
        fontx_file.seek(ofset);
        while (i--)
            *(ret++) = fontx_file.read();
        fontx_file.close();
        return ((unsigned long)Fw << 24) | ((unsigned long)Fh << 16) | Fsize;
    }
    fontx_file.close();
    return 0;
}
