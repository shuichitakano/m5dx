/*
 * author : Shuichi TAKANO
 * since  : Thu Sep 10 2020 03:56:57
 */
#ifndef _00E1F8C7_7134_3DC8_3663_430FE012A628
#define _00E1F8C7_7134_3DC8_3663_430FE012A628

#include <stdint.h>

namespace graphics
{

struct BMP;

class Texture
{
    const uint8_t* bits_{};
    int w_{};
    int h_{};
    int pitch_{};
    uint16_t palette_[256];

public:
    Texture() = default;
    Texture(const uint8_t* bits, int w, int h, int pitch)
        : bits_(bits)
        , w_(w)
        , h_(h)
        , pitch_(pitch)
    {
    }

    void initialize(const uint8_t* bits, int w, int h, int pitch)
    {
        bits_  = bits;
        w_     = w;
        h_     = h;
        pitch_ = pitch;
    }

    bool initialize(const BMP* bmp);

    const uint8_t* getBits() const { return bits_; }
    int getWidth() const { return w_; }
    int getHeight() const { return h_; }
    int getPitch() const { return pitch_; }
    const uint16_t* getPalette() const { return palette_; }
};

} // namespace graphics

#endif /* _00E1F8C7_7134_3DC8_3663_430FE012A628 */
