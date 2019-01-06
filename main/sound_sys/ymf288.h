/* -*- mode:C++; -*-
 *
 * ymf288.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/04/27(Mon) 00:47:56
 *
 * $Id$
 */
#ifndef _SoundSys_YMF288_HF8633FE8C1AF42729D5F9F2E047B41A8
#define _SoundSys_YMF288_HF8633FE8C1AF42729D5F9F2E047B41A8

/**
 * @file
 * @brief
 */

/*
 * include
 */

#include "../audio_chip_base.h"
#include "../audio_system.h"
#include <stdint.h>

#include "opna_common.h"

/*
 * class
 */

namespace sound_sys
{

class YMF288 : public SoundSystem
{
    OPNAState state_;
    uint8_t regCache_[512];

    uint16_t currentReg_[2];
    int16_t chInst_[16];

    AudioChipBase* chip_;
    SystemInfo sysInfo_;

public:
    YMF288();

    inline void setChip(AudioChipBase* c) { chip_ = c; }
    AudioChipBase* detachChip()
    {
        auto t = chip_;
        chip_  = 0;
        return t;
    }

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
    int setClock(int clock);

    inline void setInstrumentNumber(int ch, int n) { chInst_[ch] = n; }
};

} /* namespace sound_sys */

#endif /* _SoundSys_YMF288_HF8633FE8C1AF42729D5F9F2E047B41A8 */
/*
 * End of ymf288.h
 */
