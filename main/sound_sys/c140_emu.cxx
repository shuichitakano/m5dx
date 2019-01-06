/* -*- mode:C++; -*-
 *
 * c140_emu.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/12(Mon) 17:15:35
 *
 * $Id$
 */

/*
 * include
 */

#include "c140_emu.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/*
 * code
 */

namespace sound_sys
{

namespace
{
constexpr int16_t
computeTableEntry(int i)
{
    return i == 0 ? 0 : (computeTableEntry(i - 1) + (16 << i));
}

constexpr int16_t pcmTable_[] = {
    computeTableEntry(0),
    computeTableEntry(1),
    computeTableEntry(2),
    computeTableEntry(3),
    computeTableEntry(4),
    computeTableEntry(5),
    computeTableEntry(6),
    computeTableEntry(7),
};

inline int
decode(int data)
{
    int sdt = data >> 3;
    int m   = data & 7;
    int a   = sdt << m;
    int b   = pcmTable_[m];
    return sdt < 0 ? a - b : a + b;
}
} // namespace

C140::C140()
{
    initialize();
}

void
C140::initialize()
{
    memset(register_, 0, sizeof(register_));
    memset(voices_, 0, sizeof(voices_));
    setClock(49152000);
    setType(TYPE_SYSTEM2);
    setVolume(1.0f);
    setSampleRate(48000.0f);
    rom_.clear();

    sysInfo_.channelCount   = MAX_VOICE;
    sysInfo_.systemID       = SoundSystem::SYSTEM_C140;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_EMULATION;

    nextKeyOff_     = 0;
    chKeyOn_        = 0;
    chKeyOnTrigger_ = 0;
}

int
C140::getValue(int addr)
{
    return register_[addr & 0x1ff];
}

void
C140::setValue(int addr, int value)
{
    addr &= 0x1ff;

    if ((addr >= 0x1f8) && (type_ == TYPE_ASIC219))
        addr -= 8;
    register_[addr] = value;

    if (addr < 0x180)
    {
        int ch   = addr >> 4;
        int port = addr & 15;

        auto& v  = voices_[ch];
        auto& vr = voiceRegs_[ch];

        if (port == 5)
        //      if (port == 5 && ch == 17)
        {
            if (value & 0x80)
            {
                chKeyOn_ |= 1 << ch;
                chKeyOnTrigger_ |= 1 << ch;

                v.mode          = value;
                int sampleStart = (vr.startH << 8) | vr.startL;
                int sampleEnd   = (vr.endH << 8) | vr.endL;
                int sampleLoop  = (vr.loopH << 8) | vr.loopL;

                if (type_ == TYPE_ASIC219)
                {
                    sampleStart <<= 1;
                    sampleEnd <<= 1;
                    sampleLoop <<= 1;
                }
                /*
                              printf ("keyon ch%d: %x-%x %x, freq%x:%x, mode
                   %d(ml%d lp%d rv%d sg%d)\n", ch, sampleStart, sampleEnd,
                   sampleLoop,
                                      ((vr.freqH << 8) | vr.freqL),
                                      ((vr.freqH << 8) | vr.freqL) * freqBase_
                   >> 16, v.mode, v.mode & MODE_MULAW, v.mode & MODE_LOOP,
                                      v.mode & MODE_PHASEREVERSE, v.mode &
                   MODE_SIGNBIT);
                */
                v.nextSampleSize     = sampleEnd - sampleStart;
                v.nextSampleLoopSize = sampleEnd - sampleLoop;
                v.nextSample         = (const int8_t*)rom_.find(
                    convertAddr(sampleStart, vr.bank, ch));
                nextKeyOff_ &= ~(1 << ch);
            }
            else
            {
                chKeyOn_ &= ~(1 << ch);
                nextKeyOff_ |= 1 << ch;
            }
        }

        //      if (port == 3)
        if (port == 2 || port == 3) /* 上位だけ書き換えてる事がある */
        {
            v.freq = ((vr.freqH << 8) | vr.freqL) * freqBase_ >> 16;
            //          printf ("ch:%d freq %x %x: %x\n", ch, vr.freqH,
            //          vr.freqL, v.freq);
        }
    }
}

void
C140::accumSamples(int32_t* buffer, uint32_t samples)
{
    if (!samples)
        return;

    uint32_t nextKeyOff = nextKeyOff_;
    nextKeyOff_         = 0;

    for (int ch = 0; ch < chCount_; ++ch)
    //  int ch = 2; for (int ii = 0; ii < 1; ++ii)
    {
        const auto& vr = voiceRegs_[ch];
        auto& v        = voices_[ch];

        if (v.mute || nextKeyOff & (1 << ch))
        {
            v.keyon      = false;
            v.nextSample = 0;
            continue;
        }

        if (v.nextSample)
        {
            v.keyon          = true;
            v.pos            = -1;
            v.frac           = -1;
            v.val            = 0;
            v.prev           = 0;
            v.sample         = v.nextSample;
            v.sampleSize     = v.nextSampleSize;
            v.sampleLoopSize = v.nextSampleLoopSize;

            v.nextSample = 0;
        }

        if (!v.keyon || v.freq == 0)
            continue;

        int delta = v.freq;
        // vol = 6.2
        // 6.8 = .8 + 6.2 - 8 - 2 + 8
        int volL = vr.volL * volume_ >> 2;
        int volR = vr.volR * volume_ >> 2;

        int32_t* dst            = buffer;
        uint32_t pos            = v.pos;
        int ssize               = v.sampleSize;
        const int8_t* sampleTop = v.sample;
        auto frac               = v.frac;
        auto val                = v.val;
        auto prev               = v.prev;

        bool loop     = v.mode & MODE_LOOP;
        bool signBit  = v.mode & MODE_SIGNBIT;
        bool phaseRev = v.mode & MODE_PHASEREVERSE;

        //      printf ("%d:%x: pos %d, d %d, v(%d %d), size %d, loop %d,
        //      signBit %d, phaseRev %d\n",
        //              ch, (vr.startH << 8) | vr.startL,
        //              pos >> 16, delta, volL, volR, ssize, loop, signBit,
        //              phaseRev);

        int ct = samples;

        if ((v.mode & MODE_MULAW) && type_ != TYPE_ASIC219)
        {
            do
            {
                frac += delta;
                while (frac & ~0xffff)
                {
                    frac -= 0x10000;
                    pos += 1;
                    prev = val;
                    if (pos >= ssize)
                    {
                        if (loop)
                            pos -= v.sampleLoopSize;
                        else
                        {
                            v.keyon = false;
                            val     = 0;
                            break;
                        }
                    }
                    int smp = sampleTop[pos];
                    val     = decode(smp);
                }

                int d = ((val - prev) * frac >> 12) + (prev << 4);
                int l = d * volL >> 9;
                int r = d * volR >> 9;
                // 14 + .16 + 6.8 - 16 - 5 - 8 + 8
                // 14 + .16 - 12 + 6.8 - 9

                dst[0] += l;
                dst[1] += r;
                dst += 2;
            } while (--ct);
        }
        else
        {
            do
            {
                frac += delta;
                while (frac & ~0xffff)
                {
                    frac -= 0x10000;
                    pos += 1;
                    prev = val;
                    if (pos >= ssize)
                    {
                        if (loop)
                            pos -= v.sampleLoopSize;
                        else
                        {
                            v.keyon = false;
                            val     = 0;
                            break;
                        }
                    }
                    if (type_ == TYPE_ASIC219)
                    {
                        val = sampleTop[pos ^ 1];
                        if (signBit)
                        {
                            if (val < 0)
                                val = -(val & 0x7f);
                        }
                        if (phaseRev)
                            val = -val;
                    }
                    else
                        val = sampleTop[pos];
                }

                int d = ((val - prev) * frac >> 7) + (prev << 9);
                int l = d * volL >> 9;
                int r = d * volR >> 9;

                // 9 + .16 + 6.8 -16 -8 + 8
                // 9 + .16 - 7 + 6.8 -9

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

uint32_t
C140::convertAddr(int addr, int bank, int ch) const
{
    addr |= bank << 16;
    switch (type_)
    {
    case TYPE_SYSTEM2:
        return ((addr & 0x200000) >> 2) | (addr & 0x7ffff);

    case TYPE_SYSTEM21_A:
        return ((addr & 0x300000) >> 1) | (addr & 0x7ffff);

    case TYPE_SYSTEM21_B:
    {
        addr = ((addr & 0x200000) >> 2) | (addr & 0x7ffff);
        if (bank & 0x04)
            addr += 0x80000;
        if (bank & 0x20)
            addr += 0x100000;
        return addr;
    }

    case TYPE_ASIC219:
    {
        static constexpr int16_t bankReg[4] = {0x1f7, 0x1f1, 0x1f3, 0x1f5};
        return ((register_[bankReg[ch >> 2]] & 3) * 0x20000) + addr;
    }
    };
    return 0;
}

void
C140::setClock(int clock)
{
    clock_ = clock / 96.0f / 24.0f;
    updateFreqBase();
}

void
C140::setSampleRate(float freq)
{
    sampleRate_ = freq;
    updateFreqBase();
}

void
C140::updateFreqBase()
{
    freqBase_ = uint32_t(clock_ * 2 * 65536 / sampleRate_);
    //  printf ("clock %f, samplerate %f, freqBase %d\n", clock_, sampleRate_,
    //  freqBase_);
}

void
C140::setType(Type t)
{
    type_    = t;
    chCount_ = t == TYPE_ASIC219 ? 16 : 24;
}

void
C140::setVolume(float v)
{
    volume_ = int(v * (1 << 8));
}

void
C140::addROMBlock(uint32_t addr, SimpleAutoBuffer&& data, uint32_t size)
{
    rom_.addEntry(addr, std::move(data), size);
}

const C140::SystemInfo&
C140::getSystemInfo() const
{
    return sysInfo_;
}

float
C140::getNote(int ch, int) const
{
    uint32_t delta = voices_[ch].freq;
    // log(delta / 2^15)/log(2) * 12
    // (log(delta)*12/log(2) - 15*12)
    return logf(delta) * 17.31234f - 15 * 12;
}

float
C140::getVolume(int ch) const
{
    auto& vr = voiceRegs_[ch];
    return logf(vr.volR + vr.volL) * 0.1602995f; // 1/log(512)
}

float
C140::getPan(int ch) const
{
    auto& vr = voiceRegs_[ch];
    if (vr.volR > vr.volL)
        return 1 - (float)vr.volL / vr.volR;
    else if (vr.volR != 0)
        return (float)vr.volR / vr.volL - 1;
    return 0.0f;
}

int
C140::getInstrument(int ch) const
{
    (void)ch;
    return -1;
}

bool
C140::mute(int ch, bool f)
{
    voices_[ch].mute = f;
    return true;
}

uint32_t
C140::getKeyOnChannels() const
{
    return chKeyOn_;
}

uint32_t
C140::getKeyOnTrigger()
{
    //  __disable_irq ();
    uint32_t v      = chKeyOnTrigger_;
    chKeyOnTrigger_ = 0;
    //  __enable_irq ();
    return v;
}

uint32_t
C140::getEnabledChannels() const
{
    uint32_t r = 0;
    uint32_t m = 1;
    for (int i = 0; i < MAX_VOICE; ++i, m <<= 1)
        if (!voices_[i].mute)
            r |= m;
    return r;
}

const char*
C140::getStatusString(int ch, char* buf, int n) const
{
    auto& v = voices_[ch];
    snprintf(buf,
             n,
             "%s LP%d",
             v.mode & MODE_MULAW ? "MULAW" : " PCM ",
             v.mode & MODE_LOOP ? 1 : 0);
    return buf;
}

} /* namespace sound_sys */

/*
 * End of c140_emu.cxx
 */
