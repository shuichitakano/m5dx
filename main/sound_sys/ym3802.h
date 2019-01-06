/* -*- mode:C++; -*-
 *
 * ym3802.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/11/15(Sun) 02:49:19
 *
 * $Id$
 */
#ifndef _VGM_YM3802_H64D53DAA7D624F768B401FF5198EF88C
#define _VGM_YM3802_H64D53DAA7D624F768B401FF5198EF88C

/**
 * @file
 * @brief
 */

/*
 * include
 */

#include "../audio_system.h"
#include "midi_common.h"
#include <stdint.h>

/*
 * class
 */

namespace sound_sys
{

class YM3802 : public SoundSystem
{
    enum
    {
        IVR = 0,
        RGR,
        ISR,
        ICR,

        IOR = 0, // R04
        IMR,
        IER,
        DMR = 1 * 4, // R14
        DCR,
        DSR,
        DNR,
        RRR, // R24
        RMR,
        AMR,
        ADR,
        RSR, // R34
        RCR,
        RDR,
        TRR = 4 * 4, // R44
        TMR,
        TSR = 5 * 4, // R54
        TCR,
        TDR,
        FSR = 6 * 4, // R64
        FCR,
        CCR,
        CDR,
        SRR, // R74
        SCR,
        SPRL,
        SPRH,
        GTRL, // R84
        GTRH,
        MTRL,
        MTRH,
        EDR, // R94
        DOR,
        EIR,
    };

public:
    YM3802();

    void setValue(int addr, int v);
    int getValue(int addr);
    void setClock(int clk, int clkm, int clkf);

    void reset();

    // SoundSystem
    virtual const SystemInfo& getSystemInfo() const;
    virtual float getNote(int ch, int) const;
    virtual int getNoteCount(int ch) const;
    virtual float getVolume(int ch) const;
    virtual float getPan(int ch) const;
    virtual int getInstrument(int ch) const;
    virtual bool mute(int ch, bool f);
    virtual uint32_t getKeyOnChannels() const;
    virtual uint32_t getKeyOnTrigger();
    virtual uint32_t getEnabledChannels() const;
    virtual const char* getStatusString(int ch, char* buf, int n) const;

protected:
    inline int getRegisterGroup() const { return ctrlReg_[RGR] & 0xf; }

private:
    int clk_  = 4000000;
    int clkm_ = 1000000;
    int clkf_ = 614400;

    uint8_t ctrlReg_[4];
    uint8_t reg_[64];

    SystemInfo sysInfo_;
    MIDICommon midiStatus_;

    uint16_t mute_ = 0;
};

} /* namespace sound_sys */

#endif /* _VGM_YM3802_H64D53DAA7D624F768B401FF5198EF88C */
/*
 * End of ym3802.h
 */
