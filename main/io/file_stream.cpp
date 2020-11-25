/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 03 2020 04:26:18
 */

#include "file_stream.h"
#include <assert.h>
#include <stdlib.h>

namespace io
{

bool
FileBinaryStream::seek(int pos, bool tail)
{
    if (tail)
    {
        assert(size_ >= 0);
        pos += size_;
    }
    fseek(fp_, pos, SEEK_SET);
    adj_ = 0;
    return true;
}

uint32_t
FileBinaryStream::tell() const
{
    return ftell(fp_) + adj_;
}

void
FileBinaryStream::adjustPointer() const
{
    if (adj_ != 0)
    {
        fseek(fp_, adj_, SEEK_CUR);
        adj_ = 0;
    }
}

const char*
FileBinaryStream::peek(size_t size)
{
    adjustPointer();

    if (cache_.size() < size)
    {
        cache_.resize(size);
    }

    fread(cache_.data(), 1, size, fp_);
    adj_ = -size;
    return cache_.data();
}

BinaryStream::DataPtr
FileBinaryStream::get(size_t size)
{
    auto p = makeDataPtr(size);
    if (!p)
    {
        return {};
    }

    adjustPointer();
    fread(const_cast<uint8_t*>(p.get()), 1, size, fp_);
    return p;
}

bool
FileBinaryStream::advance(size_t size)
{
    adj_ += size;
    return true;
}

bool
FileBinaryStream::isEndOfStream() const
{
    adjustPointer();
    return feof(fp_);
}

void
FileBinaryStream::flush()
{
    adjustPointer();
}

} // namespace io
