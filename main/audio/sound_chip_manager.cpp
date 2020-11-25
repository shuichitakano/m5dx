/*
 * author : Shuichi TAKANO
 * since  : Fri Nov 09 2018 2:7:39
 */

#include "sound_chip_manager.h"
#include "../debug.h"
#include "../target.h"
#include "opna_volume_adjuster.h"
#include <esp32-hal.h>

namespace audio
{

namespace
{

class FMChip : public SoundChipBase
{
public:
    enum class Mode
    {
        NONE,
        YM2151,
        YMF288,
    };

public:
    void setValue(int addr, int v) override
    {
#if 1
        if (mode_ != Mode::NONE)
        {
            bool useA1 = mode_ == Mode::YMF288;
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
        target::startFMClock(clock);
        return clock;
    }

    void setMode(Mode mode) { mode_ = mode; }

public:
    Mode mode_ = {};
};

FMChip chip_;
OPNAVolumeAdjuster chip288_(chip_);

} // namespace

SoundChipBase*
allocateYM2151()
{
    chip_.setMode(FMChip::Mode::YM2151);
    return &chip_;
}

SoundChipBase*
allocateYMF288()
{
    chip_.setMode(FMChip::Mode::YMF288);
    return &chip288_;
}

void
freeYM2151(SoundChipBase* p)
{
}

void
freeYMF288(SoundChipBase* p)
{
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

void
resetSoundChip()
{
    if (0)
    {
        // todo: 現在搭載中のチップをみて処理を変える必要がある
        for (int i = 0; i < 8; ++i)
        {
            chip_.setValue(0, 8);
            chip_.setValue(1, i);
        }
    }
    else
    {
        chip_.setMode(FMChip::Mode::YMF288);
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
}

} // namespace audio
