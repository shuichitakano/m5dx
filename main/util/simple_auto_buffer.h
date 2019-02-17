/*
 * author : Shuichi TAKANO
 * since  : Sun Feb 17 2019 2:44:51
 */
#ifndef _73AB2F17_2134_13F8_254F_E6969FCA2D6B
#define _73AB2F17_2134_13F8_254F_E6969FCA2D6B

#include <stdint.h>
#include <stdlib.h>

namespace util
{

class SimpleAutoBuffer
{
    uint8_t* p_;
    bool free_;

public:
    SimpleAutoBuffer()
        : p_(0)
        , free_(false)
    {
    }

    SimpleAutoBuffer(void* p, bool fr = true)
        : p_((uint8_t*)p)
        , free_(fr)
    {
    }

    SimpleAutoBuffer(SimpleAutoBuffer&& r) noexcept
    {
        p_      = r.p_;
        free_   = r.free_;
        r.free_ = false;
    }

    SimpleAutoBuffer& operator=(SimpleAutoBuffer&& r)
    {
        p_      = r.p_;
        free_   = r.free_;
        r.free_ = false;
        return *this;
    }

    SimpleAutoBuffer(const SimpleAutoBuffer&) = delete;

    ~SimpleAutoBuffer()
    {
        if (p_ && free_)
        {
            free(p_);
        }
    }

    uint8_t* get() { return p_; }
    const uint8_t* get() const { return p_; }
};

} // namespace util

#endif /* _73AB2F17_2134_13F8_254F_E6969FCA2D6B */
