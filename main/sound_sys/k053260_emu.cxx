/* -*- mode:C++; -*-
 *
 * k053260_emu.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/25(Sun) 00:23:46
 *
 * $Id$
 */

/*
 * include
 */

#include "k053260_emu.h"
#include <stdio.h>

#include <algorithm>
#include <math.h>
#include <string.h>
//#include <assert.h>

/*
 * code
 */

namespace sound_sys
{

void
K053260::initialize()
{
    memset(register_, 0, sizeof(register_));
    memset(voices_, 0, sizeof(voices_));

    clock_      = 0;
    sampleRate_ = 0;

    nextKeyOff_ = 0;

    setSampleRate(48000.0f);
    setClock(3579545);
    setVolume(1.0f);
    rom_.clear();

    sysInfo_.channelCount   = MAX_VOICE;
    sysInfo_.systemID       = SoundSystem::SYSTEM_053260;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_EMULATION;

    chKeyOn_        = 0;
    chKeyOnTrigger_ = 0;
}

int
K053260::getValue(int addr)
{
    return register_[addr];
}

void
K053260::setValue(int addr, int value)
{
    if (addr >= (int)sizeof(register_))
        return;

    switch (addr)
    {
    case REG_CH_ENABLE:
    {
        int keyOn = (register_[REG_CH_ENABLE] ^ value) & value;
        chKeyOnTrigger_ |= keyOn;
        chKeyOn_    = value;
        nextKeyOff_ = ~value;
        for (int i = 0; i < MAX_VOICE; ++i)
        {
            if (!(keyOn & (1 << i)))
                continue;

            auto& v  = voices_[i];
            auto& vr = voiceRegs_.ch[i];

            int size  = (vr.sizeH << 8) | vr.sizeL;
            int start = (vr.bank << 16) | (vr.startH << 8) | vr.startL;

            v.nextReverse = value & (1 << (4 + i));
            v.nextPosEnd  = size;
            //            printf ("start %x, size %x\n", start, size);
            int adj      = v.nextReverse ? size : 0;
            v.nextSample = rom_.find(start - adj);
            if (v.nextSample && v.nextReverse)
                v.nextSample += adj - 1;
        }
    }
    break;

    case REG_PAN01:
        voices_[0].pan = value & 7;
        voices_[1].pan = (value >> 3) & 7;
        break;

    case REG_PAN23:
        voices_[2].pan = value & 7;
        voices_[3].pan = (value >> 3) & 7;
        break;
    }

    register_[addr] = value;
}

void
K053260::accumSamples(int32_t* buffer, uint32_t samples)
{
    if (!samples || !(register_[REG_CONTROL] & 2))
        return;

    static constexpr int32_t dpcmcnv[] = {
        0, 1, 2, 4, 8, 16, 32, 64, -128, -64, -32, -16, -8, -4, -2, -1};

    auto nextKeyOff = nextKeyOff_;
    nextKeyOff_     = 0;

    for (int ch = 0; ch < MAX_VOICE; ++ch)
    {
        auto& vr = voiceRegs_.ch[ch];
        auto& v  = voices_[ch];

        if (v.mute || nextKeyOff & (1 << ch))
        {
            v.keyon      = false;
            v.nextSample = 0;
            continue;
        }

        if (v.nextSample)
        {
            v.keyon      = true;
            v.pos        = -1;
            v.frac       = -1;
            v.val        = 0;
            v.prev       = 0;
            v.loop       = register_[REG_LOOP_DPCM] & (1 << ch);
            v.dpcm       = register_[REG_LOOP_DPCM] & (1 << (ch + 4));
            v.reverse    = v.nextReverse;
            v.posEnd     = v.nextPosEnd;
            v.sample     = v.nextSample;
            v.nextSample = 0;
            if (v.dpcm)
                v.posEnd <<= 1;

            //          if (!v.reverse)
            //            v.keyon = false;

            //          printf ("keyon %d: loop %d, dpcm %d, reverse %d, end
            //          %d\n", ch, v.loop, v.dpcm, v.reverse, v.posEnd);
        }

        if (!v.keyon)
            continue;

        int vol        = vr.volume;                   // 7.0
        uint32_t volL  = volume_ * vol * v.pan >> 10; // .16*7.0*1.2
        uint32_t volR  = volume_ * vol * (8 - v.pan) >> 10;
        uint32_t delta = deltaTable_[vr.rateL | (vr.rateH << 8)];
        //      printf ("vol %d (%d %d) delta %d\n", vol, volL, volR, delta);

        int32_t* dst       = buffer;
        auto pos           = v.pos;
        auto frac          = v.frac;
        auto posEnd        = v.posEnd;
        auto val           = v.val;
        auto prev          = v.prev;
        const auto* sample = v.sample;
        int ct             = samples;

        if (v.dpcm)
        {
            do
            {
                frac += delta;
                while (frac & ~0xffffff)
                {
                    frac -= 0x1000000;
                    pos += 1;
                    prev = val;

                    if (pos >= posEnd)
                    {
                        if (v.loop)
                            pos = 0;
                        else
                        {
                            v.keyon = false;
                            val     = 0;
                            break;
                        }
                    }
                    int smp = sample[pos >> 1];
                    int s   = pos & 1 ? smp >> 4 : smp & 15;
                    val     = ((val * 62) >> 6) + dpcmcnv[s];
                    val = std::min((int32_t)127, std::max((int32_t)-128, val));
                }

                int32_t d  = val - prev;
                int32_t iv = d * (frac >> 16) + (prev << 8);
                int32_t l  = iv * volL;
                int32_t r  = iv * volR;

                dst[0] += l;
                dst[1] += r;
                dst += 2;
            } while (--ct);
        }
        else
        {
            int posScale = v.reverse ? -1 : 1;
            do
            {
                frac += delta;
                while (frac & ~0xffffff)
                {
                    frac -= 0x1000000;
                    pos += 1;
                    prev = val;

                    if (pos >= posEnd)
                    {
                        if (v.loop)
                            pos = 0;
                        else
                        {
                            v.keyon = false;
                            val     = 0;
                            break;
                        }
                    }
                    val = (int8_t)sample[pos * posScale];
                }

                int32_t d  = val - prev;
                int32_t iv = d * (frac >> 16) + (prev << 8);
                int32_t l  = iv * volL;
                int32_t r  = iv * volR;

                dst[0] += l;
                dst[1] += r;
                dst += 2;
            } while (--ct);
        }

        v.pos  = pos;
        v.frac = frac;
        v.val  = val;
        v.prev = prev;
    }
}

void
K053260::setClock(int clock)
{
    //  printf ("%p: setClock %d\n", this, clock);
    clock_ = clock;
    updateDeltaTable();
}

void
K053260::setSampleRate(float freq)
{
    //  printf ("%p: setSampleRate %f\n", this, freq);
    sampleRate_ = freq;
    updateDeltaTable();
}

void
K053260::updateDeltaTable()
{
    if (sampleRate_ <= 0 || clock_ <= 0)
        return;

    float base = (1 << 24) * clock_ / sampleRate_;
    for (int i = 0; i < 0x1000; ++i)
    {
        int32_t v      = int32_t(base / (0x1000 - i));
        deltaTable_[i] = v == 0 ? 1 : v;
        //      printf ("dlt%d: %d\n", i, v);
    }
}

void
K053260::setVolume(float v)
{
    volume_ = int(v * (1 << 8));
}

void
K053260::addROMBlock(uint32_t addr, SimpleAutoBuffer&& data, uint32_t size)
{
    rom_.addEntry(addr, std::move(data), size);
}

const K053260::SystemInfo&
K053260::getSystemInfo() const
{
    return sysInfo_;
}

float
K053260::getNote(int ch, int) const
{
    auto& vr       = voiceRegs_.ch[ch];
    uint32_t delta = deltaTable_[vr.rateL | (vr.rateH << 8)];
    // log(delta / 2^24)/log(2) * 12
    // (log(delta)*12/log(2) - 24*12)
    return logf(delta) * 17.31234f - 24 * 12;
}

float
K053260::getVolume(int ch) const
{
    auto& vr = voiceRegs_.ch[ch];
    //  return vr.volume * (1.0f / 127.0f);
    return logf(vr.volume) * 0.206433f; // /log(127)
}

float
K053260::getPan(int ch) const
{
    return (4 - voices_[ch].pan) * (1 / 4.0f);
}

int
K053260::getInstrument(int ch) const
{
    return -1;
}

bool
K053260::mute(int ch, bool f)
{
    voices_[ch].mute = f;
    return true;
}

uint32_t
K053260::getKeyOnChannels() const
{
    return chKeyOn_;
}

uint32_t
K053260::getKeyOnTrigger()
{
    //  __disable_irq ();
    uint32_t v      = chKeyOnTrigger_;
    chKeyOnTrigger_ = 0;
    //  __enable_irq ();
    return v;
}

uint32_t
K053260::getEnabledChannels() const
{
    uint32_t r = 0;
    uint32_t m = 1;
    for (int i = 0; i < MAX_VOICE; ++i, m <<= 1)
        if (!voices_[i].mute)
            r |= m;
    return r;
}

const char*
K053260::getStatusString(int ch, char* buf, int n) const
{
    auto& v = voices_[ch];
    snprintf(buf, n, "%cPCM LP%d REV%d", v.dpcm ? 'D' : ' ', v.loop, v.reverse);
    return buf;
}

} /* namespace sound_sys */

/*
 * End of k053260_emu.cxx
 */
