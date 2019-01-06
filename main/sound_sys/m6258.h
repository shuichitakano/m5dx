/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/10/26(Sun) 04:52:08
 */
#ifndef _SoundSys_M6258_H
#define _SoundSys_M6258_H

#include "sound_system.h"
#include <functional>

namespace sound_sys
{

class M6258 final : public SoundSystem
{
public:
    enum SampleRate
    {
        SAMPLERATE_1_1024,
        SAMPLERATE_1_768,
        SAMPLERATE_1_512,
        SAMPLERATE_INHIBIT
    }; // SAM2 SAM1

    using Callback = std::function<void()>;

public:
    M6258(bool pcm8 = true);

    // void setInterface(M6258Interface* i) { interface_ = i; }
    // M6258Interface* detachInterface()
    // {
    //     auto t     = interface_;
    //     interface_ = 0;
    //     return t;
    // }

    void setNote(int ch, int n) { note_[ch] = n; }

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

    // M6258
    void startTransfer(const uint8_t* p, int length)
    {
        // if (interface_)
        //     interface_->startTransfer(p, length);
    }
    void stopTransfer()
    {
        // if (interface_)
        //     interface_->stopTransfer();
    }
    void play();
    void stop();
    void pause()
    {
        // if (interface_)
        //     interface_->pause();
    }
    void resume()
    {
        // if (interface_)
        //     interface_->resume();
    }
    void setChMask(bool l, bool r);
    bool setFreq(bool _8M);
    void setSampleRate(SampleRate mode)
    {
        // if (interface_)
        //     interface_->setSampleRate(mode);
    }
    void setCallback(Callback f)
    {
        // if (interface_)
        // {
        //     interface_->setCallback(f, p);
        // }
    }

    void pcm8(int ch, const void* addr, int mode, int len);
    void resetPCM8();

    bool isKeyOn() { return keyOn_ & 1; }

private:
    //    M6258Interface* interface_;
    SystemInfo sysInfo_;
    int note_[8];
    float vol_[8];
    float pan_ = 0;
    int mode_[8];
    uint32_t enabledCh_    = 0xff;
    uint32_t keyOn_        = 0;
    uint32_t keyOnTrigger_ = 0;
};

} /* namespace sound_sys */

#endif /* _SoundSys_M6258_H */
