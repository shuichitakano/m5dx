/* -*- mode:C++; -*-
 *
 * ym3802.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/11/15(Sun) 02:58:14
 *
 * $Id$
 */

/*
 * include
 */

#include "ym3802.h"
#include <stdio.h>

#include "../usb_midi.h"

/*
 * code
 */

namespace sound_sys
{

YM3802::YM3802()
{
    sysInfo_.channelCount	= 16;
    sysInfo_.systemID 		= SoundSystem::SYSTEM_MIDI;
    
    sysInfo_.clock 			= 31250;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_MIDI;
    sysInfo_.actualClock 	= 31250;

    sysInfo_.multiVoice		= true;

    reset();
}

void
YM3802::setValue(int addr, int v)
{
//    printf("YM3802: setValue(%x, %02x)\n", addr, v);
    
    if (addr < 4)
    {
//        printf("YM3802: write reg%02x : %02x\n", addr, v);
        ctrlReg_[addr] = v;
        switch(addr)
        {
        case IVR:
            break;

        case RGR:
            if (v & 0x80)
                reset();
            break;

        case ISR:
            break;

        case ICR:
            break;
        }
       return;
    }
    
//    printf("YM3802: write reg%02x : %02x\n", (getRegisterGroup() << 4) | addr, v);
    int rid = (addr & 3) | (getRegisterGroup() << 2);
    reg_[rid] = v;
    
    switch(rid)
    {
    case TDR:
        // FIFO-Tx Data
//        printf("%02x:", v);
        v = midiStatus_.modifyKeyOnLastByte(v, mute_);
        sendMIDI(v);
        midiStatus_.messageByte(v);
        break;
    };
}

int
YM3802::getValue(int addr)
{
//    printf("YM3802: getValue(%x)\n", addr);
    
    if (addr < 4)
    {
//        printf("YM3802: read reg%02x : %02x\n", addr, ctrlReg_[addr]);
        return ctrlReg_[addr];
    }

    auto v = reg_[(addr & 3) | (getRegisterGroup() << 2)];
//    printf("YM3802: read reg%02x : %02x\n",
//           (getRegisterGroup() << 4) | addr, v);
    return v;
}

void
YM3802::setClock(int clk, int clkm, int clkf)
{
//    printf("YM3802: setClock(%d, %d, %d)\n", clk, clkm, clkf);
    clk_  = clk;
    clkm_ = clkm;
    clkf_ = clkf;
}

void
YM3802::reset()
{
//    printf("YM3802: reset\n");
    ctrlReg_[IVR] = 0;
    ctrlReg_[RGR] = 0x80;
    ctrlReg_[ISR] = 0;
    ctrlReg_[ICR] = 0;
    
    for(auto& v : reg_)
        v = 0;

#if 1
    // hack
    // TSR
    reg_[TSR] = (0x80 |		// FIFO-Tx empty
                 0x40 |		// FIFO-Tx ready
                 0x20 |		// FIFO-Tx idle
                 0);
#endif
}

const YM3802::SystemInfo&
YM3802::getSystemInfo () const
{
    return sysInfo_;
}

float
YM3802::getNote (int ch, int voice) const
{
//    return 0;
    return midiStatus_.getNote(ch, voice);
}

int
YM3802::getNoteCount(int ch) const
{
    return midiStatus_.getNoteCount(ch);
}

float
YM3802::getVolume (int ch) const
{
    return midiStatus_.getVolume(ch);
}

float
YM3802::getPan (int ch) const
{
    return midiStatus_.getPan(ch);
}

int
YM3802::getInstrument (int ch) const
{
    return midiStatus_.getInstrument(ch);
}

bool
YM3802::mute (int ch, bool f)
{
    uint16_t m = 1 << ch;
    if (f)
        mute_ |= f;
    else
        mute_ &= ~m;
    return true;
}

uint32_t
YM3802::getKeyOnChannels () const
{
    return midiStatus_.getKeyOnChannels();
}

uint32_t
YM3802::getKeyOnTrigger ()
{
    __disable_irq();
    auto r = midiStatus_.getKeyOnTrigger();
    __enable_irq();
    return r;
}

uint32_t
YM3802::getEnabledChannels () const
{
    return ~mute_;
}

const char*
YM3802::getStatusString (int ch, char* buf, int n) const
{
    return midiStatus_.getStatusString(ch, buf, n);
}


} /* namespace sound_sys */

/*
 * End of ym3802.cxx
 */
