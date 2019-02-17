/*
 * author : Shuichi TAKANO
 * since  : Sun Feb 17 2019 3:7:6
 */

#include "data_block.h"
#include <algorithm>
#include <stdio.h>

namespace util
{

void
DataBlockContainer::addEntry(uint32_t addr,
                             SimpleAutoBuffer&& data,
                             uint32_t size)
{
    if (!size)
    {
        return;
    }

    entries_.emplace_back(addr, std::move(data), size);
    std::sort(entries_.begin(), entries_.end());
}

const uint8_t*
DataBlockContainer::find(uint32_t addr) const
{
    if (entries_.empty())
    {
        return nullptr;
    }

    auto p = std::lower_bound(entries_.begin(), entries_.end(), addr);
    if (p != entries_.end())
    {
        if (addr >= p->addr)
        {
            auto ofs = (int32_t)(addr - p->addr);
            return p->data.get() + ofs;
        }
    }
    return nullptr;
}

void
DataBlockContainer::clear()
{
    decltype(entries_)().swap(entries_);
}

} // namespace util
