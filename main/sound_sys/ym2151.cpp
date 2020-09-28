/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/10/17(Fri) 02:46:35
 */

#include "ym2151.h"
#include <audio/sound_chip.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace sound_sys
{

YM2151::YM2151()
    : chNonMute_(255)
    , chEnabled_(255)
    , chKeyOn_(0)
    , chKeyOnTrigger_(0)
    , currentReg_(0)
    , keyShift_(0)
    , chip_(0)
{
    memset(regCache_, 0, sizeof(regCache_));
    for (auto& inst : chInst_)
        inst = -1;

    sysInfo_.channelCount = 8;
    sysInfo_.systemID     = SoundSystem::SYSTEM_YM2151;

    sysInfo_.clock          = 4000000;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_YM2151;
    sysInfo_.actualClock    = 4000000;
}

const YM2151::SystemInfo&
YM2151::getSystemInfo() const
{
    return sysInfo_;
}

float
YM2151::getNote(int ch, int) const
{
    // OCT=4, NOTE=10 が440Hz
    int noteTbl[] = {
        -48 - 8, // 0: C#
        -48 - 7, // 1: D
        -48 - 6, // 2: D#
        -48 - 5, // 3: E
        -48 - 5, // 4: E
        -48 - 4, // 5: F
        -48 - 3, // 6: F#
        -48 - 2, // 7: G
        -48 - 2, // 8: G
        -48 - 1, // 9: G#
        -48 + 0, // a: A
        -48 + 1, // b: A#
        -48 + 1, // c: A#
        -48 + 2, // d: B
        -48 + 3, // e: C
        -48 + 4, // f: C#
    };

    int kc   = regCache_[ch + 0x28];
    int note = kc & 15;
    int oct  = (kc >> 4) & 7;
    int kf   = regCache_[ch + 0x30] & (63 << 2);
    return noteTbl[note] + oct * 12 + kf * (1.0f / 256) + keyShift_;
}

float
YM2151::getVolume(int ch) const
{
    int tl0 = regCache_[ch + 0x60] & 127;
    int tl2 = regCache_[ch + 0x68] & 127;
    int tl1 = regCache_[ch + 0x70] & 127;
    int tl3 = regCache_[ch + 0x78] & 127;
    return (508 - tl0 - tl1 - tl2 - tl3) * (1.0f / 508); // てぬき
}

float
YM2151::getPan(int ch) const
{
    int v       = regCache_[ch + 0x20];
    float tbl[] = {0 /* disabled */, -1, 1, 0};
    return tbl[v >> 6];
}

int
YM2151::getInstrument(int ch) const
{
    return chInst_[ch];
}

bool
YM2151::mute(int ch, bool f)
{
    if (f)
        chNonMute_ &= ~(1 << ch);
    else
        chNonMute_ |= 1 << ch;
    return true;
}

uint32_t
YM2151::getKeyOnChannels() const
{
    return chKeyOn_;
}

uint32_t
YM2151::getKeyOnTrigger()
{
    //    __disable_irq();
    uint32_t v      = chKeyOnTrigger_;
    chKeyOnTrigger_ = 0;
    //    __enable_irq();
    return v;
}

uint32_t
YM2151::getEnabledChannels() const
{
    return chNonMute_ & chEnabled_;
}

void
YM2151::setValue(int addr, int v)
{
    if (addr == 0)
    {
        currentReg_ = v;
    }
    else
    {
        //        printf("%02x:%02x\n", currentReg_, v);
        regCache_[currentReg_] = v;

        if (currentReg_ == 8)
        {
            // keyon
            int ch    = v & 7;
            int chBit = 1 << ch;
            if (v & (15 << 3))
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
        /*
      else if (currentReg_ >= 0x20 && currentReg_ < 0x28)
        {
          int ch = currentReg_ - 0x20;
          if (v >> 6 == 0)
            chEnabled_ &= ~(1 << ch);
          else
            chEnabled_ |= 1 << ch;
        }
         */
    }

    if (chip_)
    {
        chip_->setValue(addr, v);
    }
}

int
YM2151::getValue(int reg)
{
    return regCache_[reg];
}

int
YM2151::setClock(int clock)
{
    // log(clock/base)/log(2^(1/12))
    //  keyShift_ = logf (clock)*17.31233f-261.2559f;	// base = 3579545
    keyShift_      = logf(clock) * 17.31233f - 261.2581f; // base = 3580000
    sysInfo_.clock = clock;
    sysInfo_.actualClock = chip_ ? chip_->setClock(clock) : 0;
    return sysInfo_.actualClock;
}

const char*
YM2151::getStatusString(int ch, char* buf, int n) const
{
    snprintf(buf,
             n,
             "KC%02X KF%02X CON%d FL%d PMS%d AMS%d",
             regCache_[ch + 0x28],
             regCache_[ch + 0x30] >> 2,
             regCache_[ch + 0x20] & 7,
             (regCache_[ch + 0x20] >> 3) & 7,
             (regCache_[ch + 0x38] >> 4) & 7,
             (regCache_[ch + 0x38]) & 3);
    return buf;
}

// YM2151 ym2151Inst_[1];

} /* namespace sound_sys */
