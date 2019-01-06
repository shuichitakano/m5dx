/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 27 2018 5:39:45
 */

#include "font_data.h"

namespace graphics
{

void
FontData::setData(const void* data)
{
    data_     = (const Header*)data;
    glyph_    = 0;
    unitSize_ = 0;

    if (!data_)
        return;

    if (data_->magic[0] != 'F' || data_->magic[1] != 'N' ||
        data_->magic[2] != 'T')
    {
        data_ = 0;
        return;
    }

    glyph_    = data_->getGlyph();
    unitSize_ = ((data_->width + 7) >> 3) * data_->height;
}

const uint8_t*
FontData::getGlyph(int code) const
{
    if (!data_)
        return 0;

    code -= data_->codeOfs;
    int upper = code >> 8;
    if ((uint32_t)upper >= data_->entryCount)
        return 0;

    const Entry& e = data_->entry[upper];
    int lower      = code & 255;
    if (lower < e.begin || lower > e.end)
        return 0;

    int idx = e.idx + lower;
    return glyph_ + unitSize_ * idx;
}

} // namespace graphics
