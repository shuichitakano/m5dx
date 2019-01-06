/* -*- mode:C++; -*-
 *
 * k054539_emu.cxx
 *
 * author(s) : Shuichi TAKANO
 * since 2015/01/18(Sun) 00:50:57
 *
 * $Id$
 */

/*
 * include
 */

#include <stdio.h>
#include "k054539_emu.h"

#include <math.h>
#include <string.h>
#include <algorithm>

/*
 * code
 */

namespace sound_sys
{

  namespace
    {
      constexpr float maxVol_ = 1.8f;
    }

void
K054539::initialize ()
{
  memset (register_, 0, sizeof (register_));
  memset (voices_, 0, sizeof (voices_));
  memset (ram_, 0, sizeof (ram_));

  rwPtr_ = 0;
  rwPtrTail_ = 0;
  rwTop_ = 0;
  
  romSize_ = 0;
  clock_ = 0;
  sampleRate_ = 0;

  reverbEnabled_ =  true;
  reverbPos_ = 0;

  nextKeyOff_ = 0;
  chKeyOn_ = 0;
  
  setClock (48000);
  setVolume (1.0f);
  setSampleRate (48000.0f);
  rom_.clear ();

  for (int i = 0; i < 256; ++i)
    {
     volumeTable_[i]
       = powf (10.0f,
               (-36.0f * i / 64) / 20.0) / 4.0;
//      printf ("vol table %d:%f %d\n", i, volumeTable_[i], int (volumeTable_[i] * 65536.0f));
    }
  
  for (int i = 0; i < 15; ++i)
    {
      panTable_[i] = sqrtf (i / 14.0f);
//      printf ("pan table %d:%f\n", i, panTable_[i]);
    }
  

  sysInfo_.channelCount = MAX_VOICE;
  sysInfo_.systemID = SoundSystem::SYSTEM_054539;
  sysInfo_.actualSystemID = SoundSystem::SYSTEM_EMULATION;
}

int
K054539::getValue (int addr)
{
  if (addr == REG_DATA_RW)
    {
      if (register_[REG_PCM_RAM_CONTROL] & 0x10)
        {
          int v = 0;
          if (rwTop_)
            v = rwTop_[rwPtr_];
          ++rwPtr_;
          if (rwPtr_ >= rwPtrTail_)
            rwPtr_ = 0;
          return v;
        }
      else
        return 0;
    }
  return register_[addr];
}

void
K054539::setValue (int addr, int value)
{
  if (addr >= (int) sizeof (register_))
    return;
  
  int prev = register_[addr];
  register_[addr] = value;
  
  if (addr < 0x100)
    {
      int ch = addr >> 5;
      int port = addr & 31;

      auto& vra = voiceRegs_.a[ch];
      auto& v = voices_[ch];

      switch (port)
        {
        case 0:
          v.nextDeltaL = vra.pitchL * freqBase_ >> 16;
          break;
          
        case 1:
          v.nextDeltaM = vra.pitchM * freqBase_ >> 8;
          break;
          
        case 2:
          v.nextDeltaH = vra.pitchH * freqBase_;
          break;
          
        case 3:
        case 5:
          if (prev != value)
            {
              updateVolume (v, vra.vol, vra.pan);
              updateReverbVolume (v, vra.vol, vra.reverb);
            }
          break;
          
        case 4:
          if (prev != value)
            updateReverbVolume (v, vra.vol, vra.reverb);
          break;
          
//        case 6:
        case 7:
          {
            v.reverbDelay = ((vra.reverbDelayH * invFreqBase_ >> (16 + 3 - 8)) +
                             (vra.reverbDelayL * invFreqBase_ >> (16 + 3)) +
                             1);
//                             0);
//            printf ("rd %x:%x > %d\n", vra.reverbDelayH, vra.reverbDelayL, v.reverbDelay);
          }
          break;
        }
    }
  else
    {
      switch (addr)
        {
        case REG_KEYON:
          {
            nextKeyOff_ &= ~value;
            chKeyOnTrigger_ |= (chKeyOn_ ^ value) & value;
            chKeyOn_ |= value;
            
            for (int ch = 0; ch < 8; ++ch)
              {
                if (!(value & (1 << ch)))
                  continue;

                auto& vra = voiceRegs_.a[ch];
                auto& v = voices_[ch];
                
                uint32_t start = ((vra.startL << 0) |
                                  (vra.startM << 8) |
                                  (vra.startH << 16));
                
                uint32_t loop  = ((vra.loopL << 0) |
                                  (vra.loopM << 8) |
                                  (vra.loopH << 16));

//                printf ("ch%d: start %x, loop %x\n", ch, start, loop);
                v.nextSampleLoopOfs = loop - start;
                v.nextSample = rom_.find (start & romMask_);
              }
            
            register_[REG_ACTIVE_CH] |= value;
          }
          break;
          
        case REG_KEYOFF:
          register_[REG_ACTIVE_CH] &= ~value;
          nextKeyOff_ |= value;
          chKeyOn_ &= ~value;
          break;
          
        case REG_DATA_RW:
          if (register_[REG_ROM_RAM_SEL] == 0x80 && rwTop_)
            rwTop_[rwPtr_] = value;
          ++rwPtr_;
          if (rwPtr_ >= rwPtrTail_)
            rwPtr_ = 0;
          break;
          
        case REG_ROM_RAM_SEL:
          rwTop_ = value == 0x80 ? ram_ : const_cast<uint8_t*>(rom_.find (0x20000 * value));
          rwPtr_ = 0;
          rwPtrTail_ = value == 0x80 ? 0x4000 : 0x20000;
          break;
          
        default:
          break;
        };
    }
}

inline bool
K054539::Voice::update8BitPCM ()
{
  frac += delta;
  while (frac & ~0xffff)
    {
      frac -= 0x10000;
      pos += posInc;
      prev = val;
      uint_fast8_t smp = sample[pos];
      if (smp == 0x80)
        {
          if (loop)
            {
              pos = sampleLoopOfs;
              smp = sample[pos];
            }
          else
            {
              keyon = false;
              return false;
            }
        }
      
      val = (int8_t) smp << 8;
    }
  return true;
}

inline bool
K054539::Voice::update16BitPCM ()
{
  frac += delta;
  while (frac & ~0xffff)
    {
      frac -= 0x10000;
      pos += posInc;
      prev = val;
      val = (int16_t)(sample[pos] | (sample[pos + 1] << 8));
      if (val == -32768)
        {
          if (loop)
            {
              pos = sampleLoopOfs;
              val = (int16_t)(sample[pos] | (sample[pos + 1] << 8));
            }
          else
            {
              keyon = false;
              return false;
            }
        }
    }
  return true;
}

namespace
{
  constexpr int16_t dpcmTable_[16] =
    {
      0<<8, 1<<8, 4<<8, 9<<8, 16<<8, 25<<8, 36<<8, 49<<8,
      -(64<<8), -(49<<8), -(36<<8), -(25<<8),
      -(16<<8), -(9<<8), -(4<<8), -(1<<8),
    };
}

inline bool
K054539::Voice::update4BitDPCM ()
{
  frac += delta;
  while (frac & ~0xffff)
    {
      frac -= 0x10000;
      pos += posInc;
      prev = val;
      uint8_t smp = sample[pos >> 1];
      if (smp == 0x88)
        {
          if (loop)
            {
              pos = sampleLoopOfs;
              smp = sample[pos >> 1];
            }
          else
            {
              keyon = false;
              return false;
            }
        }
      int dv = (pos & 1) ? (smp >> 4) : (smp & 15);
#if 0
      val = std::min (32767,
                        std::max (-32768,
                                  (int)(prev + dpcmTable_[dv])));
#else
      val = prev + dpcmTable_[dv];
#endif
    }
  return true;
}


void
K054539::accumSamples (int32_t* buffer, uint32_t samples)
{
  if (!samples)
    return;
  
  if (!(register_[REG_PCM_RAM_CONTROL] & 1))
    return;
  
  auto reverb = (int16_t*) ram_;
  
  auto nextKeyOff = nextKeyOff_;
  nextKeyOff_ = 0;
  
  for (int ch = 0; ch < MAX_VOICE; ++ch)
//  int ch = 0; for (int ii = 0; ii < 1; ++ii)
    {
      auto& v = voices_[ch];
      if (v.mute || nextKeyOff & (1 << ch))
        {
          v.keyon = false;
          v.nextSample = 0;
        }
      else if (v.nextSample)
        {
//          auto& vra = voiceRegs_.a[ch];
          auto& vrb = voiceRegs_.b[ch];
          
          v.keyon = true;
          v.frac = -1;
          v.val = 0;
          v.prev = 0;
          v.loop = vrb.loop & 1;
          v.mode = Mode ((vrb.mode >> 2) & 3);
          v.reverse = vrb.mode & 0x20;
          v.posInc = v.reverse ? -1 : 1;
//          printf ("ch%d: rev%d loop%d mode%d\n", ch, v.reverse, v.loop, v.mode);

          v.sample = v.nextSample;
          v.sampleLoopOfs = v.nextSampleLoopOfs;
          
          switch (v.mode)
            {
            case MODE_16BIT_PCM:
              v.posInc <<= 1;
              break;
              
            case MODE_4BIT_DPCM:
              v.sampleLoopOfs <<= 1;
              break;
              
            default:
              break;
            }
          
          v.pos = -v.posInc;
          v.nextSample = 0;
        }
      
      v.delta = v.nextDeltaH + v.nextDeltaM + v.nextDeltaL;
    }
  
#if 1
  for (int ch = 0; ch < MAX_VOICE; ++ch)
    {
      int32_t* dst = buffer;

      auto& v = voices_[ch];
      
      auto pos = v.pos;
      auto frac = v.frac;
      auto val = v.val;
      auto prev = v.prev;
      auto delta = v.delta;
      auto posInc = v.posInc;
      auto volL = v.volL;
      auto volR = v.volR;
      const auto* sample = v.sample;

      if (v.keyon)
        {
          int ct = samples;
          switch (v.mode)
            {
            case MODE_8BIT_PCM:
              do
                {
                  frac += delta;
                  while (frac & ~0xffff)
                    {
                      frac -= 0x10000;
                      pos += posInc;
                      prev = val;
                      uint_fast8_t smp = sample[pos];
                      if (smp == 0x80)
                        {
                          if (v.loop)
                            {
                              pos = v.sampleLoopOfs;
                              smp = sample[pos];
                            }
                          else
                            {
                              v.keyon = false;
                              val = 0;
                              break;
                            }
                        }

                      val = (int8_t) smp << 8;
                    }

                  int32_t d = val - prev;
                  int32_t t = (d * (frac >> 8) >> 8) + prev;

                  dst[0] += t * volL;
                  dst[1] += t * volR;
                  dst += 2;
                }
              while (--ct);
              break;

            case MODE_16BIT_PCM:
              do
                {
                  frac += delta;
                  while (frac & ~0xffff)
                    {
                      frac -= 0x10000;
                      pos += posInc;
                      prev = val;
                      val = (int16_t)(sample[pos] | (sample[pos + 1] << 8));
                      if (val == -32768)
                        {
                          if (v.loop)
                            {
                              pos = v.sampleLoopOfs;
                              val = (int16_t)(sample[pos] | (sample[pos + 1] << 8));
                            }
                          else
                            {
                              v.keyon = false;
                              val = 0;
                              break;
                            }
                        }
                    }

                  int32_t d = val - prev;
                  int32_t t = (d * (frac >> 8) >> 8) + prev;

                  dst[0] += t * volL;
                  dst[1] += t * volR;
                  dst += 2;
                }
              while (--ct);
              break;

            case MODE_4BIT_DPCM:
              do
                {
                  frac += delta;
                  while (frac & ~0xffff)
                    {
                      frac -= 0x10000;
                      pos += posInc;
                      prev = val;
                      uint8_t smp = sample[pos >> 1];
                      if (smp == 0x88)
                        {
                          if (v.loop)
                            {
                              pos = v.sampleLoopOfs;
                              smp = sample[pos >> 1];
                            }
                          else
                            {
                              v.keyon = false;
                              val = 0;
                              break;
                            }
                        }
                      int dv = (pos & 1) ? (smp >> 4) : (smp & 15);
#if 1
                      val = std::min (32767,
                                      std::max (-32768,
                                                (int)(prev + dpcmTable_[dv])));
#else
                      val = prev + dpcmTable_[dv];
#endif
                    }
                  
                  int32_t d = val - prev;
                  int32_t t = (d * (frac >> 8) >> 8) + prev;

                  dst[0] += t * volL;
                  dst[1] += t * volR;
                  dst += 2;
                }
              while (--ct);
              break;
            }
          
        }

      v.pos = pos;
      v.frac = frac;
      v.val = val;
      v.prev = prev;
    }
#else
  do
    {
      int32_t l, r;
      l = r = reverb[reverbPos_] * volume_;
      reverb[reverbPos_] = 0;
      
      for (int ch = 0; ch < MAX_VOICE; ++ch)
//      for (int ch = 0; ch < 1; ++ch)
        {
          auto& v = voices_[ch];
          if (v.keyon)
            {
              bool result = false;
              switch (v.mode)
                {
                case MODE_8BIT_PCM:
                  result = v.update8BitPCM ();
                  break;
                  
                case MODE_16BIT_PCM:
                  result = v.update16BitPCM ();
                  break;
                  
                case MODE_4BIT_DPCM:
                  result = v.update4BitDPCM ();
                  break;
                }
              
              if (result)
                {
                  int32_t d = v.val - v.prev;
#if 1
                  // Little Endien
                  int32_t t = (d * *((uint8_t*)&v.frac + 1) >> 8) + v.prev;
#else
                  int32_t t = (d * (v.frac >> 8) >> 8) + v.prev;
#endif
                  
                  l += t * v.volL;
                  r += t * v.volR;
                  reverb[(v.reverbDelay + reverbPos_) & 0x1fff]
                    = t * v.volReverb >> 16;
                }
            }
        }
      
      buffer[0] += l;
      buffer[1] += r;
      buffer += 2;
      reverbPos_ = (reverbPos_ + 1) & 0x1fff;
    }
  while (--samples);
#endif
}




void
K054539::updateVolume (Voice& v,
                       uint_fast8_t vol, uint_fast8_t pan)
{
  if (pan >= 0x81 && pan <= 0x8f)
    pan -= 0x81;
  else if (pan >= 0x11 && pan <= 0x1f)
    pan -= 0x11;
  else
    pan = 0x18 - 0x11;

  float fv = volumeTable_[vol];

  float volL = std::min (maxVol_, fv * panTable_[pan]);
  float volR = std::min (maxVol_, fv * panTable_[0xe - pan]);
  v.volL = int (volL * volume_);
  v.volR = int (volR * volume_);
//  printf ("ch%d: vol (%f:%d, %f:%d)\n", &v - voices_, volL, v.volL, volR, v.volR);
}

void
K054539::updateReverbVolume (Voice& v,
                             uint_fast8_t vol, uint_fast8_t reverb)
{
  int rv = std::min (255, int(reverb + vol));
  float volReverb = std::min (maxVol_, volumeTable_[rv] * 0.5f);
  v.volReverb = reverbEnabled_ ? int (volReverb * 65536.0f) : 0;
//  printf ("ch%d: reverb vol %f\n", &v - voices_, v.volReverb);
}

void
K054539::setClock (int clock)
{
  clock_ = clock;
  updateFreqBase ();
}

void
K054539::setSampleRate (float freq)
{
  sampleRate_ = freq;
  updateFreqBase ();
}

void
K054539::updateFreqBase ()
{
  freqBase_ = uint32_t (clock_* 65536.0f / sampleRate_ );
  invFreqBase_ = uint32_t (sampleRate_ * 65536.0f / clock_);
//  printf ("clock %f, samplerate %f, freqBase %f\n", clock_, sampleRate_, freqBase_ / 65536.0f);
}

void
K054539::setVolume (float v)
{
  volume_ = int (v * (1 << 8));
}

void
K054539::enableReverb (bool f)
{
  reverbEnabled_ = f;
}

void
K054539::addROMBlock (uint32_t addr, uint32_t romSize,
                      SimpleAutoBuffer&& data,
                      uint32_t size)
{
  rom_.addEntry (addr, std::move (data), size);

  if (romSize != romSize_)
    {
      romSize_ = romSize;
      
      romMask_ = 1;
      for (; romMask_ < romSize; romMask_ <<= 1);
      --romMask_;
      
//      printf ("romSize = %x, romMask = %x\n", romSize, romMask_);
    }
}

const K054539::SystemInfo&
K054539::getSystemInfo () const
{
  return sysInfo_;
}

float
K054539::getNote (int ch, int) const
{
  auto delta = voices_[ch].delta;
  // log(delta / 2^16)/log(2) * 12
  // (log(delta)*12/log(2) - 16*12)
  return logf(delta) * 17.31234f - 16 * 12;
}

float
K054539::getVolume (int ch) const
{
  auto& vra = voiceRegs_.a[ch];
  return 1 - vra.vol * (1 / 64.0f);
}

float
K054539::getPan (int ch) const
{
  auto& v = voices_[ch];
  float d = v.volL + v.volR;
  if (d == 0)
    return 0.0f;
  return v.volL/d * 2 - 1;
}

int
K054539::getInstrument (int ch) const
{
  return -1;
}

bool
K054539::mute (int ch, bool f)
{
  voices_[ch].mute = f;
  return true;
}

uint32_t
K054539::getKeyOnChannels () const
{
  return chKeyOn_;
}

uint32_t
K054539::getKeyOnTrigger ()
{
//  __disable_irq ();
  uint32_t v = chKeyOnTrigger_;
  chKeyOnTrigger_ = 0;
//  __enable_irq ();
  return v;
}

uint32_t
K054539::getEnabledChannels () const
{
  uint32_t r = 0;
  uint32_t m = 1;
  for (int i = 0; i < MAX_VOICE; ++i, m <<= 1)
    if (!voices_[i].mute)
      r |= m;
  return r;
}

const char*
K054539::getStatusString (int ch, char* buf, int n) const
{
  const char* mode[] = { " 8PCM", "16PCM", " DPCM" };
  auto& v = voices_[ch];
  snprintf (buf, n, "%s LP%d REV%d $%08X",
            mode[v.mode],
            v.loop, v.reverse, (intptr_t)(v.sample + v.pos));
  return buf;
}


} /* namespace sound_sys */

/*
 * End of k054539_emu.cxx
 */
