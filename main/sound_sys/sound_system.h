/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 04 2018 6:2:19
 */
#ifndef _8CE7218C_8133_F06C_5BE2_43E774077F9E
#define _8CE7218C_8133_F06C_5BE2_43E774077F9E

#include <stdint.h>

namespace sound_sys
{

class SoundSystem
{
public:
    enum SystemID
    {
        SYSTEM_INVALID = -1,

        SYSTEM_YM2151 = 0,
        SYSTEM_M6258,
        SYSTEM_YMF288,
        SYSTEM_Y8950,
        SYSTEM_MIDI,

        SYSTEM_REBIRTH,
        SYSTEM_SPFM,
        SYSTEM_EMULATION,
        SYSTEM_FM1,

        SYSTEM_SEGA_PCM = 26,
        SYSTEM_M6295,
        SYSTEM_054539,
        SYSTEM_053260,

        SYSTEM_PSG,
        SYSTEM_SCC,
        SYSTEM_OPL,
        SYSTEM_PCM8,
        SYSTEM_C140,

        SYSTEM_OPNA,
        SYSTEM_OPNB,
        SYSTEM_OPN,
        SYSTEM_OPM,
        SYSTEM_OPLL,
    };

    struct SystemInfo
    {
        SystemID systemID{SYSTEM_INVALID}; //< 本来のシステム
        const SystemID* systemIDPerCh{}; //< チャネル毎に独立のシステムID
        int systemSubIndex{0};           //< システムの番号
        int clock{};                     //< 本来のクロック
        SystemID actualSystemID{SYSTEM_INVALID}; //< 実駆動システム
        int actualClock{};                       //< 実駆動クロック
        int channelCount{};                      //< チャネル数
        uint32_t oneShotChannelMask{}; //< 単発再生なスロット識別用
        bool multiVoice{};
    };

public:
    virtual ~SoundSystem() noexcept = default;

    virtual const SystemInfo& getSystemInfo() const = 0;

    // 440Hzを0とした半音単位の音階を返す
    virtual float getNote(int ch, int voice) const = 0;
    virtual int getNoteCount(int ch) const { return 1; };

    // log scaleで0-1の音量を返す
    virtual float getVolume(int ch) const = 0;

    virtual float getPan(int ch) const      = 0;
    virtual int getInstrument(int ch) const = 0;
    virtual bool mute(int ch, bool f)       = 0;

    virtual uint32_t getKeyOnChannels() const   = 0;
    virtual uint32_t getKeyOnTrigger()          = 0;
    virtual uint32_t getEnabledChannels() const = 0;

    virtual const char* getStatusString(int ch, char* buf, int n) const = 0;
};

} // namespace sound_sys

#endif /* _8CE7218C_8133_F06C_5BE2_43E774077F9E */
