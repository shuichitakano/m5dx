/* -*- mode:C++; -*-
 *
 * opna_common.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/05/27(Wed) 02:37:10
 *
 * $Id$
 */

/*
 * include
 */

#include "opna_common.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

/*
 * code
 */

namespace sound_sys
{
  namespace
    {
      enum
        {
          SLOT1	= 0,
          SLOT2	= 2,
          SLOT3	= 1,
          SLOT4	= 3,
        };
      
      int getCarrierMask (int algorithm)
        {
          int masks[] =
            {
              (1 << SLOT4),
              (1 << SLOT4),
              (1 << SLOT4),
              (1 << SLOT4),
              (1 << SLOT2) | (1 << SLOT4),
              (1 << SLOT2) | (1 << SLOT3) | (1 << SLOT4),
              (1 << SLOT2) | (1 << SLOT3) | (1 << SLOT4),
              (1 << SLOT1) | (1 << SLOT2) | (1 << SLOT3) | (1 << SLOT4),
            };
          return masks[algorithm & 7];
        }
    }

float
OPNAState::getNote (int ch, const uint8_t* regCache) const
{
  if (ch < 3)
    return PSGState::getNote (ch, regCache);
  else if (ch < 9)
    {
      // FM
      int ofs = ch + (ch < 6 ? -3 : (0x100 - 6));
      
      int rl = regCache[0xa0 + ofs];
      int rh = regCache[0xa4 + ofs];
      int fn = ((rh << 8) | rl) & 2047;
      int blk = ((rh >> 3) & 7) - 1;
      
      // fn = f * 2^20/(clock / 144)/2^blk
      // f = fn * clock/144 * 2^blk / 2^20
      // key = 12 log_2(f/440)
      float f = fBaseFM_ * (fn << blk);
      return 17.31234f * log (f) - 105.3763f;
    }

  return 0;
}

float
OPNAState::getVolume (int ch, const uint8_t* regCache) const
{
  if (ch < 3)
    return PSGState::getVolume (ch, regCache);

  if (ch < 9)
    {
      int ofs = ch + (ch < 3 ? -3 : (0x100 - 6));
      int al = regCache[0xb0 + ofs] & 7;
      int cm = getCarrierMask (al);
      int tl0 = (cm & (1 << 0) ? regCache[0x40 + 4*0 + ofs] & 127 : 127);
      int tl1 = (cm & (1 << 1) ? regCache[0x40 + 4*1 + ofs] & 127 : 127);
      int tl2 = (cm & (1 << 2) ? regCache[0x40 + 4*2 + ofs] & 127 : 127);
      int tl3 = (cm & (1 << 3) ? regCache[0x40 + 4*3 + ofs] & 127 : 127);
      return 1.0f - std::min (tl0,
                              std::min (tl1,
                                        std::min (tl2, tl3))) * (1/127.0f);
    }
  else if (ch < 15)
    {
      return (regCache[0x18 + ch - 9] & 31) * (1 / 31.0f);
    }
  return 1;
}

float
OPNAState::getPan (int ch, const uint8_t* regCache) const
{
  float tbl[] = { 0.0f, 1.0f, -1.0f, 0.0f };
  
  if (ch < 3)
    return 0;
  else if (ch >=3 && ch < 9)
    {
      int ofs = ch + (ch < 3 ? -3 : (0x100 - 6));
      int v = regCache[0xb4 + ofs];
      return tbl[v >> 6];
    }
  else if (ch < 15)
    {
      return tbl[regCache[0x18 + ch - 9] >> 6];
    }
  return 0;
}


bool
OPNAState::update (int reg, int& v, const uint8_t* regCache)
{
  if (reg < 0x10)
    return PSGState::update (reg, v, regCache);
  
  if (reg == 0x28)
    {
      // fm keyon
      static const int chmap[] = { 0,1,2,0,3,4,5,0 };
      int ch = chmap[v & 7];
      int chBit = (1 << 3) << ch;
      if (v & (15 << 4))
        {
          if (chNonMute_ & chBit)
            {
              chKeyOn_ |= chBit;
              chKeyOnTrigger_ |= chBit;
            }
          else
            return false;
        }
      else
        chKeyOn_ &= ~chBit;
    }
  else if (reg == 0x10)
    {
      for (int i = 0; i < 6; ++i)
        {
          int b = 1 << i;
          int chBit = b << 9;
          if (v & b)
            {
              if (chNonMute_ & chBit)
                {
                  chKeyOn_ |= chBit;
                  chKeyOnTrigger_ |= chBit;
                }
              else
                v &= ~b;
            }
        }
    }
  return true;
}

void
OPNAState::setClock (int clock)
{
  PSGState::setClock (clock);
  fBaseFM_ = clock * 6.622738308376736e-9f;
}

const char*
OPNAState::getStatusString (int ch, char* buf, int n,
                            const uint8_t* regCache) const
{
  if (ch < 3)
    return PSGState::getStatusString (ch, buf, n, regCache);
  if (ch >= 9 && ch < 15)
    {
      const char* name[] = {
        "BD", "SD", "TOP", "HH", "TOM", "RYM",
      };
      return name[ch - 9];
    }
  return 0;
}


} /* namespace sound_sys */

/*
 * End of opna_common.cxx
 */
