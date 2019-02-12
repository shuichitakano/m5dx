/*
 * author : Shuichi TAKANO
 * since  : Fri Nov 09 2018 1:20:53
 */
#ifndef _4C44B068_0133_F071_1262_0E78152C1627
#define _4C44B068_0133_F071_1262_0E78152C1627

#include "file_format.h"
#include <experimental/optional>
#include <stdint.h>
#include <string>

namespace sound_sys
{
class SoundSystem;
}

namespace music_player
{

class MusicPlayer
{
public:
    virtual ~MusicPlayer() noexcept = default;

    virtual bool isSupported(const char* filename) = 0;
    virtual std::experimental::optional<std::string>
    loadTitle(const char* filename) = 0;

    virtual bool start()                               = 0;
    virtual bool terminate()                           = 0;
    virtual bool load(const char* filename)            = 0;
    virtual bool play(int track = -1)                  = 0;
    virtual bool stop()                                = 0;
    virtual bool pause()                               = 0;
    virtual bool cont()                                = 0;
    virtual bool fadeout()                             = 0;
    virtual bool isFinished() const                    = 0;
    virtual bool isPaused() const                      = 0;
    virtual int getCurrentLoop() const                 = 0;
    virtual int getTrackCount() const                  = 0;
    virtual int getCurrentTrack() const                = 0;
    virtual float getPlayTime() const                  = 0;
    virtual const char* getTitle() const               = 0;
    virtual FileFormat getFormat() const               = 0;
    virtual sound_sys::SoundSystem* getSystem(int idx) = 0;
};

} // namespace music_player
#endif /* _4C44B068_0133_F071_1262_0E78152C1627 */
