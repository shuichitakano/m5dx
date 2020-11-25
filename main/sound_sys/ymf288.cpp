/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2015/04/27(Mon) 00:46:59
 */

#include "ymf288.h"
#include <audio/sound_chip.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace sound_sys
{

YMF288::YMF288()
{
    currentReg_[0] = 0;
    currentReg_[1] = 0;
    memset(regCache_, 0, sizeof(regCache_));
    for (auto& inst : chInst_)
    {
        inst = -1;
    }

    sysInfo_.channelCount = 15;
    sysInfo_.systemID     = SoundSystem::SYSTEM_YMF288;

    static constexpr SystemID sysIDs[] = {
        SoundSystem::SYSTEM_YMF288, // PSG
        SoundSystem::SYSTEM_YMF288, // PSG
        SoundSystem::SYSTEM_YMF288, // PSG

        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,

        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,
        SoundSystem::SYSTEM_YMF288,
    };
    sysInfo_.systemIDPerCh      = sysIDs;
    sysInfo_.oneShotChannelMask = 63 << 9;

    sysInfo_.clock          = 8000000;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_YMF288;
    sysInfo_.actualClock    = 8000000;
}

const YMF288::SystemInfo&
YMF288::getSystemInfo() const
{
    return sysInfo_;
}

float
YMF288::getNote(int ch, int) const
{
    return state_.getNote(ch, regCache_);
}

float
YMF288::getVolume(int ch) const
{
    return state_.getVolume(ch, regCache_);
}

float
YMF288::getPan(int ch) const
{
    return state_.getPan(ch, regCache_);
}

int
YMF288::getInstrument(int ch) const
{
    return chInst_[ch];
}

bool
YMF288::mute(int ch, bool f)
{
    return state_.mute(ch, f);
}

uint32_t
YMF288::getKeyOnChannels() const
{
    return state_.chKeyOn_;
}

uint32_t
YMF288::getKeyOnTrigger()
{
    return state_.getKeyOnTrigger();
}

uint32_t
YMF288::getEnabledChannels() const
{
    return state_.getEnabledChannels();
}

void
YMF288::setValue(int addr, int v)
{
    if ((addr & 1) == 0)
    {
        currentReg_[addr >> 1] = v;
    }
    else
    {
        int reg        = addr & 2 ? 0x100 + currentReg_[1] : currentReg_[0];
        bool r         = state_.update(reg, v, regCache_);
        regCache_[reg] = v;
        if (!r)
        {
            return;
        }
    }

    if (chip_)
    {
        chip_->setValue(addr, v);
    }
}

int
YMF288::getValue(int addr)
{
    return regCache_[addr];
}

int
YMF288::setClock(int clock)
{
    state_.setClock(clock);
    sysInfo_.clock       = clock;
    sysInfo_.actualClock = chip_ ? chip_->setClock(clock) : 0;
    return sysInfo_.actualClock;
}

void
YMF288::allKeyOff()
{
    auto set = [&](int r, int v) {
        setValue(0, r);
        setValue(1, v);
    };

    set(0x7, 63);
    set(0x8, 0);
    set(0x9, 0);
    set(0xa, 0);
    set(0x10, 0x80 + 63);
    for (int i = 0; i < 7; ++i)
    {
        set(0x28, i);
    }
}

// void
// YMF288::reset()
// {
//     setValue(0x11, 0);
//     allKeyOff();
// }

} /* namespace sound_sys */
