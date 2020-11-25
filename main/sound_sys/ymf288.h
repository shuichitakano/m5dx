/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2015/04/27(Mon) 00:47:56
 */
#ifndef _70DE98BE_D134_3E2C_41D3_ABDC2228B9DD
#define _70DE98BE_D134_3E2C_41D3_ABDC2228B9DD

#include "sound_system.h"
#include <stdint.h>

#include "opna_common.h"

namespace audio
{
class SoundChipBase;
}

namespace sound_sys
{

class YMF288 : public SoundSystem
{
    OPNAState state_;
    uint8_t regCache_[512];

    uint16_t currentReg_[2];
    int16_t chInst_[16];

    audio::SoundChipBase* chip_{};
    SystemInfo sysInfo_;

public:
    YMF288();

    void setChip(audio::SoundChipBase* c) { chip_ = c; }
    audio::SoundChipBase* detachChip()
    {
        auto t = chip_;
        chip_  = nullptr;
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

    // SoundChipBase
    void setValue(int addr, int v);
    int getValue(int addr);
    int setClock(int clock);

    void allKeyOff();
    // virtual void reset() = 0;
    //    virtual void keyOn(int ch, int note, int fine) = 0;
    //    virtual void keyOff(int ch, int note)          = 0;

    void setInstrumentNumber(int ch, int n) { chInst_[ch] = n; }
};

} /* namespace sound_sys */

#endif /* _70DE98BE_D134_3E2C_41D3_ABDC2228B9DD */
