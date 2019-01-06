/* -*- mode:C++; -*-
 *
 * midi_common.h
 *
 * author(s) : Shuichi TAKANO
 * since 2016/04/23(Sat) 03:07:15
 *
 * $Id$
 */
#ifndef	_SoundSys_MIDI_COMMON_HEB7C2AC410C341C3B005AFC247789E35
#define	_SoundSys_MIDI_COMMON_HEB7C2AC410C341C3B005AFC247789E35

/**
  * @file
  * @brief 
  */

/*
 * include
 */

#include <stdint.h>

/*
 * class
 */

namespace sound_sys
{

class MIDICommon
{
    static constexpr int MAX_CH_VOICES	= 16;
    static constexpr int CH_COUNT		= 16;
    
    struct Voice
    {
        uint8_t	note_;
        uint8_t	velocity_;
    };
    
    struct Channel
    {
        uint32_t	voiceMask_;	// bit31-
        Voice		voices_[MAX_CH_VOICES];
        int16_t		pitch_;
        uint16_t	bank_;
        uint16_t	modulation_;
        uint16_t	pan_;
        uint16_t	expression_;
        uint8_t		voiceCount_;
        uint8_t		program_;
        
        int			vol_;
        
    public:
        void reset();
        void resetNotes();
        void noteOn(int note, int vel);
        void noteOff(int note);
        int getNoteList(const Voice* buf[MAX_CH_VOICES]) const;
    };
    
    Channel		channels_[CH_COUNT];
    
    uint8_t		message_[3];
    uint8_t		messageIdx_	 = 0;
    uint8_t		messageSize_ = 0;
    
    bool		exclusive_ = false;

    mutable int	voiceCacheCh_= -1;
    mutable const Voice* voiceCache_[MAX_CH_VOICES];
    
    uint16_t	keyOnChannels_;
    uint16_t	keyOnChannelsTrigger_;
    
public:
    MIDICommon() { reset(); }
    void reset();

    void messageByte(uint8_t value);
    void command(const uint8_t* message);

    int getNoteCount(int ch) const;
    float getNote(int ch, int i) const;
    float getVolume(int ch) const;
    float getPan(int ch) const;
    int getInstrument(int ch) const;

    uint16_t getKeyOnChannels() const;
    uint16_t getKeyOnTrigger();

    const char* getStatusString(int ch, char* buf, int n) const;

    uint8_t modifyKeyOnLastByte(uint8_t data, uint16_t muteMask) const;

protected:
    void updateCache(int ch) const;
};


} /* namespace sound_sys */

#endif	/* _SoundSys_MIDI_COMMON_HEB7C2AC410C341C3B005AFC247789E35 */
/*
 * End of midi_common.h
 */
