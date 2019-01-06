/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/06/30(Mon) 03:17:00
 *
 */

#ifndef _4560C35E_C133_F071_1515_1239749E5F78
#define _4560C35E_C133_F071_1515_1239749E5F78

#include "music_player.h"
#include <sound_sys/m6258.h>
#include <sound_sys/ym2151.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace music_player
{

class MDXPlayer : public MusicPlayer
{
    typedef std::vector<uint8_t> ByteArray;
    ByteArray mdx_;
    ByteArray pdx_;

    std::string currentPDXPath_;
    std::string pdxPath_;

    std::string title_;

    bool initialized_{false};
    bool fadeout_{false};
    bool paused_{false};
    uint32_t playTimeByClock_{0};

    sound_sys::YM2151 ym2151_;
    sound_sys::M6258 m6258_;

public:
    bool loadMDX(const char* filename);

    bool isSupported(const char* filename) override;
    std::experimental::optional<std::string>
    loadTitle(const char* filename) override;
    bool start() override;
    bool terminate() override;
    bool load(const char* filename) override;
    bool play(int track) override;
    bool stop() override;
    bool pause() override;
    bool cont() override;
    bool fadeout() override;
    bool isFinished() const override;
    bool isPaused() const override;
    int getCurrentLoop() const override;
    int getTrackCount() const override;
    int getCurrentTrack() const override;
    float getPlayTime() const override;
    const char* getTitle() const override;
    const char* getFormat() const override;
    sound_sys::SoundSystem* getSystem(int idx) override;

    const std::string& getPDXPath() const { return pdxPath_; }
    void setPDXPath(const char* s);

protected:
    bool analyzeTitle();
    bool analyzePDXFilename(std::string& name) const;

    bool loadPDX(const char* mdxFilename, const std::string& pdxName);

    void freeAudioChips();
};

} // namespace music_player

#endif /* _4560C35E_C133_F071_1515_1239749E5F78 */
