/* -*- mode:C++; -*-
 *
 * c140_emu.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/12(Mon) 12:44:08
 *
 * $Id$
 */
#ifndef _VGM_C140_EMU_H
#define _VGM_C140_EMU_H

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

class C140 : public SoundSystem, public SampleGenerator
{
public:
    enum Type
    {
        TYPE_SYSTEM2,
        TYPE_SYSTEM21_A,
        TYPE_SYSTEM21_B,
        TYPE_ASIC219
    };

private:
    enum Mode
    {
        MODE_SIGNBIT      = 0x01,
        MODE_MULAW        = 0x08,
        MODE_LOOP         = 0x10,
        MODE_PHASEREVERSE = 0x40,
    };

    static constexpr int MAX_VOICE = 24;

    struct VoiceRegister
    {
        uint8_t volR;
        uint8_t volL;
        uint8_t freqH;
        uint8_t freqL;
        uint8_t bank;
        uint8_t mode;
        uint8_t startH;
        uint8_t startL;
        uint8_t endH;
        uint8_t endL;
        uint8_t loopH;
        uint8_t loopL;
        uint8_t reserved[4];
    };

    struct Voice
    {
        bool keyon;
        bool mute;
        uint8_t mode;
        uint16_t sampleSize;
        uint16_t sampleLoopSize;
        uint32_t freq;
        uint32_t pos;
        int32_t frac;
        int32_t val;
        int32_t prev;
        const int8_t* sample;

        const int8_t* nextSample;
        uint16_t nextSampleSize;
        uint16_t nextSampleLoopSize;
    };

    union {
        VoiceRegister voiceRegs_[MAX_VOICE];
        uint8_t register_[0x200];
    };

    Voice voices_[MAX_VOICE];

    uint32_t nextKeyOff_;

    uint32_t freqBase_;
    float clock_;
    float sampleRate_;

    Type type_;
    int chCount_;
    int volume_; // .8

    DataBlockContainer rom_;
    SystemInfo sysInfo_;

    uint32_t chKeyOn_;
    uint32_t chKeyOnTrigger_;

public:
    C140();
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

    void setValue(int addr, int v);
    int getValue(int addr);
    void setType(Type t);
    void setClock(int clock);
    void setVolume(float v);

    // SampleGenerator
    virtual void accumSamples(int32_t* buffer, uint32_t samples);
    virtual void setSampleRate(float rate);

    void addROMBlock(uint32_t addr, SimpleAutoBuffer&& data, uint32_t size);

protected:
    void updateFreqBase();
    uint32_t convertAddr(int addr, int bank, int ch) const;
};

} /* namespace sound_sys */

#endif /* _VGM_C140_EMU_H */
/*
 * End of c140_emu.h
 */
