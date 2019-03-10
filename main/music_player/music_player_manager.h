/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 21:25:0
 */
#ifndef _5B147C5A_8134_13F9_1444_443F1EFDAF7D
#define _5B147C5A_8134_13F9_1444_443F1EFDAF7D

#include "music_player.h"
#include <string>

namespace sys
{
class Mutex;
}

namespace music_player
{

MusicPlayer* findMusicPlayerFromFile(const char* filename);

void terminateActiveMusicPlayerWithout(MusicPlayer* without);
void setActiveMusicPlayer(MusicPlayer* p);
MusicPlayer* getActiveMusicPlayer();
bool
playMusicFile(const char* filename, int track = -1, bool terminateOld = true);

void tickMusicPlayerManager();

bool playPlayList(int idx, int track = 0);
bool nextPlayList(bool wrap = false);
bool prevPlayList();
bool prevOrRewindPlayList();

const std::string& getCurrentPlayFile();

sys::Mutex& getMutex();

} // namespace music_player

#endif /* _5B147C5A_8134_13F9_1444_443F1EFDAF7D */
