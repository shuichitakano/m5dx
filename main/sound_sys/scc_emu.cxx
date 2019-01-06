/* -*- mode:C++; -*-
 *
 * scc_emu.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/29(Thu) 01:51:59
 *
 * $Id$
 */

/*
 * include
 */

#include <string.h>
#include "scc_emu.h"
#include <math.h>

/*
 * code
 */

namespace sound_sys
{

void
SCC::initialize ()
{
  memset (register_, 0, sizeof (register_));
  memset (voicePos_, 0, sizeof (voicePos_));
  
  clock_ = 0;
  sampleRate_ = 0;
  enabledCh_ = 0xff;
  
  setSampleRate (48000.0f);
  setClock (3579545);
  setVolume (1.0f);
//  setVolume (0.25f);

  sysInfo_.channelCount = MAX_VOICE;
  sysInfo_.systemID = SoundSystem::SYSTEM_SCC;
  sysInfo_.actualSystemID = SoundSystem::SYSTEM_EMULATION;
}

namespace
{
  static constexpr uint8_t addrMap051649_[] = {
    0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,
    0xa0,0xa0,0x80,0x90,0xb0,0xb0,0xc0,0xc0,
  };
  static constexpr uint8_t addrMap052539_[] = {
    0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,
    0x80,0x90,0xa0,0xa0,0xb0,0xb0,0xc0,0xc0,
  };
}

void
SCC::setValue (int addr, int value)
{
  int upper = addr >> 8;
  int lh = (addr >> 4) & 15;
  int ll = addr & 15;
  int reg;
  if (upper == 0x98)
    {
      // 051649
      reg = addrMap051649_[lh] | ll;
      if ((addr & 0x60) == 0x60)
        register_[0x80 | (addr & 0x1f)] = value;
    }
  else if (upper == 0xb8)
    {
      // 052539
      reg = addrMap052539_[lh] | ll;
    }
  
  if (reg == 0xaf)
    {
      chKeyOn_ |= value;
      chKeyOnTrigger_ |= (~chKeyOn_) & value;
      value &= chNonMute_;
    }
  else if (reg >= 0xaa && reg <= 0xae)
    {
      // keyonとして音量変化も見てみる
      int ch = reg - 0xaa;
      int chBit = 1 << ch;
      if (chKeyOn_ & chBit)
        {
          int prev = register_[reg] & 15;
          int lv = value & 15;
          if (lv > prev)
            chKeyOnTrigger_ |= chBit;
        }
      if (!(chNonMute_ & chBit))
        value = 0;
    }
  
  register_[reg] = value;
}

int
SCC::getValue (int addr)
{
  int upper = addr >> 8;
  int lh = (addr >> 4) & 15;
  int ll = addr & 15;
  if (upper == 0x98)
    {
      // 051649
      return register_[addrMap051649_[lh] | ll];
    }
  else if (upper == 0xb8)
    {
      // 052539
      return register_[addrMap052539_[lh] | ll];
    }
  
  return 0;
}

void
SCC::accumSamples (int32_t* buffer, uint32_t samples)
{
  if (!samples)
    return;

  int chEnable = namedReg_.chEnable & enabledCh_;
  for (int ch = 0; ch < MAX_VOICE; ++ch, chEnable >>= 1)
    {
      if (!(chEnable & 1))
        continue;

      int rate = namedReg_.rates[ch].get ();
      if (rate < 9)
        continue;
      
      auto* waveform = namedReg_.waveforms[ch];
      uint32_t delta = deltaTable_[rate & 0xfff];
      int32_t vol = (namedReg_.volumes[ch] & 15) * volume_;
      uint32_t pos = voicePos_[ch];
      
      uint32_t ct = samples;
      int32_t* dst = buffer;
      do
        {
          pos += delta;
          int32_t v = waveform[pos >> 27] * vol;
          dst[0] += v;
          dst[1] += v;
          dst += 2;
        }
      while (--ct);
      
      voicePos_[ch] = pos;
    }
}

void
SCC::setClock (int clock)
{
  fBase_ = 12.0f*log2(clock*(2.0f/64/440));
  clock_ = clock;
  updateDeltaTable ();
}

void
SCC::setSampleRate (float freq)
{
  sampleRate_ = freq;
  updateDeltaTable ();
}

void
SCC::updateDeltaTable ()
{
  if (sampleRate_ <= 0 || clock_ <= 0)
    return;
  
  float base = (1 << 27) * clock_ / sampleRate_;
  for (int i = 0; i < 0x1000; ++i)
    {
      deltaTable_[i] = int32_t (base / (1 + i) + 0.5f);
    }
}

void
SCC::setVolume (float v)
{
//  volume_ = int (v * (1 << (8 + 2)));
  volume_ = int (v * (1 << (8 + 0)));
}

const SCC::SystemInfo&
SCC::getSystemInfo () const
{
  return sysInfo_;
}

float
SCC::getNote (int ch, int) const
{
  int tp = namedReg_.rates[ch].get();
  if (tp == 0)
    return -1000;
  
  // f = clock*2 / (64 * tp)
  // key = 12 log_2(f/440)
  //     = 12 log_2(clock*2/64/440) - 12 log_2(tp)
  return fBase_ - 12.0f * log2(tp);
}

float
SCC::getVolume (int ch) const
{
  return (namedReg_.volumes[ch] & 15) * (1 / 15.0f);
}

float
SCC::getPan (int ch) const
{
  return 0.0f;
}

int
SCC::getInstrument (int ch) const
{
  return -1;
}

bool
SCC::mute (int ch, bool f)
{
  if (f)
    chNonMute_ &= ~(1 << ch);
  else
    chNonMute_ |= 1 << ch;
  return true;
}

uint32_t
SCC::getKeyOnChannels () const
{
  return chKeyOn_;
}

uint32_t
SCC::getKeyOnTrigger ()
{
//  __disable_irq ();
  uint32_t v = chKeyOnTrigger_;
  chKeyOnTrigger_ = 0;
//  __enable_irq ();
  return v;
}

uint32_t
SCC::getEnabledChannels () const
{
  return chNonMute_ & chEnabled_;
}

const char*
SCC::getStatusString (int ch, char* buf, int n) const
{
  return 0;
}

} /* namespace sound_sys */

/*
 * End of scc_emu.cxx
 */
