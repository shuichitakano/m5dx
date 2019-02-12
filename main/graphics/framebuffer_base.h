/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 16:29:56
 */
#ifndef _294F65DF_7133_F008_F8A7_4D243931B5D2
#define _294F65DF_7133_F008_F8A7_4D243931B5D2

#include <stdint.h>

namespace graphics
{

class FrameBuffer;

class FrameBufferBase
{
public:
    virtual ~FrameBufferBase() noexcept = default;

    virtual void setWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) = 0;
    virtual uint32_t getLeft() const                                       = 0;
    virtual uint32_t getTop() const                                        = 0;
    virtual uint32_t getWidth() const                                      = 0;
    virtual uint32_t getHeight() const                                     = 0;
    virtual uint32_t getBufferWidth() const                                = 0;
    virtual uint32_t getBufferHeight() const                               = 0;

    virtual uint32_t makeColor(int r, int g, int b) const = 0;

    virtual void setPixel(uint32_t x, uint32_t y, uint32_t c) = 0;
    virtual uint32_t getPixel(uint32_t x, uint32_t y) const   = 0;

    virtual void fill(int x, int y, int w, int h, uint32_t c);
    virtual void fill(uint32_t c);
    virtual void blit(const FrameBufferBase& fb, int x, int y);

    void
    drawBits(int x, int y, int w, int h, const uint8_t* bits, uint32_t color);
    void drawBits(int x,
                  int y,
                  int w,
                  int h,
                  const uint8_t* bits,
                  uint32_t color,
                  uint32_t bgColor);

    void drawBitsThick(int x,
                       int y,
                       int w,
                       int h,
                       const uint8_t* bits,
                       uint32_t color,
                       uint32_t edgeColor);

protected:
    virtual void _drawBitsLineTrans(
        const uint8_t* bits, int bitOfs, uint32_t color, int x, int y, int w);

    virtual void _drawBitsLine(const uint8_t* bits,
                               int bitOfs,
                               uint32_t color,
                               uint32_t bgColor,
                               int x,
                               int y,
                               int w);

    struct InternalLCD;

public:
    virtual bool _blitToLCD(
        InternalLCD*, int dx, int dy, int wx, int wy, int ww, int wh) const
    {
        return false;
    }
};

inline uint32_t
makeColor(const FrameBufferBase& fb, uint32_t c)
{
    auto r = fb.makeColor(c >> 16, (c >> 8) & 255, c & 255);
    return r;
}

} // namespace graphics

#endif /* _294F65DF_7133_F008_F8A7_4D243931B5D2 */
