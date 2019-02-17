/*
 * author : Shuichi TAKANO
 * since  : Sun Feb 17 2019 1:45:40
 */
#ifndef _88AA8384_C134_13F8_1630_CF77341533DD
#define _88AA8384_C134_13F8_1630_CF77341533DD

#include <array>
#include <stdint.h>
#include <sys/mutex.h>
#include <vector>

namespace audio
{

class SampleGenerator
{
public:
    virtual ~SampleGenerator() = default;

    virtual void accumSamples(std::array<int32_t, 2>* buffer,
                              uint32_t samples) = 0;
    virtual void setSampleRate(float rate)      = 0;
};

class SampleGeneratorManager
{
    std::vector<SampleGenerator*> generators_;
    sys::Mutex mutex_;

    float sampleRate_ = 44100;

public:
    void add(SampleGenerator* s);
    void remove(SampleGenerator* s);

    void accumSamples(std::array<int32_t, 2>* buffer, uint32_t samples);
    void setSampleRate(float rate);
};

SampleGeneratorManager& getSampleGeneratorManager();

} // namespace audio

#endif /* _88AA8384_C134_13F8_1630_CF77341533DD */
