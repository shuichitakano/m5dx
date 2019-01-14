/*
 * author : Shuichi TAKANO
 * since  : Sun Jan 13 2019 23:4:42
 */

#include "audio_out.h"
#include <algorithm>
#include <assert.h>
#include <debug.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <mutex>
#include <string.h>
#include <sys/mutex.h>

namespace audio
{

struct AudioOutDriverManager::Impl
{
    static constexpr size_t UNIT_SAMPLE_COUNT =
        AudioOutDriverManager::getUnitSampleCount();

    std::array<int32_t, 2> buffer_[UNIT_SAMPLE_COUNT];

    AudioOutDriver* driver_{};
    AudioStreamOut* stream_{};

    sys::Mutex mutex_;
    TaskHandle_t taskHandle_{};
    EventGroupHandle_t eventGroupHandle_{};

public:
    void start()
    {
        eventGroupHandle_ = xEventGroupCreate();
        assert(eventGroupHandle_);

        static constexpr int prio = 5;
        xTaskCreate([](void* p) { ((Impl*)p)->task(); },
                    "AudioOut",
                    2048,
                    this,
                    prio,
                    &taskHandle_);
    }

    void task()
    {
        DBOUT(("Start AudioOutDriverManager task.\n"));
        mutex_.lock();
        while (1)
        {
            if (stream_ && driver_ && driver_->isDriverUseUpdate())
            {
                driver_->onUpdate(buffer_, generateSamples(UNIT_SAMPLE_COUNT));

                mutex_.unlock();
                mutex_.lock();
            }
            else
            {
                mutex_.unlock();

                xEventGroupWaitBits(eventGroupHandle_,
                                    1,
                                    pdTRUE /* clear */,
                                    pdFALSE /* wait for all bit */,
                                    portMAX_DELAY);

                mutex_.lock();
            }
        };
        mutex_.unlock();
    }

    void signal() { xEventGroupSetBits(eventGroupHandle_, 1); }

    size_t generateSamples(size_t n)
    {
        n = std::min(n, UNIT_SAMPLE_COUNT);
        if (stream_)
        {
            stream_->onUpdateAudioStream(buffer_, n, driver_->getSampleRate());
        }
        else
        {
            memset(buffer_, 0, sizeof(buffer_));
        }
        return n;
    }

    bool lock(const AudioOutDriver* d)
    {
        mutex_.lock();
        if (d == driver_)
        {
            return true;
        }
        mutex_.unlock();
        return false;
    }
};

AudioOutDriverManager::AudioOutDriverManager()
    : pimpl_(new Impl)
{
}

AudioOutDriverManager::~AudioOutDriverManager() = default;

void
AudioOutDriverManager::start()
{
    pimpl_->start();
}

void
AudioOutDriverManager::setAudioStreamOut(AudioStreamOut* s)
{
    std::lock_guard<sys::Mutex> lock(pimpl_->mutex_);
    pimpl_->stream_ = s;
    pimpl_->signal();
}

void
AudioOutDriverManager::setDriver(AudioOutDriver* d)
{
    std::lock_guard<sys::Mutex> lock(pimpl_->mutex_);
    if (d == pimpl_->driver_)
    {
        return;
    }

    if (auto pre = pimpl_->driver_)
    {
        pre->onDetach();
    }

    pimpl_->driver_ = d;

    if (d)
    {
        d->onAttach();
    }

    pimpl_->signal();
}

bool
AudioOutDriverManager::lock(const AudioOutDriver* d)
{
    return pimpl_->lock(d);
}

void
AudioOutDriverManager::unlock()
{
    pimpl_->mutex_.unlock();
}

size_t
AudioOutDriverManager::generateSamples(size_t n)
{
    return pimpl_->generateSamples(n);
}

const std::array<int32_t, 2>*
AudioOutDriverManager::getSampleBuffer()
{
    return pimpl_->buffer_;
}

AudioOutDriverManager&
AudioOutDriverManager::instance()
{
    static AudioOutDriverManager inst;
    return inst;
}

} // namespace audio
