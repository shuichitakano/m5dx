/* -*- mode:C++; -*-
 *
 * ym2203.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/04/27(Mon) 02:30:30
 *
 * $Id$
 */

/*
 * include
 */

#include "ym2203.h"
#include <algorithm>
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

namespace
{
enum
{
    SLOT1 = 0,
    SLOT2 = 2,
    SLOT3 = 1,
    SLOT4 = 3,
};

int
getCarrierMask(int algorithm)
{
    int masks[] = {
        (1 << SLOT4),
        (1 << SLOT4),
        (1 << SLOT4),
        (1 << SLOT4),
        (1 << SLOT2) | (1 << SLOT4),
        (1 << SLOT2) | (1 << SLOT3) | (1 << SLOT4),
        (1 << SLOT2) | (1 << SLOT3) | (1 << SLOT4),
        (1 << SLOT1) | (1 << SLOT2) | (1 << SLOT3) | (1 << SLOT4),
    };
    return masks[algorithm & 7];
}
} // namespace

YM2203::YM2203()
    : chNonMute_(65535)
    , chEnabled_(65535)
    , chKeyOn_(0)
    , chKeyOnTrigger_(0)
    , currentReg_(0)
    , chip_(0)
    , fBasePSG_(0.1059638129340278f)
    , fBaseFM_(0.1059638129340278f)
{
    memset(regCache_, 0, sizeof(regCache_));
    for (auto& inst : chInst_)
        inst = -1;

    sysInfo_.channelCount              = 6;
    sysInfo_.systemID                  = SoundSystem::SYSTEM_OPN;
    static constexpr SystemID sysIDs[] = {
        SoundSystem::SYSTEM_PSG,
        SoundSystem::SYSTEM_PSG,
        SoundSystem::SYSTEM_PSG,
        SoundSystem::SYSTEM_OPN,
        SoundSystem::SYSTEM_OPN,
        SoundSystem::SYSTEM_OPN,
    };
    sysInfo_.systemIDPerCh = sysIDs;

    sysInfo_.clock          = 4000000;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_YMF288;
    sysInfo_.actualClock    = 4000000;
}

const YM2203::SystemInfo&
YM2203::getSystemInfo() const
{
    return sysInfo_;
}

float
YM2203::getNote(int ch, int) const
{
    if (ch < 3)
    {
        // PSG
        int rl = regCache_[0 + (ch << 1)];
        int rh = regCache_[1 + (ch << 1)];
        int tp = ((rh << 8) | rl) & 4095;
        if (tp == 0)
            return -1000; // 適当。keyoffとして扱う？
        // f = clock*2 / (64 * tp)
        // key = 12 log_2(f/440)
        //     = 12 log_2(clock*2/64/440) - 12 log_2(tp)
        return fBasePSG_ - 12.0f * log2(tp);
    }
    else
    {
        ch -= 3;
        // FM
        int rl  = regCache_[0xa0 + ch];
        int rh  = regCache_[0xa4 + ch];
        int fn  = ((rh << 8) | rl) & 2047;
        int blk = ((rh >> 3) & 7) - 1;

        // fn = f * 2^20/(clock*2 / 144)/2^blk
        // f = fn * clock*2/144 * 2^blk / 2^20
        // key = 12 log_2(f/440)
        float f = fBaseFM_ * (fn << blk);
        return 17.31234f * log(f) - 105.3763f;
    }
    return 0;
}

float
YM2203::getVolume(int ch) const
{
    if (ch < 3)
        return (regCache_[0x8 + ch] & 15) * (1.0f / 15);
    else
    {
        ch -= 3;
        int al  = regCache_[0xb0 + ch] & 7;
        int cm  = getCarrierMask(al);
        int tl0 = (cm & (1 << 0) ? regCache_[0x40 + 4 * 0 + ch] & 127 : 127);
        int tl1 = (cm & (1 << 1) ? regCache_[0x40 + 4 * 1 + ch] & 127 : 127);
        int tl2 = (cm & (1 << 2) ? regCache_[0x40 + 4 * 2 + ch] & 127 : 127);
        int tl3 = (cm & (1 << 3) ? regCache_[0x40 + 4 * 3 + ch] & 127 : 127);
        return 1.0f -
               std::min(tl0, std::min(tl1, std::min(tl2, tl3))) * (1 / 127.0f);
    }
    return 0;
}

float
YM2203::getPan(int ch) const
{
    return 0;
}

int
YM2203::getInstrument(int ch) const
{
    return chInst_[ch];
}

bool
YM2203::mute(int ch, bool f)
{
    if (f)
        chNonMute_ &= ~(1 << ch);
    else
        chNonMute_ |= 1 << ch;
    return true;
}

uint32_t
YM2203::getKeyOnChannels() const
{
    return chKeyOn_;
}

uint32_t
YM2203::getKeyOnTrigger()
{
    //  __disable_irq ();
    uint32_t v      = chKeyOnTrigger_;
    chKeyOnTrigger_ = 0;
    //  __enable_irq ();
    return v;
}

uint32_t
YM2203::getEnabledChannels() const
{
    return chNonMute_ & chEnabled_;
}

void
YM2203::setValue(int addr, int v)
{
    if (addr == 0)
        currentReg_ = v;
    else
    {
        int prev               = regCache_[currentReg_];
        regCache_[currentReg_] = v;

        if (currentReg_ == 0x28)
        {
            // fm keyon
            static const int chmap[] = {0, 1, 2, 0};
            int ch                   = chmap[v & 3];
            int chBit                = (1 << 3) << ch;
            if (v & (15 << 4))
            {
                if (chNonMute_ & chBit)
                {
                    chKeyOn_ |= chBit;
                    chKeyOnTrigger_ |= chBit;
                }
                else
                    return;
            }
            else
                chKeyOn_ &= ~chBit;
        }
        else if (currentReg_ == 0x7)
        {
            // psg keyon
            int ko = (v | (v >> 3)) & 7;
            int t  = (~chKeyOn_) & ko;
            chKeyOn_ |= ko;
            chKeyOnTrigger_ |= t;

            int mask = chNonMute_ & 7;
            v &= (mask << 3) | mask;
        }
        else if (currentReg_ >= 0x8 && currentReg_ <= 0xa)
        {
            // keyonとして音量変化も見てみる
            int ch    = currentReg_ - 0x8;
            int chBit = 1 << ch;
            if (chKeyOn_ & chBit)
            {
                int plv = prev & 15;
                int lv  = v & 15;
                if (lv > plv)
                    chKeyOnTrigger_ |= chBit;
            }

            if (!(chNonMute_ & chBit))
                v &= ~15;
        }
        //      else if (currentReg_ < 6)
        //        {
        //          int rl = regCache_[0 + (ch << 1)];
        //          int rh = regCache_[1 + (ch << 1)];
        //          int tp = ((rh << 8) | rl) & 4095;
        //        }
    }

    if (chip_)
        chip_->setValue(addr, v);
}

int
YM2203::getValue(int addr)
{
    return chip_ ? chip_->getValue(addr) : regCache_[currentReg_];
}

int
YM2203::setClock(int clock)
{
    fBasePSG_            = 12.0f * log2(clock * (2.0f / 64 / 440));
    fBaseFM_             = clock * 1.324547661675347e-8f;
    sysInfo_.clock       = clock;
    sysInfo_.actualClock = chip_ ? chip_->setClock(clock) : 0;
    return sysInfo_.actualClock;
}

const char*
YM2203::getStatusString(int ch, char* buf, int n) const
{
    return 0;
}

void
YM2203::setFMOnly(bool f)
{
    fmOnly_ = f;
    if (f)
        chEnabled_ &= ~7;
    else
        chEnabled_ |= 7;
}

} /* namespace sound_sys */

/*
 * End of ym2203.cxx
 */
