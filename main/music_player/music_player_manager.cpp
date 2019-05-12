/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 21:26:15
 */

#include "music_player_manager.h"

#include "play_list.h"
#include <debug.h>
#include <music_player/mdxplayer.h>
#include <mutex>
#include <string>
#include <system/mutex.h>
#include <ui/system_setting.h>

namespace music_player
{

namespace
{

MDXPlayer mdxPlayer_;

MusicPlayer* musicPlayers_[] = {&mdxPlayer_};

MusicPlayer* activeMusicPlayer_ = {};
std::string currentPlayListFile_;

struct Song
{
    std::string filename_;
    float playTime_ = -1;
    int listIndex_  = -1;

    void reset()
    {
        filename_.clear();
        playTime_  = -1;
        listIndex_ = -1;
    }
};
Song currentSong_;

sys::Mutex mutex_;

} // namespace

sys::Mutex&
getMutex()
{
    return mutex_;
}

MusicPlayer*
findMusicPlayerFromFile(const char* filename)
{
    for (auto p : musicPlayers_)
    {
        if (p->isSupported(filename))
        {
            return p;
        }
    }
    return nullptr;
}

void
terminateActiveMusicPlayerWithout(MusicPlayer* without)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    if (activeMusicPlayer_)
    {
        activeMusicPlayer_->stop();
        if (activeMusicPlayer_ != without)
        {
            activeMusicPlayer_->terminate();
        }
        activeMusicPlayer_ = nullptr;
    }
}

void
setActiveMusicPlayer(MusicPlayer* p)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    activeMusicPlayer_ = p;
}

MusicPlayer*
getActiveMusicPlayer()
{
    return activeMusicPlayer_;
}

bool
playMusicFile(const char* filename, int track, bool terminateOld)
{
    std::lock_guard<sys::Mutex> lock(mutex_);

    auto* player = findMusicPlayerFromFile(filename);
    if (!player)
    {
        return false;
    }

    if (terminateOld)
    {
        terminateActiveMusicPlayerWithout(player);
    }

    player->start();

    if (!player->load(filename))
    {
        player->terminate();
        return false;
    }

    setActiveMusicPlayer(player);

    return player->play(track);
}

void
tickMusicPlayerManager()
{
    auto player = getActiveMusicPlayer();
    if (player)
    {
        const auto& sysSettings = ui::SystemSettings::instance();
        auto repeatMode         = sysSettings.getRepeatMode();

        int lp = player->getCurrentLoop();
        if (repeatMode != ui::RepeatMode::SINGLE &&
            (lp >= sysSettings.getLoopCount() ||
             (currentSong_.playTime_ >= 0 &&
              player->getPlayTime() > currentSong_.playTime_)))
        {
            player->fadeout();
        }

        if (player->isFinished() &&
            player->getPlayTime() > 0.1f /* 0.1s 以上再生してから */)
        {
            if (repeatMode == ui::RepeatMode::SINGLE)
            {
                player->play(player->getCurrentTrack());
            }
            else
            {
                bool wrap = repeatMode == ui::RepeatMode::ALL;
                nextPlayList(wrap);
            }
        }
    }
}

bool
playPlayList(int idx, int track)
{
    auto& pl    = getDefaultPlayList();
    auto* entry = pl.get(idx);
    if (!entry)
    {
        currentSong_.reset();
        return false;
    }

    currentSong_.filename_  = entry->filename;
    currentSong_.listIndex_ = idx;
    currentSong_.playTime_  = entry->playTime;

    if (entry->track >= 0)
    {
        track = entry->track;
    }

    return playMusicFile(entry->filename.c_str(), track, true);
}

bool
nextPlayList(bool wrap)
{
    if (currentSong_.listIndex_ < 0)
    {
        return false;
    }

    auto& pl    = getDefaultPlayList();
    auto player = getActiveMusicPlayer();

    int track = 0;

    int nextIdx = currentSong_.listIndex_;
    if (ui::SystemSettings::instance().isShuffleMode())
    {
        auto* entry = pl.get(currentSong_.listIndex_);
        if (!entry)
        {
            return false;
        }

        nextIdx = pl.findOrder(entry->order + 1);
        if (nextIdx < 0 && wrap)
        {
            nextIdx = pl.findOrder(0);
        }
        if (nextIdx < 0)
        {
            return false;
        }
    }
    else
    {
        if (player)
        {
            int nTrack = player->getTrackCount();
            track      = player->getCurrentTrack();
            ++track;
            if (track >= nTrack)
            {
                track = 0;
            }
        }
        if (track == 0)
        {
            nextIdx = (currentSong_.listIndex_ + 1) % pl.getListCount();
            if (!wrap && nextIdx == 0)
                return false;
        }
    }

    return playPlayList(nextIdx, track);
}

bool
prevPlayList()
{
    if (currentSong_.listIndex_ < 0)
    {
        return false;
    }

    auto& pl = getDefaultPlayList();
    int ct   = pl.getListCount();

    int prevIdx;
    if (ui::SystemSettings::instance().isShuffleMode())
    {
        auto* entry = pl.get(currentSong_.listIndex_);
        if (!entry)
        {
            return false;
        }

        prevIdx = pl.findOrder((entry->order + ct - 1) % ct);
        if (prevIdx < 0)
        {
            return false;
        }
    }
    else
    {
        prevIdx = (currentSong_.listIndex_ + ct - 1) % ct;
    }

    return playPlayList(prevIdx);
}

constexpr float rewindTime_ = 0.5f;

bool
prevOrRewindPlayList()
{
    auto player = getActiveMusicPlayer();
    if (player)
    {
        float time = player->getPlayTime();
        if (time > rewindTime_)
        {
            return player->play(player->getCurrentTrack());
        }
    }

    return prevPlayList();
}

const std::string&
getCurrentPlayFile()
{
    return currentSong_.filename_;
}

} // namespace music_player
