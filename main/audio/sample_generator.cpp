/*
 * author : Shuichi TAKANO
 * since  : Sun Feb 17 2019 1:50:23
 */

#include "sample_generator.h"
#include <mutex>

namespace audio
{

namespace
{
SampleGeneratorManager manager_;
}

SampleGeneratorManager&
getSampleGeneratorManager()
{
    return manager_;
}

void
SampleGeneratorManager::add(SampleGenerator* s)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    s->setSampleRate(sampleRate_);
    for (auto p : generators_)
    {
        if (p == s)
        {
            return;
        }
    }
    generators_.push_back(s);
}
void
SampleGeneratorManager::remove(SampleGenerator* s)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    for (auto p = generators_.begin(); p != generators_.end();)
    {
        if (*p == s)
        {
            p = generators_.erase(p);
        }
        else
        {
            ++p;
        }
    }
}

void SampleGeneratorManager::accumSamples(std::array<int32_t, 2>* buffer,
                                          uint32_t samples)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    for (auto p : generators_)
    {
        p->accumSamples(buffer, samples);
    }
}
void
SampleGeneratorManager::setSampleRate(float rate)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    sampleRate_ = rate;
    for (auto p : generators_)
    {
        p->setSampleRate(rate);
    }
}

} // namespace audio
