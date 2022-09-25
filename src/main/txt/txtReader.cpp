#include "txtReader.hpp"

uint16_t tmp[1024];

textReader::textReader(SDClass sd) : _sd(sd)
{
}

uint16_t textReader::getText16(char *path, uint16_t *dst)
{
    uint16_t idx = 0;
    File text = _sd.open(path);

    if (!text)
        return;

    while (text.available())
    {
        uint8_t dat = text.read();
        if (((dat & 0xF0) == 0x80) || ((dat & 0xF0) == 0x90) || ((dat & 0xF0) == 0xE0))
        {
            tmp[idx] = dat << 8;
            dat = text.read();
            tmp[idx++] |= dat;
        }
        else
        {
            tmp[idx++] = dat;
        }
    }
    text.close();
    memcpy(dst, tmp, idx << 1);
    return idx;
}

trackData textReader::getTrack(char *path, uint8_t index)
{
    uint16_t input[1024];
    uint16_t size = getText16(path, input);
    trackData ret;
    uint16_t countIndex = 0;
    uint16_t topIndex = 0; /*行開始位置*/
    uint16_t endIndex = 0; /*行終了位置*/

    /*指定インデックスの範囲検索*/
    for (uint16_t i = 0; i < size; i++)
    {
        if ((input[i] == (uint16_t)0x1A))
        {
            // EOF
            endIndex = i;

            countIndex++;

            if (countIndex < index)
                return;

            if (countIndex == index)
                break;
        }

        if ((input[i] == (uint16_t)0x0D) && (input[i + 1] == (uint16_t)0x0A))
        {

            //改行
            endIndex = i;

            countIndex++;

            if (countIndex == index)
                break;

            topIndex = endIndex + 2;
        }
    }

    /*分離処理*/

    /*
        ファイルのフォーマット
        - 改行区切り(CR+LF)
        - トラック内の区切りはTAB文字
        - 「ファイル名」　「タイトル」　「歌手」の順で羅列する。
    */
    uint8_t state = 0;
    uint16_t writeIdx = 0;

    ret.artistLength = 0;
    ret.fileNameLength = 0;
    ret.titleLength = 0;

    for (uint16_t i = topIndex; i <= endIndex; i++)
    {
        if ((input[i] == 0x09) || i == endIndex)
        {
            if (state == 2)
            {
                ret.fileName[ret.fileNameLength] = 0x00;
                return ret;
            }
            writeIdx = 0;
            state++;
            continue;
        }
        if (state == 0)
        {
            /*ファイル名*/
            ret.fileName[writeIdx++] = (uint8_t)input[i];
            ret.fileNameLength = writeIdx;
        }
        else if (state == 1)
        {
            /*タイトル*/
            ret.title[writeIdx++] = input[i];
            ret.titleLength = writeIdx;
        }
        else if (state == 2)
        {
            /*歌手*/
            ret.artist[writeIdx++] = input[i];
            ret.artistLength = writeIdx;
        }
    }
}

uint16_t textReader::getTrackCount(char *path)
{
    uint16_t input[1024];
    uint16_t size = getText16(path, input);
    uint16_t countIndex = 0;

    /*指定インデックスの範囲検索*/
    for (uint16_t i = 0; i < size; i++)
    {
        if ((input[i] == (uint16_t)0x1A))
        {
            // EOF
            countIndex++;
        }

        if ((input[i] == (uint16_t)0x0D) && (input[i + 1] == (uint16_t)0x0A))
        {

            //改行
            countIndex++;
        }
    }

    return countIndex;
}