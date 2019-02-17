/* -*- mode:C++; -*-
 * author(s) : Shuichi TAKANO
 * since 2015/12/17(Thu) 04:23:57
 */

#include "swpcm8.h"
#include <debug.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace sound_sys
{

void
SWPCM8::initialize()
{
    memset(voices_, 0, sizeof(voices_));

    nextKeyOff_ = 0;
    clock_      = 0;
    sampleRate_ = 0;
    setSampleRate(44100.0f);
    setClock(8000000);
    setVolume(1.0f);

    for (auto& v : voices_)
    {
        v.pan    = 3;
        v.volume = 16;
        v.mode   = 4;
        v.delta  = baseDelta_ / 1024;
    }

    keyOnCh_      = 0;
    keyOnTrigger_ = 0;

    pan_      = 3;
    panShare_ = false;

    sysInfo_.channelCount   = 1;
    sysInfo_.systemID       = SoundSystem::SYSTEM_M6258;
    sysInfo_.clock          = 8000000;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_PCM8;
    sysInfo_.actualClock    = 8000000;
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
    //    DBOUT(("ch rate %d:%f %x\n", ch, r, voices_[ch].delta));
}

void
SWPCM8::play(int ch, const void* addr, int len, Type type)
{
    auto& v      = voices_[ch];
    v.nextType   = type;
    v.nextPosEnd = len;
    v.nextSample = static_cast<const uint8_t*>(addr);
    v.pause      = false;
    nextKeyOff_ &= ~(1 << ch);
}

void
SWPCM8::stop(int ch)
{
    nextKeyOff_ |= 1 << ch;
}

void SWPCM8::accumSamples(std::array<int32_t, 2>* buffer, uint32_t samples)
{
    if (!samples)
    {
        return;
    }

    auto nextKeyOff = nextKeyOff_;
    nextKeyOff_     = 0;
    int keyOnCh     = 0;
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

            keyOnTrigger_ |= 1 << ch;
        }
        keyOnCh |= v.keyon ? 1 << ch : 0;
    }
    keyOnCh_ = keyOnCh;
    //    DBOUT(("koc %d %d\n", keyOnCh, nextKeyOff));

    for (int ch = 0; ch < MAX_VOICE; ++ch)
    {
        auto& v = voices_[ch];
        if (!v.keyon || v.pause)
        {
            continue;
        }

        if (v.type == TYPE_PCM8 || v.type == TYPE_PCM16)
        {
            continue;
        }

        auto buf    = buffer;
        auto delta  = v.delta;
        auto prev   = v.prev;
        auto val    = v.val;
        auto frac   = v.frac;
        auto pos    = v.pos;
        auto posEnd = v.posEnd;
        auto sample = v.sample;
        auto pan    = panShare_ ? pan_ : v.pan;
        auto vol    = v.volume * volume_;
        auto volL   = pan & 1 ? vol : 0;
        auto volR   = pan & 2 ? vol : 0;

        //        DBOUT(("v = %d %d %d\n", vol, v.volume, volume_));

        // DBOUT(("accum %d, d %x, p%d, pe %d, vl%d vr%d, pan %d, vol %d, %d\n",
        //        ch,
        //        delta,
        //        pos,
        //        posEnd,
        //        volL,
        //        volR,
        //        pan,
        //        v.volume,
        //        volume_));

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

            (*buf)[0] += volL * d;
            (*buf)[1] += volR * d;
            ++buf;
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
    volume_ = int(v * (1 << (24 - 12 - 4)));
}

void
SWPCM8::updateDelta()
{
    baseDelta_ = clock_ / sampleRate_ * (1 << 24);
    // DBOUT(("clk %f, sample rate %f, base delta %f\n",
    //        clock_,
    //        sampleRate_,
    //        baseDelta_));
}

/////////

float
SWPCM8::getNote(int ch, int) const
{
    return voices_[ch].note;
}

const SoundSystem::SystemInfo&
SWPCM8::getSystemInfo() const
{
    return sysInfo_;
}

float
SWPCM8::getVolume(int ch) const
{
    return logf(voices_[ch].volume) * 0.36067376022224085f; // 1/log(16)
}

float
SWPCM8::getPan(int ch) const
{
    auto p = voices_[ch].pan;
    switch (p & 3)
    {
    case 1:
        return -1.0f;

    case 2:
        return 1.0f;
    }
    return 0.0f;
}

int
SWPCM8::getInstrument(int ch) const
{
    return -1;
}

bool
SWPCM8::mute(int ch, bool f)
{
    voices_[ch].mute = f;
    return true;
}

uint32_t
SWPCM8::getKeyOnChannels() const
{
    return keyOnCh_;
}

uint32_t
SWPCM8::getKeyOnTrigger()
{
    uint32_t v    = keyOnTrigger_;
    keyOnTrigger_ = 0;
    return v;
}

uint32_t
SWPCM8::getEnabledChannels() const
{
    uint32_t r = 0;
    for (int i = 0; i < MAX_VOICE; ++i)
    {
        if (!voices_[i].mute)
        {
            r |= 1 << i;
        }
    }
    return r;
}

const char*
SWPCM8::getStatusString(int ch, char* buf, int n) const
{
    return "";
}

void
SWPCM8::setChMask(bool l, bool r)
{
    panShare_ = true;
    pan_      = (l ? 1 : 0) | (r ? 2 : 0);
    //    DBOUT(("ch mask %d %d, p %d\n", l, r, pan_));
}

void
SWPCM8::pcm8(int ch, const void* addr, int mode, int len)
{
    //    DBOUT(("pcm8 %d:%p mode:%d\n", ch, addr, mode));
    auto& v = voices_[ch];
    if (!v.mute)
    {
        setChRate(ch, 1 / 512.0f);

        if ((mode & 0xff) != 0xff)
        {
            int pan = mode & 3;
            v.pan   = pan;
            if (pan == 0)
            {
                stop(ch);
            }
            else
            {
                sysInfo_.systemID     = SoundSystem::SYSTEM_PCM8;
                sysInfo_.channelCount = 8;
            }
        }

        if ((mode & 0xff0000) != 0xff0000)
        {
            static constexpr int volumeTable[16] = {
                2,
                3,
                4,
                5,
                6,
                8,
                10,
                12,
                16,
                20,
                24,
                32,
                40,
                48,
                64,
                80,
            };

            v.volume = volumeTable[(mode >> 16) & 15];
        }

        if ((mode & 0xff00) != 0xff00)
        {
            v.mode = (mode >> 8) & 7;
            switch (v.mode)
            {
            case MODE_ADPCM3_9K:
                setChRate(ch, 3.9f / 15.6f / 512.0f);
                break;
            case MODE_ADPCM5_2K:
                setChRate(ch, 5.2f / 15.6f / 512.0f);
                break;
            case MODE_ADPCM7_8K:
                setChRate(ch, 7.8f / 15.6f / 512.0f);
                break;
            case MODE_ADPCM10_4K:
                setChRate(ch, 10.4f / 15.6f / 512.0f);
                break;
            default:
                setChRate(ch, 1 / 512.0f);
                break;
            }

            if (v.mode == MODE_PCM16BIT)
            {
                DBOUT(("pcm16!\n"));
            }
            if (v.mode == MODE_PCM8BIT)
            {
                DBOUT(("pcm8!\n"));
            }
        }

        if (addr)
        {
            Type t = TYPE_ADPCM;
            switch (v.mode)
            {
            case MODE_PCM16BIT:
                t = TYPE_PCM16;
                break;
            case MODE_PCM8BIT:
                t = TYPE_PCM8;
                break;
            default:
                break;
            }
            play(ch, addr, len, t);
        }
    }
}

void
SWPCM8::resetPCM8()
{
    for (auto& v : voices_)
    {
        v.volume = 16;
        v.mode   = MODE_ADPCM15_6K;
        v.delta  = baseDelta_ / 1024;
        v.keyon  = false;
    }
}

void
SWPCM8::startTransfer(const uint8_t* p, int length)
{
    //    DBOUT(("start transfer %p: %d\n", p, length));
    setChVolume(0, 16);
    play(0, p, length);
}

void
SWPCM8::stopTransfer()
{
    stop(0);
}

void
SWPCM8::setSampleRate6258(int r)
{
    switch (r)
    {
    case SAMPLERATE_1_1024:
        setChRate(0, 1 / 1024.0f);
        break;
    case SAMPLERATE_1_768:
        setChRate(0, 1 / 768.0f);
        break;
    case SAMPLERATE_1_512:
        setChRate(0, 1 / 512.0f);
        break;
    case SAMPLERATE_INHIBIT:
        break;
    }
}

} /* namespace sound_sys */
