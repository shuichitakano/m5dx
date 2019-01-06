/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/12/16(Tue) 04:42:36
 */
#ifndef _SoundSys_Y8950_H
#define _SoundSys_Y8950_H

#include "sound_system.h"
#include <stdint.h>

namespace sound_sys
{

class Y8950 : public SoundSystem
{
    uint8_t regCache_[256];
    uint16_t chNonMute_;
    uint16_t chEnabled_;
    uint16_t chKeyOn_;
    uint16_t chKeyOnTrigger_;
    uint8_t currentReg_;
    int16_t chInst_[15];
    float fBase_;
    uint16_t cnvFNumber_[9];
    uint32_t fnCnv_;

    AudioChipBase* chip_;
    SystemInfo sysInfo_;

public:
    Y8950();

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

    // AudioChipBase
    void setValue(int addr, int v);
    int getValue(int addr);
    int setClock(int clock);

    inline void setInstrumentNumber(int ch, int n) { chInst_[ch] = n; }
    inline int getReg(int reg) const { return regCache_[reg]; }
};

} /* namespace sound_sys */

#endif /* _SoundSys_Y8950_H */
