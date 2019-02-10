/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 16:25:22
 */
#ifndef D8D14E5A_A133_F008_F7FC_B44CBB78814B
#define D8D14E5A_A133_F008_F7FC_B44CBB78814B

#include "framebuffer_base.h"
#include <M5Display.h>
#undef min

namespace graphics
{

class Display final : public FrameBufferBase
{
public:
    bool initialize();

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

    void blit(const FrameBufferBase& fb, int x, int y) override;

public:
    TFT_eSPI* _getLCD() { return &display_; }

private:
    uint32_t wx_ = 0;
    uint32_t wy_ = 0;
    uint32_t ww_ = 0;
    uint32_t wh_ = 0;

    M5Display display_;
};

Display& getDisplay();

} // namespace graphics
#endif /* D8D14E5A_A133_F008_F7FC_B44CBB78814B */
