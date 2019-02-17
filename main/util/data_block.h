/*
 * author : Shuichi TAKANO
 * since  : Sun Feb 17 2019 2:47:57
 */
#ifndef _222E94BF_1134_13F8_25C7_BFAC9CFFC667
#define _222E94BF_1134_13F8_25C7_BFAC9CFFC667

#include "simple_auto_buffer.h"
#include <memory>
#include <stdint.h>
#include <vector>

namespace util
{

class DataBlockContainer
{
    struct Entry
    {
        uint32_t addr;
        uint32_t tail;
        SimpleAutoBuffer data;

        Entry(uint32_t a, SimpleAutoBuffer&& p, uint32_t s)
            : addr(a)
            , tail(a + s - 1)
            , data(std::move(p))
        {
        }

        friend bool operator<(const Entry& a, const Entry& b)
        {
            return a.tail < b.tail;
        }
        friend bool operator<(const Entry& a, uint32_t b) { return a.tail < b; }
        friend bool operator<(uint32_t a, const Entry& b) { return a < b.tail; }
    };

    std::vector<Entry> entries_;

public:
    void addEntry(uint32_t addr, SimpleAutoBuffer&& data, uint32_t size);

    const uint8_t* find(uint32_t addr) const;
    void clear();
};

} // namespace util

#endif /* _222E94BF_1134_13F8_25C7_BFAC9CFFC667 */
