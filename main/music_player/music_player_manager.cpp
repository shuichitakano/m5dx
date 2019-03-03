/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 21:26:15
 */

#include "music_player_manager.h"

#include <debug.h>
#include <music_player/mdxplayer.h>
#include <mutex>
#include <sys/mutex.h>

namespace music_player
{

namespace
{

MDXPlayer mdxPlayer_;

MusicPlayer* musicPlayers_[] = {&mdxPlayer_};

MusicPlayer* activeMusicPlayer_ = {};

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

} // namespace music_player
