/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/12/16(Tue) 04:44:16
 *
 */

#include "y8950.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace sound_sys
{

namespace
{
static const uint8_t regSlotChMap_[][2] = {
    // ch, slot
    {0, 0}, //+0x00
    {1, 0}, //+0x01
    {2, 0}, //+0x02
    {0, 1}, //+0x03
    {1, 1}, //+0x04
    {2, 1}, //+0x05
    {0, 0}, //+0x06	invalid
    {0, 0}, //+0x07 invalid
    {3, 0}, //+0x08
    {4, 0}, //+0x09
    {5, 0}, //+0x0a
    {3, 1}, //+0x0b
    {4, 1}, //+0x0c
    {5, 1}, //+0x0d
    {0, 0}, //+0x0e	invalid
    {0, 0}, //+0x0f invalid
    {6, 0}, //+0x10
    {7, 0}, //+0x11
    {8, 0}, //+0x12
    {6, 1}, //+0x13
    {7, 1}, //+0x14
    {8, 1}, //+0x15
};

static const uint8_t chRegMap_[] = {
    // slot0のみ。slot1は+3
    0,
    1,
    2,
    8,
    9,
    10,
    16,
    17,
    18,
};
}; // namespace

Y8950::Y8950()
    : chNonMute_(65535)
    , chEnabled_(65535)
    , chKeyOn_(0)
    , chKeyOnTrigger_(0)
    , currentReg_(0)
    , fBase_(0.1059638129340278f)
    , chip_(0)
{
    memset(regCache_, 0, sizeof(regCache_));
    for (auto& inst : chInst_)
        inst = -1;
    for (auto& c : cnvFNumber_)
        c = 0;
    fnCnv_ = 0;

    sysInfo_.channelCount       = 9 + 5 + 1;
    sysInfo_.systemID           = SoundSystem::SYSTEM_Y8950;
    sysInfo_.oneShotChannelMask = 31 << 9;

    sysInfo_.clock          = 4000000;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_Y8950;
    sysInfo_.actualClock    = 4000000;
}

const Y8950::SystemInfo&
Y8950::getSystemInfo() const
{
    return sysInfo_;
}

float
Y8950::getNote(int ch, int) const
{
    if (ch >= 9)
        return 0;

    int rl  = regCache_[0xa0 + ch];
    int rh  = regCache_[0xb0 + ch];
    int fn  = ((rh << 8) | rl) & 1023;
    int blk = ((rh >> 2) & 7) - 1;

    // fn = f * 2^19/(clock / 72)/2^blk
    // f = fn * clock/72 * 2^blk / 2^19
    // key = 12 log_2(f/440)
    float f = fBase_ * (fn << blk);
    return 17.31234f * log(f) - 105.3763f;
}

float
Y8950::getVolume(int ch) const
{
    if (ch < 9)
    {
        int tl0 = regCache_[chRegMap_[ch] + 0x40] & 63;
        return (63 - tl0) * (1.0f / 63);
    }
    return 1.0f;
}

float
Y8950::getPan(int ch) const
{
    return 0.0f;
}

int
Y8950::getInstrument(int ch) const
{
    return chInst_[ch];
}

bool
Y8950::mute(int ch, bool f)
{
    if (f)
        chNonMute_ &= ~(1 << ch);
    else
        chNonMute_ |= 1 << ch;
    return true;
}

uint32_t
Y8950::getKeyOnChannels() const
{
    return chKeyOn_;
}

uint32_t
Y8950::getKeyOnTrigger()
{
    //  __disable_irq ();
    uint32_t v      = chKeyOnTrigger_;
    chKeyOnTrigger_ = 0;
    //  __enable_irq ();
    return v;
}

uint32_t
Y8950::getEnabledChannels() const
{
    return chNonMute_ & chEnabled_;
}

void
Y8950::setValue(int addr, int v)
{
    if (addr == 0)
        currentReg_ = v;
    else
    {
        regCache_[currentReg_] = v;
        if (currentReg_ >= 0xb0 && currentReg_ <= 0xb8)
        {
            int ch    = currentReg_ - 0xb0;
            int chBit = 1 << ch;
            if (v & 32)
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

            if (fnCnv_)
            {
                int fn =
                    ((regCache_[0xa0 + ch] | (v << 8)) & 1023) * fnCnv_ >> 16;
                v = (v & ~0x3) | ((fn >> 8) & 3);
                if (chip_ && (fn & 255) != cnvFNumber_[ch])
                {
                    chip_->setValue(1, v);
                    chip_->setValue(0, 0xa0 + ch);
                    v = fn;
                }
                cnvFNumber_[ch] = fn;
            }
        }
        else if (currentReg_ >= 0xa0 && currentReg_ <= 0xa8)
        {
            int ch = currentReg_ & 15;
            if (fnCnv_ && chNonMute_ & (1 << ch))
            {
                int rh = regCache_[0xb0 + ch];
                int fn = (((rh << 8) | v) & 1023) * fnCnv_ >> 16;
                v      = fn & 255;
                if (chip_ && (fn & 768) != (cnvFNumber_[ch] & 768))
                {
                    chip_->setValue(1, v);
                    chip_->setValue(0, 0xb0 + ch);
                    v = (rh & ~03) | ((fn >> 8) & 3);
                }
                cnvFNumber_[ch] = fn;
            }
        }
        else if (currentReg_ == 0xbd)
        {
            for (int i = 0; i < 5; ++i)
            {
                int mask = 1 << i;
                if (v & mask)
                {
                    int chBit = mask << 9;
                    if (chNonMute_ & chBit)
                    {
                        chKeyOn_ |= chBit;
                        chKeyOnTrigger_ |= chBit;
                    }
                    else
                        v &= ~mask;
                }
            }
        }
    }

    if (chip_)
        chip_->setValue(addr, v);
}

int
Y8950::getValue(int addr)
{
    return chip_ ? chip_->getValue(addr) : regCache_[currentReg_];
}

int
Y8950::setClock(int clock)
{
    fBase_               = clock * 2.649095323350694e-8f;
    sysInfo_.clock       = clock;
    sysInfo_.actualClock = chip_ ? chip_->setClock(clock) : 0;

    int d = sysInfo_.clock - sysInfo_.actualClock;
    if (d < 0)
        d = -d;
    if (d > (sysInfo_.clock >> 7))
    {
        fnCnv_ = int((float)sysInfo_.clock * (1 << 16) /
                     (float)sysInfo_.actualClock);
    }
    else
        fnCnv_ = 0;

    return sysInfo_.actualClock;
}

const char*
Y8950::getStatusString(int ch, char* buf, int n) const
{
    return 0;
    //  snprintf (buf, n, "KC%02X KF%02X CON%d FL%d PMS%d AMS%d",
    //            regCache_[ch + 0x28],
    //            regCache_[ch + 0x30] >> 2,
    //            regCache_[ch + 0x20] & 7,
    //            (regCache_[ch + 0x20] >> 3) & 7,
    //            (regCache_[ch + 0x38] >> 4) & 7,
    //            (regCache_[ch + 0x38]) & 3
    //            );
    //  return buf;
}

} /* namespace sound_sys */
