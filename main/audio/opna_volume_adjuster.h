/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 17 2020 17:12:37
 */
#ifndef A029EF1A_D134_3E2E_1054_88EDCF03E95E
#define A029EF1A_D134_3E2E_1054_88EDCF03E95E

#include "sound_chip.h"

namespace audio
{

class OPNAVolumeAdjuster final : public SoundChipBase
{
public:
    OPNAVolumeAdjuster(SoundChipBase& base)
        : base_(base)
    {
    }

    void setValue(int addr, int v) override;

    int getValue(int addr) override { return base_.getValue(addr); }
    int setClock(int clock) override { return base_.setClock(clock); }

    void setFMAdjust(int v) { adjFM_ = v; }
    void setRhythmAdjust(int v) { adjRhythm_ = v; }

private:
    SoundChipBase& base_;
    int currentReg_[2];
    int algorithm_[2][4] = {};

    int adjFM_     = 0;
    int adjRhythm_ = 0;
};

} // namespace audio

#endif /* A029EF1A_D134_3E2E_1054_88EDCF03E95E */
