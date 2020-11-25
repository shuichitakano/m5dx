/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2015/05/22(Fri) 01:43:34
 */

#include "psg_common.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace sound_sys
{

PSGState::PSGState()
    : chNonMute_(65535)
    , chEnabled_(65535)
    , chKeyOn_(0)
    , chKeyOnTrigger_(0)
    , fBase_(0.1059638129340278f)
{
}

float
PSGState::getNote(int ch, const uint8_t* regCache) const
{
    // PSG
    int rl = regCache[0 + (ch << 1)];
    int rh = regCache[1 + (ch << 1)];
    int tp = ((rh << 8) | rl) & 4095;
    if (tp == 0)
        return -1000; // 適当。keyoffとして扱う？
    // f = clock*2 / (64 * tp)
    // key = 12 log_2(f/440)
    //     = 12 log_2(clock*2/64/440) - 12 log_2(tp)
    return fBase_ - 12.0f * log2(tp);
}

float
PSGState::getVolume(int ch, const uint8_t* regCache) const
{
    return (regCache[0x8 + ch] & 15) * (1.0f / 15);
}

bool
PSGState::mute(int ch, bool f)
{
    if (f)
        chNonMute_ &= ~(1 << ch);
    else
        chNonMute_ |= 1 << ch;
    return true;
}

uint32_t
PSGState::getKeyOnTrigger()
{
    //  __disable_irq ();
    uint32_t v      = chKeyOnTrigger_;
    chKeyOnTrigger_ = 0;
    //  __enable_irq ();
    return v;
}

uint32_t
PSGState::getEnabledChannels() const
{
    return chNonMute_ & chEnabled_;
}

bool
PSGState::update(int reg, int& v, const uint8_t* regCache)
{
    if (reg == 0x7)
    {
        // psg keyon
        int ko = (v | (v >> 3)) & 7;
        int t  = (~chKeyOn_) & ko;
        chKeyOn_ |= ko;
        chKeyOnTrigger_ |= t;

        int mask = chNonMute_ & 7;
        v &= (mask << 3) | mask;
    }
    else if (reg >= 0x8 && reg <= 0xa)
    {
        // keyonとして音量変化も見てみる
        int ch    = reg - 0x8;
        int chBit = 1 << ch;
        if (chKeyOn_ & chBit)
        {
            int plv = regCache[reg] & 15;
            int lv  = v & 15;
            if (lv > plv)
                chKeyOnTrigger_ |= chBit;
        }

        if (!(chNonMute_ & chBit))
            v &= ~15;
    }
    return true;
}

void
PSGState::setClock(int clock)
{
    fBase_ = 12.0f * log2(clock * (2.0f / 64 / 440));
}

} /* namespace sound_sys */
