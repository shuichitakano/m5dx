/*
 * author : Shuichi TAKANO
 * since  : Fri Nov 09 2018 2:7:39
 */

#include "sound_chip_manager.h"
#include "../debug.h"
#include "../target.h"
#include "audio.h"
#include "opna_volume_adjuster.h"
#include <esp32-hal.h>
#include <system/util.h>

namespace audio
{

namespace
{

enum class ChipType
{
    NONE,
    YM2151,
    YMF288,
};

ChipType attachedChip_ = ChipType::NONE;

class FMChip : public SoundChipBase
{
public:
    void setValue(int addr, int v) override
    {
#if 1
        if (mode_ != ChipType::NONE)
        {
            bool useA1 = mode_ == ChipType::YMF288;
            target::setupBus(useA1);

            target::writeBusData(v);
            target::setFMA0(addr & 1);
            if (useA1)
            {
                target::setFMA1(addr & 2 ? 1 : 0);
            }

            target::assertFMCS();

            delayMicroseconds(1); // 250nsくらいでいい

            target::negateFMCS();

            // 3.579545MHz 68clock = 19us
            delayMicroseconds(10);
            target::setBusIdle();
            delayMicroseconds(9);

            target::restoreBus(useA1);
        }
#endif
    }

    int getValue(int addr) override { return 0; }

    int setClock(int clock) override
    {
        DBOUT(("setClock: %d\n", clock));
        //        target::startFMClock(clock);
        setFMClock(clock);
        return clock;
    }

    void setMode(ChipType mode) { mode_ = mode; }

public:
    ChipType mode_ = {};
};

FMChip chip_;
OPNAVolumeAdjuster chip288_(chip_);

bool occupied_ = false;

} // namespace

void
attachYM2151()
{
    DBOUT(("attach chip: YM2151\n"));
    attachedChip_ = ChipType::YM2151;
    setFMAudioModeYM2151();
}

void
atatchYMF288()
{
    DBOUT(("attach chip: YM2151\n"));
    attachedChip_ = ChipType::YMF288;
    setFMAudioModeYMF288();
}

void
detachAllChip()
{
    DBOUT(("detach all chip\n"));
    attachedChip_ = ChipType::NONE;
}

//

SoundChipBase*
allocateYM2151()
{
    if (!occupied_ && attachedChip_ == ChipType::YM2151)
    {
        occupied_ = true;
        chip_.setMode(ChipType::YM2151);
        return &chip_;
    }
    return nullptr;
}

SoundChipBase*
allocateYMF288()
{
    if (!occupied_ && attachedChip_ == ChipType::YMF288)
    {
        occupied_ = true;
        chip_.setMode(ChipType::YMF288);
        return &chip288_;
    }
    return nullptr;
}

void
freeYM2151(SoundChipBase* p)
{
    occupied_ = false;
}

void
freeYMF288(SoundChipBase* p)
{
    occupied_ = false;
}

void
setYMF288FMVolume(int adj)
{
    chip288_.setFMAdjust(adj);
}

void
setYMF288RhythmVolume(int adj)
{
    chip288_.setRhythmAdjust(adj);
}

namespace
{

void
resetYM2151()
{
    DBOUT(("resetYM2151\n"));
    chip_.setMode(ChipType::YM2151);
    chip_.setClock(3579545);
    sys::delay(1);
    for (int i = 0; i < 8; ++i)
    {
        chip_.setValue(0, 8);
        chip_.setValue(1, i);
    }
}

void
resetYMF288()
{
    DBOUT(("resetYMF288\n"));
    chip_.setMode(ChipType::YMF288);
    chip_.setClock(8000000);
    sys::delay(1);
    for (int i = 0; i < 256; ++i)
    {
        chip_.setValue(0, i);
        chip_.setValue(1, 0);
    }
    for (int i = 0; i < 256; ++i)
    {
        chip_.setValue(2, i);
        chip_.setValue(3, 0);
    }
    chip_.setValue(0, 0x2d); // set prescaler : fm 1/6, psg 1/4

    chip_.setValue(0, 0x29);
    chip_.setValue(1, 0);

    for (int i = 0; i < 7; ++i)
    {
        chip_.setValue(0, 0x28);
        chip_.setValue(1, i);
    }
    for (int i = 0; i < 3; ++i)
    {
        chip_.setValue(0, 0xb4 + i);
        chip_.setValue(1, 128 + 64);
    }
}

} // namespace

void
resetSoundChip()
{
    DBOUT(("resetSoundChip %d\n", (int)attachedChip_));
    switch (attachedChip_)
    {
    case ChipType::YM2151:
        resetYM2151();
        break;

    case ChipType::YMF288:
        resetYMF288();
        break;

    default:
        break;
    }
}

} // namespace audio
