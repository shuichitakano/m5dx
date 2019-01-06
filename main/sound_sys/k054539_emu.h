/* -*- mode:C++; -*-
 *
 * k054539_emu.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/18(Sun) 00:44:03
 *
 * $Id$
 */
#ifndef	_VGM_K054539_EMU_H015C93F698C3486AB766FA3DFC44DC82
#define	_VGM_K054539_EMU_H015C93F698C3486AB766FA3DFC44DC82

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

class K054539 : public SoundSystem, public SampleGenerator
{
  static constexpr int MAX_VOICE = 8;

  struct VoiceRegisterA
    {
      uint8_t		pitchL;		// +00
      uint8_t		pitchM;		// +01
      uint8_t		pitchH;		// +02
      uint8_t		vol;		// +03
      uint8_t		reverb;		// +04
      uint8_t		pan;		// +05
      uint8_t		reverbDelayL;	// +06
      uint8_t		reverbDelayH;	// +07
      uint8_t		loopL;		// +08
      uint8_t		loopM;		// +09
      uint8_t		loopH;		// +0a
      uint8_t		reserved1;
      uint8_t		startL;		// +0c
      uint8_t		startM;		// +0d
      uint8_t		startH;		// +0e
      uint8_t		reseved2[17];
    };

  struct VoiceRegisterB
    {
      uint8_t	       	mode;
      uint8_t		loop;
    };
  
  struct VoiceRegisters
    {
      VoiceRegisterA	a[MAX_VOICE];
      uint8_t		reserved[0x100];
      VoiceRegisterB	b[MAX_VOICE];
    };

  enum RegisterType
    {
      REG_ANALOG_IN_PAN		= 0x13f,
      REG_KEYON			= 0x214,
      REG_KEYOFF		= 0x215,
      REG_ACTIVE_CH		= 0x22c,
      REG_DATA_RW		= 0x22d,
      REG_ROM_RAM_SEL		= 0x22e,
      REG_PCM_RAM_CONTROL	= 0x22f,
    };

  enum Mode
    {
      MODE_8BIT_PCM,
      MODE_16BIT_PCM,
      MODE_4BIT_DPCM,
    };
  
  struct Voice
    {
      bool		keyon;
      bool		mute;
      bool		loop;
      bool		reverse;
      
      Mode		mode;
      
      int32_t		pos;
      int32_t		frac;
      
      int32_t		delta;
      int32_t		posInc;
      
      uint32_t		volL;
      uint32_t		volR;
      uint32_t		volReverb;
      uint32_t		reverbDelay;
      
      int32_t		val;
      int32_t		prev;
      
      int32_t		sampleLoopOfs;
      const uint8_t*	sample;
      
      uint32_t		nextDeltaL;
      uint32_t		nextDeltaM;
      uint32_t		nextDeltaH;
      int32_t		nextSampleLoopOfs;
      const uint8_t*	nextSample;

    public:
      inline bool update8BitPCM ();
      inline bool update16BitPCM ();
      inline bool update4BitDPCM ();

    };

  union
    {
      VoiceRegisters	voiceRegs_;
      uint8_t		register_[0x230];
    };
  
  Voice			voices_[MAX_VOICE];

  float			volumeTable_[256];
  float			panTable_[15];
  
  int			reverbPos_;
  bool			reverbEnabled_;

  uint32_t		nextKeyOff_;

  uint32_t		romSize_;
  uint32_t		romMask_;

  uint32_t		rwPtr_;
  uint32_t		rwPtrTail_;
  uint8_t*		rwTop_;
  
  uint32_t		freqBase_;
  uint32_t		invFreqBase_;
  float			clock_;
  float			sampleRate_;
  uint32_t		volume_;
  
  uint32_t		chKeyOn_;
  uint32_t		chKeyOnTrigger_;
  
  uint8_t		ram_[0x4000];
  DataBlockContainer	rom_;
  SystemInfo		sysInfo_;

public:
  K054539 ()			{ initialize (); }
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
  void enableReverb (bool f);

  void addROMBlock (uint32_t addr, uint32_t romSize,
                    SimpleAutoBuffer&& data,
                    uint32_t size);

protected:
  void updateFreqBase ();

  void updateVolume (Voice& v,
                     uint_fast8_t vol, uint_fast8_t pan);

  void updateReverbVolume (Voice& v,
                           uint_fast8_t vol, uint_fast8_t reverb);


};

} /* namespace sound_sys */

#endif	/* _VGM_K054539_EMU_H015C93F698C3486AB766FA3DFC44DC82 */
/*
 * End of k054539_emu.h
 */
