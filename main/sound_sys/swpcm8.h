/* -*- mode:C++; -*-
 *
 * swpcm8.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/12/17(Thu) 04:22:42
 *
 * $Id$
 */
#ifndef	_VGM_SWPCM8_HF82945CEC14C4D56A63177C905481210
#define	_VGM_SWPCM8_HF82945CEC14C4D56A63177C905481210

/**
  * @file
  * @brief 
  */

/*
 * include
 */

#include <stdint.h>
#include "../sample_gen.h"
#include "../data_block.h"
#include "../m6258_coder.h"

/*
 * class
 */

namespace sound_sys
{

class SWPCM8 : public SampleGenerator
{
public:
    enum Type
    {
        TYPE_ADPCM,
        TYPE_PCM16,
        TYPE_PCM8,
    };

private:
    static constexpr int MAX_VOICE = 8;

    
    struct Voice
    {
        bool			keyon;
        bool			mute;

        uint8_t			nextType;
        uint8_t			type;
        uint8_t			pan;
        uint8_t			volume;
        
        uint32_t		pos;
        uint32_t		posEnd;
        uint32_t		delta;
        const uint8_t*	sample;
        
        int32_t			val;
        int32_t			frac;
        int32_t			prev;
        
        uint32_t		nextPosEnd;
        const uint8_t*	nextSample;
        
        M6258Coder		coder;
    };
    
    Voice				voices_[MAX_VOICE];
    uint8_t				nextKeyOff_;
    
    uint8_t				pan_;
    uint8_t				type_;
    bool				panShare_;
    
    float				clock_;
    float				sampleRate_;
    float				baseDelta_;
    uint32_t			volume_;
    
    
public:
    SWPCM8()			{ initialize(); }
    void initialize();



    // SoundSystem
#if 0
    virtual const SystemInfo& getSystemInfo () const;
    virtual float getNote (int ch, int) const;
    virtual float getVolume (int ch) const;
    virtual float getPan (int ch) const;
    virtual int getInstrument (int ch) const;
    virtual bool mute (int ch, bool f);
    virtual uint32_t getKeyOnChannels () const;
    virtual uint32_t getKeyOnTrigger ();
    virtual uint32_t getEnabledChannels () const;
    virtual const char* getStatusString (int ch, char* buf, int n) const;
#endif

    // SampleGenerator
    virtual void accumSamples(int32_t* buffer, uint32_t samples);
    virtual void setSampleRate(float rate);
    
    void setClock(int clock);
    void setVolume(float v);
    
    void setChPan(int ch, int pan);
    void setChVolume(int ch, uint8_t vol);
    void setChRate(int ch, float r);
    void play(int ch, void* addr, int len, Type type = TYPE_ADPCM);
    void stop(int ch);

    bool isChKeyOn(int ch) const		{ return voices_[ch].keyon; }
    
protected:
    void updateDelta ();

};

} /* namespace sound_sys */

#endif	/* _VGM_SWPCM8_HF82945CEC14C4D56A63177C905481210 */
/*
 * End of swpcm8.h
 */
