/* -*- mode:C++; -*-
 *
 * midi_common.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2016/04/24(Sun) 05:52:25
 *
 * $Id$
 */

/*
 * include
 */

#include "midi_common.h"
#include <assert.h>
#include <stdio.h>

/*
 * code
 */

namespace sound_sys
{

void
MIDICommon::messageByte(uint8_t value)
{
    if (value < 0x80)
    {
        // message data
        if (!exclusive_ && messageSize_ != 0)
        {
            message_[messageIdx_++] = value;
            if (messageIdx_ == messageSize_)
            {
                command(message_);
                messageSize_ = 0;
            }
        }
    }
    else
    {
        if (value < 0xf0)
        {
            // message header
            static const uint8_t size[] = {3, 3, 3, 3, 2, 2, 3};
            message_[0]                 = value;
            messageIdx_                 = 1;
            messageSize_                = size[(value >> 4) - 8];
        }
        else
        {
            // system common
            if (value == 0xf0)
                exclusive_ = true;
            else if (value == 0xf7)
                exclusive_ = false;
        }
    }
}

void
MIDICommon::command(const uint8_t* message)
{
    int chIdx  = message[0] & 15;
    int chMask = 1 << chIdx;
    auto& ch   = channels_[chIdx];

    switch (message[0] >> 4)
    {
    case 8:
        // note off
        ch.noteOff(message_[1]);
        if (ch.voiceCount_ == 0)
            keyOnChannels_ &= ~chMask;
        break;

    case 9:
        // note on
        ch.noteOn(message_[1], message_[2]);
        if (ch.voiceCount_)
        {
            keyOnChannels_ |= chMask;
            keyOnChannelsTrigger_ |= chMask;
        }
        else
            keyOnChannels_ &= ~chMask;
        break;

    case 0xa:
        // polyphonic key pressure
        break;

    case 0xb:
        // control change
        {
            int ccv = message_[2];
            switch (message_[1])
            {
            case 0:
                ch.bank_ = (ch.bank_ & 0x7f) | ccv;
                break;
            case 0x20:
                ch.bank_ = (ch.bank_ & 0x3f80) | (ccv << 7);
                break;

            case 0x1:
                ch.modulation_ = (ch.modulation_ & 0x7f) | ccv;
                break;
            case 0x21:
                ch.modulation_ = (ch.modulation_ & 0x3f80) | (ccv << 7);
                break;

            case 0xa:
                ch.pan_ = (ch.pan_ & 0x7f) | ccv;
                break;
            case 0x2a:
                ch.pan_ = (ch.pan_ & 0x3f80) | (ccv << 7);
                break;

            case 0xb:
                ch.expression_ = (ch.expression_ & 0x7f) | ccv;
                break;
            case 0x2b:
                ch.expression_ = (ch.expression_ & 0x3f80) | (ccv << 7);
                break;

            // channel mode message
            case 0x78:
            case 0x7b:
                ch.resetNotes();
                break;

            case 0x79:
                ch.reset();
                break;
            }
        }
        break;

    case 0xc:
        // program change
        ch.program_ = message_[1];
        break;

    case 0xd:
        // pressure
        break;

    case 0xe:
        // pitch
        ch.pitch_ = (message_[1] | (message_[2] << 7)) - 8192;
        break;
    }
}

void
MIDICommon::reset()
{
    exclusive_   = false;
    messageIdx_  = 0;
    messageSize_ = 0;
    for (auto& ch : channels_)
        ch.reset();
}

void
MIDICommon::Channel::reset()
{
    resetNotes();
    bank_       = 0;
    modulation_ = 0;
    pan_        = 0;
    expression_ = 127;
    pitch_      = 0;
}

void
MIDICommon::Channel::resetNotes()
{
    voiceCount_ = 0;
    voiceMask_  = 0;
}

void
MIDICommon::Channel::noteOn(int note, int vel)
{
    if (vel == 0)
    {
        noteOff(note);
        return;
    }

    int i = __builtin_clz(voiceMask_ ^ 0xffffffff);
    if (i < MAX_CH_VOICES)
    {
        voiceMask_ |= 1 << (31 - i);

        auto& v     = voices_[i];
        v.note_     = note;
        v.velocity_ = vel;
        ++voiceCount_;

        vol_ = vel;
    }
}

void
MIDICommon::Channel::noteOff(int note)
{
    uint32_t m = voiceMask_;
    while (1)
    {
        int i = __builtin_clz(m);
        if (i >= MAX_CH_VOICES)
            break;

        auto& v = voices_[i];
        if (v.note_ == note)
        {
            voiceMask_ &= ~(1 << (31 - i));
            --voiceCount_;
            break;
        }

        m &= ~(1 << (31 - i));
    };
}

int
MIDICommon::Channel::getNoteList(const Voice* buf[MAX_CH_VOICES]) const
{
    auto p     = buf;
    uint32_t m = voiceMask_;
    while (1)
    {
        int i = __builtin_clz(m);
        if (i >= MAX_CH_VOICES)
            break;

        *p++ = &voices_[i];
        m &= ~(1 << (31 - i));
    };
    int count = p - buf;
    assert(voiceCount_ == count);
    return voiceCount_;
}

int
MIDICommon::getNoteCount(int ch) const
{
    return channels_[ch].voiceCount_;
}

float
MIDICommon::getNote(int ch, int i) const
{
    updateCache(ch);
    return voiceCache_[i]->note_ - 69;
}

void
MIDICommon::updateCache(int ch) const
{
    if (ch == voiceCacheCh_)
        return;

    voiceCacheCh_ = ch;
    channels_[ch].getNoteList(voiceCache_);
}

float
MIDICommon::getVolume(int ch) const
{
    auto& c = channels_[ch];
    return c.vol_ * c.expression_ * (1.0f / (127 * 127));
}

float
MIDICommon::getPan(int ch) const
{
    return (channels_[ch].pan_ - 8192) * (1.0f / 8192.0f);
}

int
MIDICommon::getInstrument(int ch) const
{
    return channels_[ch].program_;
}

uint8_t
MIDICommon::modifyKeyOnLastByte(uint8_t data, uint16_t muteMask) const
{
    if (messageIdx_ == 2 && ((message_[0] & 0xf0) == 0x90))
    {
        int ch = message_[0] & 0xf;
        if (muteMask & (1 << ch))
            return 0;
    }
    return data;
}

uint16_t
MIDICommon::getKeyOnChannels() const
{
    return keyOnChannels_;
}

uint16_t
MIDICommon::getKeyOnTrigger()
{
    auto v                = keyOnChannelsTrigger_;
    keyOnChannelsTrigger_ = 0;
    return v;
}

const char*
MIDICommon::getStatusString(int ch, char* buf, int n) const
{
    auto& c = channels_[ch];
    snprintf(buf,
             n,
             "B%04X P%5d M%3d E%3d",
             c.bank_,
             c.pitch_,
             c.modulation_ >> 7,
             c.expression_ >> 7);
    return buf;
}

} /* namespace sound_sys */

/*
 * End of midi_common.cxx
 */
