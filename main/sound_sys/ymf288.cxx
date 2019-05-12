/* -*- mode:C++; -*-
 *
 * ymf288.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/04/27(Mon) 00:46:59
 *
 * $Id$
 */

/*
 * include
 */

#include "ymf288.h"
#include <diag/Trace.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <stm32f4xx_hal.h>

/*
 * code
 */

namespace sound_sys
{

YMF288::YMF288()
    : chip_(0)
{
    currentReg_[0] = 0;
    currentReg_[1] = 0;
    memset(regCache_, 0, sizeof(regCache_));
    for (auto& inst : chInst_)
        inst = -1;

    sysInfo_.channelCount              = 15;
    sysInfo_.systemID                  = SoundSystem::SYSTEM_OPNA;
    static constexpr SystemID sysIDs[] = {
        SoundSystem::SYSTEM_PSG,
        SoundSystem::SYSTEM_PSG,
        SoundSystem::SYSTEM_PSG,

        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,

        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,
        SoundSystem::SYSTEM_OPNA,
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
        currentReg_[addr >> 1] = v;
    else
    {
        int reg        = addr & 2 ? 0x100 + currentReg_[1] : currentReg_[0];
        bool r         = state_.update(reg, v, regCache_);
        regCache_[reg] = v;
        if (!r)
            return;
    }

    if (chip_)
        chip_->setValue(addr, v);
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

const char*
YMF288::getStatusString(int ch, char* buf, int n) const
{
    return state_.getStatusString(ch, buf, n, regCache_);
}

} /* namespace sound_sys */

/*
 * End of ymf288.cxx
 */
