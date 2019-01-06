/* -*- mode:C++; -*-
 *
 * ay8910.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/04/28(Tue) 04:03:03
 *
 * $Id$
 */
#ifndef	_SoundSys_AY8910_HC2AA1679D45C40968FCB7C59CBAEF9F3
#define	_SoundSys_AY8910_HC2AA1679D45C40968FCB7C59CBAEF9F3

/**
  * @file
  * @brief 
  */

/*
 * include
 */

#include <stdint.h>
#include "../audio_system.h"
#include "../audio_chip_base.h"

#include "psg_common.h"

/*
 * class
 */

namespace sound_sys
{

class AY8910 : public SoundSystem
{
  PSGState	state_;
  
  uint8_t	currentReg_;
  uint8_t	regCache_[0x10];
  int16_t	chInst_[16];
  
  AudioChipBase* chip_;
  SystemInfo	sysInfo_;
  
public:
  AY8910 ();
  
  inline void setChip (AudioChipBase* c)	{ chip_ = c; }
  AudioChipBase* detachChip ()	{ auto t = chip_; chip_ = 0; return t; }
  
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
  
  void setValue (int addr, int v);
  int getValue (int addr);
  int setClock (int clock);

  inline void setInstrumentNumber (int ch, int n)	{ chInst_[ch] = n; }
};

} /* namespace sound_sys */

#endif	/* _SoundSys_AY8910_HC2AA1679D45C40968FCB7C59CBAEF9F3 */
/*
 * End of ay8910.h
 */
