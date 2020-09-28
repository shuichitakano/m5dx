/*
 * author : Shuichi TAKANO
 * since  : Thu Sep 10 2020 03:58:42
 */

#include "texture.h"
#include "bmp.h"
#include "color.h"

namespace graphics
{

bool
Texture::initialize(const BMP* bmp)
{
    if (!bmp->isBMP() || bmp->getBitCount() != 8)
    {
        return false;
    }

    auto w     = bmp->getWidth();
    auto h     = bmp->getHeight();
    auto pitch = ((w + 3) & ~3);
    auto bits  = static_cast<const uint8_t*>(bmp->getBits()) + pitch * (h - 1);
    initialize(bits, w, h, -pitch);

    int nPal  = bmp->getPaletteCount();
    auto* pal = bmp->getPalette();
    for (int i = 0; i < nPal; ++i)
    {
        int r       = pal->getR();
        int g       = pal->getG();
        int b       = pal->getB();
        palette_[i] = makeColor16(r, g, b);
        ++pal;
    }

    return true;
}

} // namespace graphics
