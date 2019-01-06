/* -*- mode:C++; -*-
 *
 * segapcm_emu.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/17(Sat) 17:37:14
 *
 * $Id$
 */
#ifndef _VGM_SEGAPCM_EMU_H
#define _VGM_SEGAPCM_EMU_H

/**
 * @file
 * @brief
 */

/*
 * include
 */

#include "../audio_system.h"
#include "../data_block.h"
#include "../sample_gen.h"
#include <stdint.h>

/*
 * class
 */

namespace sound_sys
{

class SegaPCM : public SoundSystem, public SampleGenerator
{
    static constexpr int MAX_VOICE = 16;

    struct VoiceRegisterA
    {
        uint8_t reserved[2];
        uint8_t volL;  // +02
        uint8_t volR;  // +03
        uint8_t loopL; // +04
        uint8_t loopH; // +05
        uint8_t endH;  // +06
        uint8_t freq;  // +07
    };

    struct VoiceRegisterB
    {
        uint8_t reserved1[4];
        uint8_t startL;    // +84
        uint8_t startH;    // +85
        uint8_t mode;      // +86
        uint8_t reserved2; // +87
    };

    struct VoiceRegisters
    {
        VoiceRegisterA a[MAX_VOICE];
        VoiceRegisterB b[MAX_VOICE];
    };

    struct Voice
    {
        bool keyon;
        bool mute;
        bool loop;
        uint16_t sampleSize;
        uint16_t sampleLoopSize;
        uint32_t delta;
        uint32_t pos;
        const uint8_t* sample;
    };

    union {
        VoiceRegisters voiceRegs_;
        uint8_t register_[256];
    };

    Voice voices_[MAX_VOICE];

    uint32_t romSize_;
    uint32_t baseMask_;
    int bankMask_;
    int bankShift_;

    uint32_t freqBase_;
    float clock_;
    float sampleRate_;
    int volume_; // .8

    DataBlockContainer rom_;
    SystemInfo sysInfo_;

    uint32_t chKeyOn_;
    uint32_t chKeyOnTrigger_;

public:
    SegaPCM() { initialize(); }
    void initialize();

    // SoundSystem
    virtual const SystemInfo& getSystemInfo() const;
    virtual float getNote(int ch, int) const;
    virtual float getVolume(int ch) const;
    virtual float getPan(int ch) const;
    virtual int getInstrument(int ch) const;
    virtual bool mute(int ch, bool f);
    virtual uint32_t getKeyOnChannels() const;
    virtual uint32_t getKeyOnTrigger();
    virtual uint32_t getEnabledChannels() const;
    virtual const char* getStatusString(int ch, char* buf, int n) const;

    // SampleGenerator
    virtual void accumSamples(int32_t* buffer, uint32_t samples);
    virtual void setSampleRate(float rate);

    void setValue(int addr, int v);
    int getValue(int addr);
    void setClock(int clock);
    void setVolume(float v);
    void setBankParameter(int shift, int mask);

    void addROMBlock(uint32_t addr,
                     uint32_t romSize,
                     SimpleAutoBuffer&& data,
                     uint32_t size);

protected:
    void updateFreqBase();
};

} /* namespace sound_sys */

#endif /* _VGM_SEGAPCM_EMU_H */
/*
 * End of segapcm_emu.h
 */
