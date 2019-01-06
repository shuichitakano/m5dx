/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/10/17(Fri) 02:47:25
 */
#ifndef _954F7CF9_5133_F06C_553B_AE019FB3560B
#define _954F7CF9_5133_F06C_553B_AE019FB3560B

#include "sound_system.h"
#include <stdint.h>

namespace audio
{
class SoundChipBase;
}

namespace sound_sys
{

class YM2151 final : public SoundSystem
{
    uint8_t regCache_[256]{};
    uint8_t chNonMute_{255};
    uint8_t chEnabled_{255};
    uint8_t chKeyOn_{0};
    uint8_t chKeyOnTrigger_{0};
    uint8_t currentReg_{0};
    float keyShift_{0};
    int16_t chInst_[8]{};

    audio::SoundChipBase* chip_{};
    SystemInfo sysInfo_;

public:
    YM2151();

    inline void setChip(audio::SoundChipBase* c) { chip_ = c; }
    audio::SoundChipBase* detachChip()
    {
        auto t = chip_;
        chip_  = 0;
        return t;
    }

    // SoundSystem
    float getNote(int ch, int) const override;
    const SystemInfo& getSystemInfo() const override;
    float getVolume(int ch) const override;
    float getPan(int ch) const override;
    int getInstrument(int ch) const override;
    bool mute(int ch, bool f) override;
    uint32_t getKeyOnChannels() const override;
    uint32_t getKeyOnTrigger() override;
    uint32_t getEnabledChannels() const override;
    const char* getStatusString(int ch, char* buf, int n) const override;

    // AudioChipBase
    void setValue(int addr, int v);
    int getValue(int addr);
    int setClock(int clock);

    inline void setInstrumentNumber(int ch, int n) { chInst_[ch] = n; }
};

// extern YM2151 ym2151Inst_[1];
// inline YM2151& getYM2151 (int id = 0)	{ return ym2151Inst_[id]; }

} /* namespace sound_sys */

#endif /* _954F7CF9_5133_F06C_553B_AE019FB3560B */
