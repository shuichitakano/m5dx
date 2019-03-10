/*
 * author : Shuichi TAKANO
 * since  : Wed Mar 06 2019 1:23:7
 */
#ifndef _8B3FECBA_0134_145F_12C8_B9B4BFDF47FC
#define _8B3FECBA_0134_145F_12C8_B9B4BFDF47FC

#include <string>
#include <vector>

namespace music_player
{

class PlayList
{
public:
    struct Entry
    {
        std::string filename;
        float playTime = -1;
        int track      = -1;
        int order      = -1;

        Entry(const std::string& fn, float time = -1, int tr = -1)
            : filename(fn)
            , playTime(time)
            , track(tr)
        {
        }
    };

public:
    void clear();
    void reserve(size_t n);
    inline size_t getListCount() const { return entries_.size(); }

    void add(const std::string& fn);
    void shuffle();

    const PlayList::Entry* get(int i) const;
    int findOrder(int idx) const;

private:
    std::vector<Entry> entries_;
};

PlayList& getDefaultPlayList();

} // namespace music_player

#endif /* _8B3FECBA_0134_145F_12C8_B9B4BFDF47FC */
