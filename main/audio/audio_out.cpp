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
#include <util/simple_ring_buffer.h>

namespace audio
{

struct AudioOutDriverManager::Impl
{
    static constexpr size_t UNIT_SAMPLE_COUNT =
        AudioOutDriverManager::getUnitSampleCount();
    static constexpr size_t HISTORY_SAMPLE_COUNT = 1024;
    static constexpr size_t DEFAULT_SAMPLE_RATE =
        AudioOutDriverManager::getSampleRate();

    using Sample            = AudioOutDriverManager::Sample;
    using HistorySample     = AudioOutDriverManager::HistorySample;
    using HistoryRingBuffer = AudioOutDriverManager::HistoryRingBuffer;

    Sample buffer_[UNIT_SAMPLE_COUNT];

    AudioOutDriver* driver_{};
    AudioStreamOut* stream_{};

    sys::Mutex mutex_;
    TaskHandle_t taskHandle_{};
    EventGroupHandle_t eventGroupHandle_{};

    HistorySample historyBuffer_[HISTORY_SAMPLE_COUNT]{};
    HistoryRingBuffer historyRing_{historyBuffer_, HISTORY_SAMPLE_COUNT};
    sys::Mutex historyMutex_;

public:
    void start()
    {
        eventGroupHandle_ = xEventGroupCreate();
        assert(eventGroupHandle_);

        static constexpr int prio = 21;
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
            //            if (stream_ && (!driver_ ||
            //            driver_->isDriverUseUpdate()))
            if (stream_ && driver_ && driver_->isDriverUseUpdate())
            {
                auto n = generateSamples(UNIT_SAMPLE_COUNT);
                if (driver_)
                {
                    driver_->onUpdate(buffer_, n);
                }
                // todo:
                // !driver_かつFM音源からの読み出しがなくても固まらないようにする

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
            stream_->onUpdateAudioStream(buffer_,
                                         n,
                                         driver_ ? driver_->getSampleRate()
                                                 : DEFAULT_SAMPLE_RATE);
        }
        else
        {
            memset(buffer_, 0, sizeof(buffer_));
        }
        pushHistorySamples(buffer_, n);
        return n;
    }

    void pushHistorySamples(const Sample* s, size_t n)
    {
        std::lock_guard<sys::Mutex> lock(historyMutex_);

        auto p   = historyRing_.getBufferTop();
        auto pos = historyRing_.getCurrentPos();

        auto update = [&](auto* dst, const auto* src, size_t n) {
            while (n)
            {
                (*dst)[0] = (*src)[0] >> 8;
                (*dst)[1] = (*src)[1] >> 8;
                ++dst;
                ++src;
                --n;
            }
        };
        auto n1 = std::min<int>(n, historyRing_.getMask() + 1 - pos);
        update(p + pos, s, n1);
        auto n2 = n - n1;
        if (n2)
        {
            assert(n2 <= HISTORY_SAMPLE_COUNT);
            update(p, s + n1, n2);
        }
        historyRing_.advancePointer(n);
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

const AudioOutDriverManager::Sample*
AudioOutDriverManager::getSampleBuffer()
{
    return pimpl_->buffer_;
}

const AudioOutDriverManager::HistoryRingBuffer&
AudioOutDriverManager::getHistoryBuffer() const
{
    return pimpl_->historyRing_;
}

void
AudioOutDriverManager::lockHistoryBuffer()
{
    pimpl_->historyMutex_.lock();
}

void
AudioOutDriverManager::unlockHistoryBuffer()
{
    pimpl_->historyMutex_.unlock();
}

AudioOutDriverManager&
AudioOutDriverManager::instance()
{
    static AudioOutDriverManager inst;
    return inst;
}

} // namespace audio
