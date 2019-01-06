/* -*- mode:C++; -*-
 *
 * segapcm_emu.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/17(Sat) 17:55:17
 *
 * $Id$
 */

/*
 * include
 */

#include "segapcm_emu.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/*
 * code
 */

namespace sound_sys
{

void
SegaPCM::initialize()
{
    memset(register_, 0, sizeof(register_));
    memset(voices_, 0, sizeof(voices_));

    romSize_    = 0;
    clock_      = 0;
    sampleRate_ = 0;

    setBankParameter(0, 0);
    setClock(4000000);
    setVolume(1.0f);
    setSampleRate(48000.0f);
    rom_.clear();

    sysInfo_.channelCount   = MAX_VOICE;
    sysInfo_.systemID       = SoundSystem::SYSTEM_SEGA_PCM;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_EMULATION;
    chKeyOn_                = 0;
    chKeyOnTrigger_         = 0;
}

int
SegaPCM::getValue(int addr)
{
    return register_[addr & 0xff];
}

void
SegaPCM::setValue(int addr, int value)
{
    addr &= 0xff;

    register_[addr] = value;

    int ch   = (addr >> 3) & 15;
    int port = addr & 0x87;

    auto& v = voices_[ch];

    if (port == 0x86)
    {
        // mode
        if (value & 1)
        {
            // keyoff
            v.keyon = false;
            chKeyOn_ &= ~(1 << ch);
        }
        else
        {
            // keyon
            chKeyOn_ |= 1 << ch;
            chKeyOnTrigger_ |= 1 << ch;
            v.keyon = true;
            v.pos   = -1;
            v.loop  = !(value & 2);

            int bank = (value & bankMask_) << bankShift_;

            auto& ra = voiceRegs_.a[ch];
            auto& rb = voiceRegs_.b[ch];

            int sampleStart = (rb.startH << 8) | rb.startL;
            int sampleEnd   = (ra.endH << 8) | 0xff;
            int sampleLoop  = (ra.loopH << 8) | ra.loopL;

            v.sample         = (const uint8_t*)rom_.find(bank + sampleStart);
            v.sampleSize     = sampleEnd - sampleStart;
            v.sampleLoopSize = sampleEnd - sampleLoop;

            //          printf ("keyon ch%d: %x(%p)-%x %x, freq%x, loop%d\n",
            //                  ch, sampleStart, v.sample, sampleEnd,
            //                  sampleLoop, ra.freq, v.loop);
        }
    }
    else if (port == 7)
    {
        // 1.7 * 0.16 >> 7 >> 16 << 16
        v.delta = value * freqBase_ >> 7;
    }
}

void
SegaPCM::accumSamples(int32_t* buffer, uint32_t samples)
{
    if (!samples)
        return;

    for (int ch = 0; ch < MAX_VOICE; ++ch)
    //  int ch = 2; for (int ii = 0; ii < 1; ++ii)
    {
        const auto& vr = voiceRegs_.a[ch];
        auto& v        = voices_[ch];

        if (!v.keyon || v.mute || v.delta == 0 || !v.sample)
            continue;

        int delta = v.delta;
        // vol = 8.0
        // 8.8 = .8 * 8.0 >> 8 << 8
        int volL = vr.volL * volume_;
        int volR = vr.volR * volume_;

        //      printf ("ch%d: v %d,%d delta %d\n", ch, volL, volR, delta);

        int32_t* dst             = buffer;
        uint32_t pos             = v.pos;
        int ssize                = v.sampleSize;
        const uint8_t* sampleTop = v.sample;

        bool loop = v.loop;

        int ct = samples;
        do
        {
            pos += delta;
            int ofs = pos >> 16;
            int fr  = pos & 0xffff;
            if (ofs >= ssize)
            {
                if (loop)
                {
                    ofs -= v.sampleLoopSize;
                    pos = (ofs << 16) + fr;
                }
                else
                {
                    v.keyon = false;
                    chKeyOn_ &= ~(1 << ch); // タイミングによってはまずい
                    break;
                }
            }

            int nofs = ofs + 1;
            if (nofs == ssize)
            {
                if (loop)
                    nofs -= v.sampleLoopSize;
                else
                    nofs = ofs;
            }

            int d0 = sampleTop[ofs];
            int d1 = sampleTop[nofs];
            int dd = d1 - d0;
            d0 -= 0x80;

            int d = (dd * fr >> 9) + (d0 << 7);
            int l = d * volL >> 7;
            int r = d * volR >> 7;

            // 9 + 0.16 + 8.8 -16 - 8 + 8
            // 9 + 0.16 -9 + 8.8 -7

            dst[0] += l;
            dst[1] += r;
            dst += 2;
        } while (--ct);
        v.pos = pos;
    }
}

void
SegaPCM::setClock(int clock)
{
    clock_ = clock;
    updateFreqBase();

    sysInfo_.clock       = clock;
    sysInfo_.actualClock = clock;
}

void
SegaPCM::setSampleRate(float freq)
{
    sampleRate_ = freq;
    updateFreqBase();
}

void
SegaPCM::updateFreqBase()
{
    freqBase_ = uint32_t(clock_ * (65536.0f / 256.0f) / sampleRate_);
    //  printf ("clock %f, samplerate %f, freqBase %f\n", clock_, sampleRate_,
    //  freqBase_ / 65536.0f);
}

void
SegaPCM::setVolume(float v)
{
    volume_ = int(v * (1 << 8));
}

void
SegaPCM::setBankParameter(int shift, int mask)
{
    bankShift_ = shift & 0xff;
    baseMask_  = mask ? mask : 0x70;
    bankMask_  = baseMask_;
}

void
SegaPCM::addROMBlock(uint32_t addr,
                     uint32_t romSize,
                     SimpleAutoBuffer&& data,
                     uint32_t size)
{
    rom_.addEntry(addr, std::move(data), size);

    if (romSize != romSize_)
    {
        romSize_ = romSize;

        uint32_t romMask = 1;
        for (; romMask < romSize; romMask <<= 1)
            ;
        --romMask;
        bankMask_ = baseMask_ & (romMask >> bankShift_);

        //      printf ("romSize = %x, bank shift %d, mask %d\n",
        //              romSize, bankShift_, bankMask_);
    }
}

const SegaPCM::SystemInfo&
SegaPCM::getSystemInfo() const
{
    return sysInfo_;
}

float
SegaPCM::getNote(int ch, int) const
{
    uint32_t delta = voices_[ch].delta;
    // log(delta / 2^15)/log(2) * 12
    // (log(delta)*12/log(2) - 15*12)
    return logf(delta) * 17.31234f - 15 * 12;
}

float
SegaPCM::getVolume(int ch) const
{
    auto& vr = voiceRegs_.a[ch];
    return logf(vr.volR + vr.volL) * 0.1602995f; // 1/log(512)
}

float
SegaPCM::getPan(int ch) const
{
    auto& vr = voiceRegs_.a[ch];
    if (vr.volR > vr.volL)
        return 1 - (float)vr.volL / vr.volR;
    else if (vr.volR != 0)
        return (float)vr.volR / vr.volL - 1;
    return 0.0f;
}

int
SegaPCM::getInstrument(int ch) const
{
    (void)ch;
    return -1;
}

bool
SegaPCM::mute(int ch, bool f)
{
    voices_[ch].mute = f;
    return true;
}

uint32_t
SegaPCM::getKeyOnChannels() const
{
    return chKeyOn_;
}

uint32_t
SegaPCM::getKeyOnTrigger()
{
    //  __disable_irq ();
    uint32_t v      = chKeyOnTrigger_;
    chKeyOnTrigger_ = 0;
    //  __enable_irq ();
    return v;
}

uint32_t
SegaPCM::getEnabledChannels() const
{
    uint32_t r = 0;
    uint32_t m = 1;
    for (int i = 0; i < MAX_VOICE; ++i, m <<= 1)
        if (!voices_[i].mute)
            r |= m;
    return r;
}

const char*
SegaPCM::getStatusString(int ch, char* buf, int n) const
{
    auto& v  = voices_[ch];
    auto& ra = voiceRegs_.a[ch];
    auto& rb = voiceRegs_.b[ch];

    int bank = (rb.mode & bankMask_) << bankShift_;
    int top  = bank + ((rb.startH << 8) | rb.startL);

    snprintf(
        buf, n, "P%02X LP%d ADDR+$%06X", ra.freq, v.loop, top + (v.pos >> 16));
    return buf;
}

} /* namespace sound_sys */

/*
 * End of segapcm_emu.cxx
 */
