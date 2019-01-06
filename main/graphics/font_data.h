/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 27 2018 5:32:55
 */
#ifndef _09B1A4C6_5133_F00E_5141_FB93E311CE58
#define _09B1A4C6_5133_F00E_5141_FB93E311CE58

#include <stdint.h>

namespace graphics
{

class FontData
{
    struct Entry
    {
        int16_t idx;   // 下位バイト0x00 のフォントインデックス
        uint8_t begin; // 文字の存在する範囲先頭
        uint8_t end;   // 文字の存在する範囲末尾
    };

    struct Header
    {
        char magic[3]; // FNT
        uint8_t entryCount;
        uint8_t width;
        uint8_t height;
        uint16_t codeOfs; // 全角は0x2121, 半角は0

        Entry entry[1]; // 上位バイトで引く

    public:
        uint8_t* getGlyph() const { return (uint8_t*)&entry[entryCount]; }
    };

    const Header* data_;
    const uint8_t* glyph_;
    int unitSize_;

public:
    FontData(const void* data = 0) { setData(data); }
    void setData(const void* data);

    inline int width() const { return data_->width; }
    inline int height() const { return data_->height; }

    const uint8_t* getGlyph(int code) const;
};

} // namespace graphics

#endif /* _09B1A4C6_5133_F00E_5141_FB93E311CE58 */
