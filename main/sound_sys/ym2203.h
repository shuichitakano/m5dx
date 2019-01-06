/* -*- mode:C++; -*-
 *
 * ym2203.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/04/27(Mon) 02:31:14
 *
 * $Id$
 */
#ifndef	_SoundSys_YM2203_HD40D4883DC714DE09936107515875979
#define	_SoundSys_YM2203_HD40D4883DC714DE09936107515875979

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

/*
 * class
 */

namespace sound_sys
{

class YM2203 : public SoundSystem
{
  uint8_t	regCache_[512];
  uint16_t	chNonMute_;
  uint16_t	chEnabled_;
  uint16_t	chKeyOn_;
  uint16_t	chKeyOnTrigger_;
  uint16_t	currentReg_;
  int16_t	chInst_[16];
  
  AudioChipBase* chip_;
  SystemInfo	sysInfo_;

  float		fBasePSG_;
  float		fBaseFM_;
  
  bool		fmOnly_ = false;

public:
  YM2203 ();
  
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
  
  void setFMOnly (bool f = true);
  
  inline void setInstrumentNumber (int ch, int n)	{ chInst_[ch] = n; }
};

} /* namespace sound_sys */

#endif	/* _SoundSys_YM2203_HD40D4883DC714DE09936107515875979 */
/*
 * End of ym2203.h
 */
