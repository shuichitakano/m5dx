/*
 * author : Shuichi TAKANO
 * since  : Fri Nov 09 2018 2:7:39
 */

#include "sound_chip_manager.h"
#include "../debug.h"
#include "../target.h"
#include <esp32-hal.h>

namespace audio
{

namespace
{

class FMChip : public SoundChipBase
{
public:
    void setValue(int addr, int v) override
    {
#if 1
        target::setupBus();

        target::setFMA0(addr);
        target::writeBusData(v);

        target::assertFMCS();

        delayMicroseconds(1); // 250nsくらいでいい

        target::negateFMCS();

        // 3.579545MHz 68clock = 19us
        delayMicroseconds(19);

        target::restoreBus();
#endif
    }

    int getValue(int addr) override { return 0; }

    int setClock(int clock) override
    {
        DBOUT(("setClock: %d\n", clock));
        target::startFMClock(clock);
        return clock;
    }

    void activate() override { active_ = true; }
    void deactivate() override { active_ = false; }

public:
    bool active_ = false;
};

FMChip ym2151_;

} // namespace

SoundChipBase*
allocateYM2151()
{
    return &ym2151_;
}

void
freeYM2151(SoundChipBase* p)
{
}

void
resetSoundChip()
{
    for (int i = 0; i < 8; ++i)
    {
        ym2151_.setValue(0, 8);
        ym2151_.setValue(1, i);
    }
}

} // namespace audio
