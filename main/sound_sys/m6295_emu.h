/* -*- mode:C++; -*-
 *
 * m6295_emu.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/31(Sat) 12:24:25
 *
 * $Id$
 */
#ifndef	_VGM_M6295_EMU_H97916C913C4645F6A07609CD90FC8AEB
#define	_VGM_M6295_EMU_H97916C913C4645F6A07609CD90FC8AEB

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
#include "../m6258_coder.h"

/*
 * class
 */

namespace sound_sys
{

class M6295 : public SoundSystem, public SampleGenerator
{
  static constexpr int MAX_VOICE = 4;

  struct Voice
    {
      bool		keyon;
      bool		mute;
      
      uint8_t		volume;
      
      uint32_t		pos;
      uint32_t		posEnd;
      const uint8_t*	sample;
      
      uint32_t		nextPosEnd;
      const uint8_t*	nextSample;
      
      M6258Coder	coder;
    };
  
  Voice			voices_[MAX_VOICE];
  uint8_t		nextKeyOff_;
  
  int			phrase_;
  
  float			clock_;
  float			sampleRate_;
  uint32_t		volume_;
  
  uint32_t		delta_;
  int32_t		frac_;
  int32_t		val_;
  int32_t		prev_;
  
  bool			pin7_;
  uint32_t		bankOffset_;
  DataBlockContainer	rom_;
  SystemInfo		sysInfo_;

public:
  M6295 ()			{ initialize (); }
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
  
  void setValue (int value);
  int getValue ();
  
  void setClock (int clock);
  void setVolume (float v);
  void setPin7 (bool v);
  void setBankOffset (uint32_t v);

  void addROMBlock (uint32_t addr, SimpleAutoBuffer&& data,
                    uint32_t size);

protected:
  void updateDelta ();

};


} /* namespace sound_sys */

#endif	/* _VGM_M6295_EMU_H97916C913C4645F6A07609CD90FC8AEB */
/*
 * End of m6295_emu.h
 */
