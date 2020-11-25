/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 03 2020 04:51:34
 */

#include "memory_stream.h"

namespace io
{

MemoryBinaryStream::MemoryBinaryStream(const void* p, size_t s)
{
    setMemory(p, s);
}

void
MemoryBinaryStream::setMemory(const void* p, size_t s)
{
    p_    = static_cast<const uint8_t*>(p);
    top_  = p_;
    size_ = s;
}

bool
MemoryBinaryStream::seek(int pos, bool tail)
{
    if (tail)
    {
        pos += (int)size_;
    }
    p_ = top_ + pos;
    return pos <= size_;
}

uint32_t
MemoryBinaryStream::tell() const
{
    return p_ - top_;
}

const char* MemoryBinaryStream::peek(size_t)
{
    return (const char*)p_;
}

BinaryStream::DataPtr
MemoryBinaryStream::get(size_t size)
{
    auto pp = p_;
    p_ += size;
    return makeRefPtr(pp);
}

bool
MemoryBinaryStream::advance(size_t size)
{
    p_ += size;
    return (p_ - top_) <= (int)size_;
}

bool
MemoryBinaryStream::isEndOfStream() const
{
    return p_ - top_ >= (int)size_;
}

uint_fast8_t
MemoryBinaryStream::getU8()
{
    return *p_++;
}

uint_fast16_t
MemoryBinaryStream::getU16()
{
    p_ += 2;
    return p_[-2] | (p_[-1] << 8);
}

uint_fast32_t
MemoryBinaryStream::getU32()
{
    p_ += 4;
    return p_[-4] | (p_[-3] << 8) | (p_[-2] << 16) | (p_[-1] << 24);
}

uint_fast16_t
MemoryBinaryStream::getU16Aligned()
{
    // for LE system
    uint_fast16_t v = *(const uint16_t*)p_;
    p_ += 2;
    return v;
}

uint_fast32_t
MemoryBinaryStream::getU32Aligned()
{
    // for LE system
    uint_fast32_t v = *(const uint32_t*)p_;
    p_ += 4;
    return v;
}

} // namespace io
