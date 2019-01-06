/* -*- mode:C++; -*-
 *
 * swpcm8.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/12/17(Thu) 04:23:57
 *
 * $Id$
 */

/*
 * include
 */

#include "swpcm8.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/*
 * code
 */

namespace sound_sys
{

void
SWPCM8::initialize()
{
    memset(voices_, 0, sizeof(voices_));

    nextKeyOff_ = 0;
    clock_      = 0;
    sampleRate_ = 0;
    setSampleRate(48000.0f);
    setClock(4000000);
    setVolume(1.0f);

    for (auto& v : voices_)
    {
        v.pan    = 3;
        v.volume = 16;
    }

    pan_      = 3;
    panShare_ = false;
}

void
SWPCM8::setChPan(int ch, int pan)
{
    voices_[ch].pan = pan;
    pan_            = pan;
}

void
SWPCM8::setChVolume(int ch, uint8_t vol)
{
    // 16 = 1.0
    voices_[ch].volume = vol;
}

void
SWPCM8::setChRate(int ch, float r)
{
    voices_[ch].delta = r * baseDelta_;
}

void
SWPCM8::play(int ch, void* addr, int len, Type type)
{
    auto& v      = voices_[ch];
    v.nextType   = type;
    v.nextPosEnd = len;
    v.nextSample = static_cast<const uint8_t*>(addr);
    nextKeyOff_ &= ~(1 << ch);
}

void
SWPCM8::stop(int ch)
{
    nextKeyOff_ |= 1 << ch;
}

void
SWPCM8::accumSamples(int32_t* buffer, uint32_t samples)
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
            v.type   = v.nextType;
            v.keyon  = true;
            v.pos    = -1;
            v.posEnd = v.nextPosEnd;
            v.sample = v.nextSample;
            v.frac   = 0xffffff;
            v.prev   = 0;
            v.val    = 0;
            v.coder.reset();
            v.nextSample = 0;
            //            printf ("ch%d:keyon posEnd:%d %p\n", ch, v.posEnd,
            //            v.sample);
        }
    }

    for (int ch = 0; ch < MAX_VOICE; ++ch)
    {
        auto& v = voices_[ch];
        if (!v.keyon)
            continue;

        if (v.type == TYPE_PCM8 || v.type == TYPE_PCM16)
            continue;

        auto buf    = buffer;
        auto delta  = v.delta;
        auto prev   = v.prev;
        auto val    = v.val;
        auto frac   = v.frac;
        auto pos    = v.pos;
        auto posEnd = v.posEnd;
        auto sample = v.sample;
        auto pan    = panShare_ ? pan_ : v.pan;
        auto vol    = v.volume * volume_ >> 4;
        auto volL   = pan & 1 ? vol : 0;
        auto volR   = pan & 2 ? vol : 0;

        int ct = samples;
        do
        {
            frac += delta;
            while (frac & ~0xffffff)
            {
                frac -= 0x1000000;
                prev = val;
                val  = 0;

                ++pos;
                auto spos = pos >> 1;
                if (spos >= posEnd)
                {
                    v.keyon = false;
                    ct      = 1;
                    break;
                }
                int data = sample[spos];
                int smp  = pos & 1 ? data >> 4 : data & 15;
                val      = v.coder.update(smp);
            }

            int32_t dd = val - prev;
            int32_t d  = (dd * (frac >> 16) >> 8) + prev;

            //            printf("pos %d, d %d, val %d\n", pos, d, val);

            buf[0] += volL * d;
            buf[1] += volR * d;
            buf += 2;
        } while (--ct);

        v.pos  = pos;
        v.frac = frac;
        v.prev = prev;
        v.val  = val;
    }
}

void
SWPCM8::setClock(int clock)
{
    clock_ = clock;
    updateDelta();
}

void
SWPCM8::setSampleRate(float freq)
{
    sampleRate_ = freq;
    updateDelta();
}

void
SWPCM8::setVolume(float v)
{
    volume_ = int(v * (1 << (16 - 2 + 6)));
}

void
SWPCM8::updateDelta()
{
    baseDelta_ = clock_ / sampleRate_ * (1 << 24);
}

} /* namespace sound_sys */

/*
 * End of swpcm8.cxx
 */
