/* -*- mode:C++; -*-
 *
 * k053260_emu.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/25(Sun) 00:17:14
 *
 * $Id$
 */
#ifndef	_VGM_K053260_EMU_HF6E1B382A91D4125A3BF9F8A2C20768B
#define	_VGM_K053260_EMU_HF6E1B382A91D4125A3BF9F8A2C20768B

/**
  * @file
  * @brief 
  */

/*
 * include
 */

#include <stdint.h>
#include "../audio_system.h"
#include "../sample_gen.h"
#include "../data_block.h"

/*
 * class
 */

namespace sound_sys
{

class K053260 : public SoundSystem, public SampleGenerator
{
  static constexpr int MAX_VOICE = 4;

  struct VoiceRegister
    {
      uint8_t		rateL;		// +00
      uint8_t		rateH;		// +01
      uint8_t		sizeL;		// +02
      uint8_t		sizeH;		// +03
      uint8_t		startL;		// +04
      uint8_t		startH;		// +05
      uint8_t		bank;		// +06
      uint8_t		volume;		// +07
    };
  
  struct VoiceRegisters
    {
      uint8_t		reserved[8];
      VoiceRegister	ch[4];
    };
  
  enum RegisterType
    {
      REG_CH_ENABLE	= 0x28,
      REG_LOOP_DPCM	= 0x2a,
      REG_PAN01		= 0x2c,
      REG_PAN23		= 0x2d,
      REG_CONTROL	= 0x2f,
    };

  enum Mode
    {
      MODE_PCM,
      MODE_DPCM,
    };
  
  struct Voice
    {
      bool		keyon;
      bool		mute;
      bool		loop;
      bool		reverse;
      bool		dpcm;
      bool		nextReverse;
      
      uint8_t		pan;	// .3
      
      int32_t		pos;
      int32_t		frac;
      
      int32_t		val;
      int32_t		prev;
      
      const uint8_t*	sample;
      int32_t		posEnd;
      
      const uint8_t*	nextSample;
      uint32_t		nextPosEnd;
    };

  union
    {
      VoiceRegisters	voiceRegs_;
      uint8_t		register_[0x30];
    };
  
  Voice			voices_[MAX_VOICE];
  uint32_t		deltaTable_[0x1000];
  
  float			clock_;
  float			sampleRate_;
  uint32_t		volume_;

  uint32_t		nextKeyOff_;

  DataBlockContainer	rom_;
  SystemInfo		sysInfo_;
  
  uint32_t		chKeyOn_;
  uint32_t		chKeyOnTrigger_;
  

public:
  K053260 ()			{ initialize (); }
  void initialize ();

  // SoundSystem
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
  
  // SampleGenerator
  virtual void accumSamples (int32_t* buffer, uint32_t samples);
  virtual void setSampleRate (float rate);
  
  void setValue (int addr, int v);
  int getValue (int addr);
  void setClock (int clock);
  void setVolume (float v);

  void addROMBlock (uint32_t addr, SimpleAutoBuffer&& data,
                    uint32_t size);

public:
  void updateDeltaTable ();
};


} /* namespace sound_sys */

#endif	/* _VGM_K053260_EMU_HF6E1B382A91D4125A3BF9F8A2C20768B */
/*
 * End of k053260_emu.h
 */
