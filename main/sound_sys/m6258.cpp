/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/11/01(Sat) 13:08:38
 *
 */

#include "m6258.h"
//#include "../pcm8_inst.h"
#include <stdio.h>

namespace sound_sys
{

M6258::M6258(bool pcm8)
{
    sysInfo_.channelCount = pcm8 ? 8 : 1;
    sysInfo_.systemID     = SoundSystem::SYSTEM_M6258;

    sysInfo_.clock          = 8000000;
    sysInfo_.actualSystemID = SoundSystem::SYSTEM_M6258;
    sysInfo_.actualClock    = 8000000;

    for (int i = 0; i < 8; ++i)
    {
        mode_[i] = 4;
        vol_[i]  = 0;
        note_[i] = 0;
    }
}

const M6258::SystemInfo&
M6258::getSystemInfo() const
{
    return sysInfo_;
}

float
M6258::getNote(int ch, int) const
{
    return note_[ch];
}

float
M6258::getVolume(int ch) const
{
    return vol_[ch];
}

float
M6258::getPan(int) const
{
    return pan_;
}

int
M6258::getInstrument(int) const
{
    return -1;
}

bool
M6258::mute(int ch, bool f)
{
    if (f)
        enabledCh_ &= ~(1 << ch);
    else
        enabledCh_ |= 1 << ch;
    return true;
}

uint32_t
M6258::getKeyOnChannels() const
{
    return keyOn_;
}

uint32_t
M6258::getKeyOnTrigger()
{
    //    __disable_irq();
    uint32_t v    = keyOnTrigger_;
    keyOnTrigger_ = 0;
    //    __enable_irq();
    return v;
}

uint32_t
M6258::getEnabledChannels() const
{
    return enabledCh_;
}

void
M6258::play()
{
    if (enabledCh_ & 1)
    {
        keyOn_ |= 1;
        keyOnTrigger_ |= 1;
        // if (interface_)
        //     interface_->play();
        vol_[0] = 0.5f;
    }
}

void
M6258::stop()
{
    keyOn_ &= ~1;
    // if (interface_)
    //     interface_->stop();
}

void
M6258::setChMask(bool l, bool r)
{
    if (l)
    {
        if (r)
            pan_ = 0;
        else
            pan_ = -1;
    }
    else
    {
        if (r)
            pan_ = 1;
        else
            pan_ = 0;
    }
    // if (interface_)
    //     interface_->setChMask(l, r);
}

bool
M6258::setFreq(bool _8M)
{
    sysInfo_.systemID     = SoundSystem::SYSTEM_M6258;
    sysInfo_.channelCount = 1;
    mode_[0]              = 4;

    sysInfo_.clock = _8M ? 8000000 : 4000000;
    // if (interface_ && interface_->setFreq(_8M))
    // {
    //     sysInfo_.actualClock = sysInfo_.clock;
    //     return true;
    // }
    return false;
}

const char*
M6258::getStatusString(int ch, char* buf, int n) const
{
    /*
    const char* modeStr[] = {
        " 3.9K",
        " 5.2K",
        " 7.8K",
        "10.4K",
        "15.6K",
        "16BIT",
        " 8BIT",
        "",
    };

    int m           = mode_[ch];
    const void* ptr = getCurrentPointerPCM8(ch);
    if (!ptr && ch == 0 && interface_)
        ptr = interface_->getCurrentTransferPointer();
    snprintf(buf,
             n,
             "NOTE% 3d MODE%d %s ADDR$%08X",
             note_[ch] + 40,
             m,
             modeStr[m],
             (intptr_t)ptr);

    return buf;
    */
    return 0;
}

void
M6258::pcm8(int ch, const void* addr, int mode, int len)
{
    uint32_t mask = 1 << ch;
    if (enabledCh_ & mask)
    {
        keyOn_ |= mask;
        keyOnTrigger_ |= mask;
        //        PCM8Out(ch, addr, mode, len);

        if ((mode & 0xff) != 0xff)
        {
            int pan = mode & 3;
            switch (pan)
            {
            case 0:
                pan_ = 0;
                keyOn_ &= ~mask;
                break;
            case 1:
                pan_ = -1.0f;
                break;
            case 2:
                pan_ = 1.0f;
                break;
            case 3:
                pan_ = 0;
                break;
            }
            if (pan != 0)
            {
                sysInfo_.systemID     = SoundSystem::SYSTEM_PCM8;
                sysInfo_.channelCount = 8;
            }
        }

        if ((mode & 0xff0000) != 0xff0000)
            vol_[ch] = (mode & 0xff0000) * (1.0f / 0xf0000);

        if ((mode & 0xff00) != 0xff00)
            mode_[ch] = (mode >> 8) & 7;
    }
}

void
M6258::resetPCM8()
{
    for (int i = 0; i < 8; ++i)
    {
        vol_[i]  = 0;
        mode_[i] = 4;
        //        PCM8Out(i, 0, 0x080400, 0);
    }
    keyOn_ = 0;
}

} /* namespace sound_sys */
