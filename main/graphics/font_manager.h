/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 16:2:29
 */
#ifndef _5BE0863E_1133_F008_F47F_027896B0DCA7
#define _5BE0863E_1133_F008_F47F_027896B0DCA7

#include <stdint.h>

namespace graphics
{
struct FontData;
class FrameBufferBase;

class FontManager
{
public:
    void setAsciiFontData(const FontData* f) { asciiFont_ = f; }
    void setKanjiFontData(const FontData* f) { kanjiFont_ = f; }

    void putAscii(int code);
    void putKanji(int code);
    void put(int code);
    void putString(const char* str);

    void computeTextSize(const char* str, int& w, int& h);

    void setWindow(int x, int y, int w, int h)
    {
        vx_ = x;
        vy_ = y;
        vw_ = w;
        vh_ = h;
    }

    void setPosition(int x, int y)
    {
        x_ = vx_ + x;
        y_ = vy_ + y;
    }
    void setColor(uint32_t c) { color_ = c; }
    void setBGColor(uint32_t c) { bgColor_ = c; }
    void setTransparentMode(bool f = true) { transparent_ = f; }
    void setEdgedMode(bool f = true) { edged_ = f; }
    void setMultiLineMode(bool f = true) { multiLine_ = f; }
    void setAKConvertMode(bool f = true) { akcnv_ = f; }

    void setFrameBuffer(FrameBufferBase* fb);

protected:
    void putFont(int code, const FontData* f);

private:
    const FontData* asciiFont_{};
    const FontData* kanjiFont_{};

    FrameBufferBase* frameBuffer_{};

    uint32_t color_   = 0xffffffff;
    uint32_t bgColor_ = 0;
    bool transparent_ = true;
    bool edged_       = false;
    bool multiLine_   = true;
    bool akcnv_       = false;

    int vx_ = 0;
    int vy_ = 0;
    int vw_ = 320;
    int vh_ = 240;

    int x_ = 0;
    int y_ = 0;
};

FontManager& getDefaultFontManager();

} // namespace graphics

#endif /* _5BE0863E_1133_F008_F47F_027896B0DCA7 */
