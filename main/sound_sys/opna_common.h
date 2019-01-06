/* -*- mode:C++; -*-
 *
 * opna_common.h
 *
 * author(s) : Shuichi TAKANO
 * since 2015/05/27(Wed) 01:31:48
 *
 * $Id$
 */
#ifndef	_SoundSys_OPNA_COMMON_HFFDACECC232B4D5A96EF1D43C4209523
#define	_SoundSys_OPNA_COMMON_HFFDACECC232B4D5A96EF1D43C4209523

/**
  * @file
  * @brief 
  */

/*
 * include
 */

#include "psg_common.h"

/*
 * class
 */

namespace sound_sys
{

struct OPNAState : public PSGState
{
  float		fBaseFM_;

public:
  float getNote (int ch, const uint8_t* regCache) const;
  float getVolume (int ch, const uint8_t* regCache) const;
  float getPan (int ch, const uint8_t* regCache) const;
  bool update (int reg, int& v, const uint8_t* regCache);
  void setClock (int clock);
  const char* getStatusString (int ch, char* buf, int n,
                               const uint8_t* regCache) const;
};


} /* namespace sound_sys */

#endif	/* _SoundSys_OPNA_COMMON_HFFDACECC232B4D5A96EF1D43C4209523 */
/*
 * End of opna_common.h
 */
