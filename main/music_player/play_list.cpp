/*
 * author : Shuichi TAKANO
 * since  : Wed Mar 06 2019 1:25:49
 */

#include "play_list.h"
#include <algorithm>
#include <random>
#include <system/util.h>

namespace music_player
{

PlayList&
getDefaultPlayList()
{
    static PlayList inst;
    return inst;
}

void
PlayList::clear()
{
    decltype(entries_)().swap(entries_);
}

void
PlayList::reserve(size_t n)
{
    entries_.reserve(n);
}

void
PlayList::add(const std::string& fn)
{
    entries_.push_back(Entry(fn));
}

void
PlayList::shuffle()
{
    std::mt19937 rdev(sys::micros());

    for (int i = 0; i < (int)entries_.size(); ++i)
    {
        entries_[i].order = i;
    }

    for (int i = 0; i < (int)entries_.size() - 1; ++i)
    {
        std::uniform_int_distribution<> dist(i + 1, (int)entries_.size() - 1);
        int r = dist(rdev);
        std::swap(entries_[i].order, entries_[r].order);
    }
}

const PlayList::Entry*
PlayList::get(int i) const
{
    return i < (int)entries_.size() ? &entries_[i] : 0;
}

int
PlayList::findOrder(int idx) const
{
    auto p = std::find_if(entries_.begin(),
                          entries_.end(),
                          [idx](const Entry& e) { return e.order == idx; });
    if (p == entries_.end())
        return -1;
    return p - entries_.begin();
}

} // namespace music_player
