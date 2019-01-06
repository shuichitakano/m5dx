/* -*- mode:C++; -*-
 *
 * ay8910.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/04/28(Tue) 04:04:06
 *
 * $Id$
 */

/*
 * include
 */

#include "ay8910.h"
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

AY8910::AY8910()
    : currentReg_(0)
    , chip_(0)
{
    memset(regCache_, 0, sizeof(regCache_));
    for (auto& inst : chInst_)
        inst = -1;

    sysInfo_.channelCount = 3;
    sysInfo_.systemID     = SoundSystem::SYSTEM_PSG;

    sysInfo_.clock          = 4000000;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_YMF288;
    sysInfo_.actualClock    = 4000000;
}

const AY8910::SystemInfo&
AY8910::getSystemInfo() const
{
    return sysInfo_;
}

float
AY8910::getNote(int ch, int) const
{
    return state_.getNote(ch, regCache_);
}

float
AY8910::getVolume(int ch) const
{
    return state_.getVolume(ch, regCache_);
}

float
AY8910::getPan(int ch) const
{
    return 0;
}

int
AY8910::getInstrument(int ch) const
{
    return chInst_[ch];
}

bool
AY8910::mute(int ch, bool f)
{
    return state_.mute(ch, f);
}

uint32_t
AY8910::getKeyOnChannels() const
{
    return state_.chKeyOn_;
}

uint32_t
AY8910::getKeyOnTrigger()
{
    return state_.getKeyOnTrigger();
}

uint32_t
AY8910::getEnabledChannels() const
{
    return state_.getEnabledChannels();
}

void
AY8910::setValue(int addr, int v)
{
    if (addr == 0)
        currentReg_ = v;
    else
    {
        bool r                 = state_.update(currentReg_, v, regCache_);
        regCache_[currentReg_] = v;
        if (!r)
            return;
    }

    if (chip_)
        chip_->setValue(addr, v);
}

int
AY8910::getValue(int addr)
{
    return regCache_[addr];
}

int
AY8910::setClock(int clock)
{
    state_.setClock(clock);
    sysInfo_.clock       = clock;
    sysInfo_.actualClock = chip_ ? chip_->setClock(clock) : 0;
    return sysInfo_.actualClock;
}

const char*
AY8910::getStatusString(int ch, char* buf, int n) const
{
    return state_.getStatusString(ch, buf, n, regCache_);
}

} /* namespace sound_sys */

/*
 * End of ay8910.cxx
 */
