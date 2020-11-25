/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 03 2020 04:13:33
 */

#include "stream.h"
#include <string.h>

namespace io
{

char
BinaryStream::getChar()
{
    const char* p = peek(1);
    if (!p)
    {
        return 0;
    }
    char v = p[0];
    advance(1);
    return v;
}

uint_fast8_t
BinaryStream::getU8()
{
    return (uint8_t)getChar();
}

uint_fast16_t
BinaryStream::getU16()
{
    const uint8_t* p = (const uint8_t*)peek(2);
    if (!p)
    {
        return 0;
    }
    uint16_t v = p[0] | (p[1] << 8);
    advance(2);
    return v;
}

uint_fast32_t
BinaryStream::getU32()
{
    const uint8_t* p = (const uint8_t*)peek(4);
    if (!p)
    {
        return 0;
    }
    uint32_t v = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    advance(4);
    return v;
}

float
BinaryStream::getFloat()
{
    union
    {
        uint32_t i_;
        float f_;
    };
    i_ = getU32();
    return f_;
}

float
BinaryStream::getFloatAligned()
{
    union
    {
        uint32_t i_;
        float f_;
    };
    i_ = getU32Aligned();
    return f_;
}

uint_fast16_t
BinaryStream::getU16Aligned()
{
    return getU16();
}

uint_fast32_t
BinaryStream::getU32Aligned()
{
    return getU32();
}

BinaryStream::DataPtr
BinaryStream::get(size_t size)
{
    auto p = peek(size);
    if (!p)
    {
        return {};
    }

    auto r = makeDataPtr(size);
    memcpy(const_cast<uint8_t*>(r.get()), p, size);
    advance(size);
    return r;
}

} // namespace io
