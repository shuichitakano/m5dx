/* -*- mode:C++; -*-
 *
 * m6295_emu.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/31(Sat) 12:30:10
 *
 * $Id$
 */

/*
 * include
 */

#include "m6295_emu.h"
#include <stdio.h>

#include <math.h>
#include <string.h>

/*
 * code
 */

namespace sound_sys
{

void
M6295::initialize()
{
    memset(voices_, 0, sizeof(voices_));

    val_        = 0;
    prev_       = 0;
    delta_      = 0;
    frac_       = 0;
    phrase_     = -1;
    bankOffset_ = 0;
    nextKeyOff_ = 0;
    pin7_       = false;
    clock_      = 0;
    sampleRate_ = 0;
    setSampleRate(48000.0f);
    setClock(2013750);
    setVolume(1.0f);
    rom_.clear();

    sysInfo_.channelCount   = MAX_VOICE;
    sysInfo_.systemID       = SoundSystem::SYSTEM_M6295;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_EMULATION;
}

void
M6295::setValue(int value)
{
    if (phrase_ < 0)
    {
        if (value & 0x80)
        {
            phrase_ = value & 0x7f;
        }
        else
        {
            nextKeyOff_ |= value >> 3;
        }
    }
    else
    {
        uint32_t chMask = value;
        int ch          = 27 - __builtin_clz(chMask);
        int att         = value & 15;
        //      printf ("%d chmask %d ch %d, att %d\n", value, chMask >> 4, ch,
        //      att);

        static constexpr int volume_table[16] = {
            0x20, //   0 dB
            0x16, //  -3.2 dB
            0x10, //  -6.0 dB
            0x0b, //  -9.2 dB
            0x08, // -12.0 dB
            0x06, // -14.5 dB
            0x04, // -18.0 dB
            0x03, // -20.5 dB
            0x02, // -24.0 dB
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
        };

        int addrTable = phrase_ << 3;
        auto addrData = rom_.find(addrTable + bankOffset_);
        if (addrData)
        {
            uint32_t start = ((addrData[0] << 16) | (addrData[1] << 8) |
                              (addrData[2] << 0)) &
                             0x3ffff;
            uint32_t stop = ((addrData[3] << 16) | (addrData[4] << 8) |
                             (addrData[5] << 0)) &
                            0x3ffff;
            if (start < stop)
            {
                auto p = rom_.find(start + bankOffset_);
                if (p)
                {
                    auto& v      = voices_[ch];
                    v.volume     = volume_table[att];
                    v.nextPosEnd = stop - start;
                    v.nextSample = p;
                    nextKeyOff_ &= ~(1 << ch);
                }
            }
        }

        phrase_ = -1;
    }
}

void
M6295::accumSamples(int32_t* buffer, uint32_t samples)
{
    if (!samples)
        return;

    auto nextKeyOff = nextKeyOff_;
    nextKeyOff_     = 0;
    for (int ch = 0; ch < MAX_VOICE; ++ch)
    {
        auto& v = voices_[ch];
        if (v.mute || nextKeyOff & (1 << ch))
        {
            v.keyon      = false;
            v.nextSample = 0;
            continue;
        }
        if (v.nextSample)
        {
            v.keyon  = true;
            v.pos    = -1;
            v.posEnd = v.nextPosEnd;
            v.sample = v.nextSample;
            v.coder.reset();
            v.nextSample = 0;
            //          printf ("ch%d:keyon posEnd:%d %p\n", ch, v.posEnd,
            //          v.sample);
        }
    }

    int32_t prev   = prev_;
    int32_t val    = val_;
    int32_t frac   = frac_;
    uint32_t delta = delta_;

    do
    {
        frac += delta;
        while (frac & ~0xffffff)
        {
            frac -= 0x1000000;
            prev = val;
            val  = 0;
            for (int ch = 0; ch < MAX_VOICE; ++ch)
            {
                auto& v = voices_[ch];
                if (!v.keyon)
                    continue;

                ++v.pos;
                auto spos = v.pos >> 1;
                if (spos >= v.posEnd)
                {
                    v.keyon = false;
                    continue;
                }
                int data = v.sample[spos];
                int smp  = v.pos & 1 ? data & 15 : data >> 4;
                val += v.coder.update(smp) * v.volume;
            }
        }

#if 0
      int32_t vv = volume_ * val;
#else
        int32_t dd = val - prev;
        int32_t d  = (dd * (frac >> 16) >> 8) + prev;
        int32_t vv = d * volume_;
#endif
        buffer[0] += vv;
        buffer[1] += vv;
        buffer += 2;
    } while (--samples);

    frac_ = frac;
    prev_ = prev;
    val_  = val;
}

int
M6295::getValue()
{
    return 0; // todo
}

void
M6295::setClock(int clock)
{
    clock_ = clock;
    updateDelta();
}

void
M6295::setSampleRate(float freq)
{
    sampleRate_ = freq;
    updateDelta();
}

void
M6295::setVolume(float v)
{
    volume_ = int(v * (1 << (8 - 3)));
}

void
M6295::setPin7(bool v)
{
    pin7_ = v;
    updateDelta();
}

void
M6295::updateDelta()
{
    delta_ = (uint32_t)(clock_ / sampleRate_ / (pin7_ ? 132 : 165) * (1 << 24) +
                        0.5f);
    //  printf ("clk %f, rate %f, pin7 %d, delta = %d\n", clock_, sampleRate_,
    //  pin7_, delta_);
}

void
M6295::setBankOffset(uint32_t v)
{
    bankOffset_ = v;
}

void
M6295::addROMBlock(uint32_t addr, SimpleAutoBuffer&& data, uint32_t size)
{
    rom_.addEntry(addr, std::move(data), size);
}

const M6295::SystemInfo&
M6295::getSystemInfo() const
{
    return sysInfo_;
}

float
M6295::getNote(int ch, int) const
{
    return 0;
}

float
M6295::getVolume(int ch) const
{
    return 1.0f;
}

float
M6295::getPan(int ch) const
{
    return 0.0f;
}

int
M6295::getInstrument(int ch) const
{
    return -1;
}

bool
M6295::mute(int ch, bool f)
{
    return false;
}

uint32_t
M6295::getKeyOnChannels() const
{
    return 0;
}

uint32_t
M6295::getKeyOnTrigger()
{
    return 0;
}

uint32_t
M6295::getEnabledChannels() const
{
    return 0;
}

const char*
M6295::getStatusString(int ch, char* buf, int n) const
{
    return 0;
}

} /* namespace sound_sys */

/*
 * End of m6295_emu.cxx
 */
