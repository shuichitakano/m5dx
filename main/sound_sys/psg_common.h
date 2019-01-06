/* -*- mode:C++; -*-
 *
 * psg_common.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/05/22(Fri) 01:34:11
 *
 * $Id$
 */
#ifndef	_SoundSys_PSG_COMMON_H8E67440F40CD4BB7BB2547166A4D9460
#define	_SoundSys_PSG_COMMON_H8E67440F40CD4BB7BB2547166A4D9460

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

struct PSGState
{
  uint16_t	chNonMute_;
  uint16_t	chEnabled_;
  uint16_t	chKeyOn_;
  uint16_t	chKeyOnTrigger_;
  
  float		fBase_;
  
public:
  PSGState ();
  float getNote (int ch, const uint8_t* regCache) const;
  float getVolume (int ch, const uint8_t* regCache) const;
  bool mute (int ch, bool f);
  uint32_t getKeyOnTrigger ();
  uint32_t getEnabledChannels () const;
  bool update (int reg, int& v, const uint8_t* regCache);
  void setClock (int clock);
  const char* getStatusString (int ch, char* buf, int n,
                               const uint8_t* regCache) const;
};


} /* namespace sound_system */

#endif	/* _SoundSys_PSG_COMMON_H8E67440F40CD4BB7BB2547166A4D9460 */
/*
 * End of psg_common.h
 */
