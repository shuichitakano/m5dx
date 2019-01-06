/* -*- mode:C++; -*-
 *
 * scc_emu.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/28(Wed) 03:45:12
 *
 * $Id$
 */
#ifndef	_VGM_SCC_EMU_HD22ACF071B2148DFB8D55A608A6A1EDD
#define	_VGM_SCC_EMU_HD22ACF071B2148DFB8D55A608A6A1EDD

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

/*
 * class
 */

namespace sound_sys
{

class SCC : public SoundSystem, public SampleGenerator
{
  static constexpr int MAX_VOICE = 5;

  struct U16
    {
      uint8_t	l;
      uint8_t	h;

      inline int get () const	{ return (h << 8) | l; }
    };

  struct Register
    {
      int8_t	waveforms[5][32];	// 00-9f
      U16	rates[5];		// a0-a9
      uint8_t	volumes[5];		// aa-ae
      uint8_t	chEnable;		// af
      uint8_t	test[16];		// b0
      uint8_t	reserved[16];		// c0
    };

  union
    {
      Register		namedReg_;
      uint8_t		register_[sizeof(Register)];
    };
  
  uint8_t		chNonMute_	= 255;
  uint8_t		chEnabled_	= 255;
  uint8_t		chKeyOn_	= 0;
  uint8_t		chKeyOnTrigger_	= 0;
  
  uint32_t		voicePos_[MAX_VOICE];
  uint32_t		deltaTable_[0x1000];
  
  float			clock_;
  float			sampleRate_;
  float			fBase_;
  uint32_t		volume_;

  uint8_t		enabledCh_;
  SystemInfo		sysInfo_;

public:
  SCC ()			{ initialize (); }
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


protected:
  void writeWaveform (int offset, int data);
  void updateDeltaTable ();
};


} /* namespace sound_sys */

#endif	/* _VGM_SCC_EMU_HD22ACF071B2148DFB8D55A608A6A1EDD */
/*
 * End of scc_emu.h
 */
