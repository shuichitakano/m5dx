/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 27 2018 12:47:36
 */
#ifndef _5DD24AA2_6133_F00E_BE5E_10D81AEF7E1E
#define _5DD24AA2_6133_F00E_BE5E_10D81AEF7E1E

#include "framebuffer_base.h"
#include <M5Display.h> // utility/spriteには include guardがない
#undef min

namespace graphics
{

class FrameBuffer final : public FrameBufferBase
{
public:
    FrameBuffer();
    FrameBuffer(uint32_t w, uint32_t h, int bpp);
    ~FrameBuffer() noexcept override { release(); }

    void initialize(uint32_t w, uint32_t h, int bpp);
    void release();

    int getBitPerPixel() const { return bpp_; }

    void setWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
    uint32_t getLeft() const override;
    uint32_t getTop() const override;
    uint32_t getWidth() const override;
    uint32_t getHeight() const override;
    uint32_t getBufferWidth() const override;
    uint32_t getBufferHeight() const override;

    uint32_t makeColor(int r, int g, int b) const override;

    void fill(uint32_t c) override;
    void fill(int x, int y, int w, int h, uint32_t c) override;

    void setPixel(uint32_t x, uint32_t y, uint32_t c) override;
    uint32_t getPixel(uint32_t x, uint32_t y) const override;

    void transferTo(FrameBufferBase& dst,
                    int dx,
                    int dy,
                    int sx,
                    int sy,
                    int w,
                    int h) const override;

private:
    using Img = TFT_eSprite;
    Img img_;

    uint32_t wx_ = 0;
    uint32_t wy_ = 0;
    uint32_t ww_ = 0;
    uint32_t wh_ = 0;

    void* buffer_                = nullptr;
    uint32_t bw_                 = 0;
    uint32_t bh_                 = 0;
    int bpp_                     = 0;
    uint32_t unitTransferPixels_ = 1280; // 40Mで2500pixelが1ms
};

} // namespace graphics

#endif /* _5DD24AA2_6133_F00E_BE5E_10D81AEF7E1E */
