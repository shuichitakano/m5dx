/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 04 2018 5:34:29
 */

#include "timer.h"
#include "../debug.h"
#include <assert.h>
#include <driver/periph_ctrl.h>
#include <driver/timer.h>
#include <soc/timer_group_struct.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

namespace sys
{

namespace
{

class TimerImpl
{
    timer_group_t timerGrp_ = TIMER_GROUP_0;
    timer_idx_t timerIdx_ = TIMER_0;

    int currentPeriod_ = -1;
    bool currentAutoReload_ = false;

    bool initialized_ = false;
    bool started_ = false;

    std::function<void()> callback_;

    SemaphoreHandle_t semaphore_{};

  public:
    TimerImpl(int grp = 0, int idx = 0)
        : timerGrp_((timer_group_t)(TIMER_GROUP_0 + grp)), timerIdx_((timer_idx_t)(TIMER_0 + idx))
    {
    }

    void setCallback(std::function<void()> &&f) { callback_ = f; }

    void init(int div, bool autoReload)
    {
        if (!initialized_)
        {
            semaphore_ = xSemaphoreCreateBinary();
            assert(semaphore_);

            constexpr int prio = 10;
            auto r =
                xTaskCreate(timerTaskEntry, "timer", 4096, this, prio, nullptr);
            assert(r);
            initialized_ = true;
        }

        timer_config_t config{};
        config.divider = div;
        config.counter_dir = TIMER_COUNT_UP;
        config.counter_en = TIMER_PAUSE;
        config.alarm_en = TIMER_ALARM_EN;
        config.intr_type = TIMER_INTR_LEVEL;
        config.auto_reload = autoReload;
        timer_init(timerGrp_, timerIdx_, &config);

        currentAutoReload_ = autoReload;

        timer_set_counter_value(timerGrp_, timerIdx_, 0);
        timer_isr_register(
            timerGrp_, timerIdx_, timerISR, this, ESP_INTR_FLAG_IRAM, nullptr);
    }

    void setPeriod(uint32_t v)
    {
        if (v != currentPeriod_)
        {
            timer_set_alarm_value(timerGrp_, timerIdx_, v);
            currentPeriod_ = v;
        }
    }

    void setAutoReload(bool f)
    {
        if (f != currentAutoReload_)
        {
            timer_set_auto_reload(timerGrp_,
                                  timerIdx_,
                                  f ? TIMER_AUTORELOAD_EN
                                    : TIMER_AUTORELOAD_DIS);
            currentAutoReload_ = f;
        }
    }

    void start()
    {
        if (started_)
        {
            return;
        }
        timer_start(timerGrp_, timerIdx_);

        timer_enable_intr(timerGrp_, timerIdx_);
        started_ = true;
    }

    void stop()
    {
        if (started_)
        {
            timer_pause(timerGrp_, timerIdx_);
        }
        started_ = false;
    }

    static void IRAM_ATTR timerISR(void *p) { ((TimerImpl *)p)->timerFunc(); }

    void timerFunc()
    {
        auto &tg = timerGrp_ == TIMER_GROUP_0 ? TIMERG0 : TIMERG1;
        auto &timer = tg.hw_timer[timerIdx_];

        xSemaphoreGiveFromISR(semaphore_, nullptr);

        if (timerIdx_ == 0)
        {
            tg.int_clr_timers.t0 = 1;
        }
        else
        {
            tg.int_clr_timers.t1 = 1;
        }
        timer.config.alarm_en = TIMER_ALARM_EN;
    }

    static void timerTaskEntry(void *p) { ((TimerImpl *)p)->timerTask(); }

    void timerTask()
    {
        DBOUT(("Enter timerTask %d:%d\n", timerGrp_, timerIdx_));
        while (1)
        {
            xSemaphoreTake(semaphore_, portMAX_DELAY);
            if (callback_)
            {
                callback_();
            }
        }
    }

    void enableInt() { timer_enable_intr(timerGrp_, timerIdx_); }
    void disableInt() { timer_disable_intr(timerGrp_, timerIdx_); }
};

TimerImpl timer0_;

} // namespace

void initTimer(int baseClock)
{
    auto div = TIMER_BASE_CLK / baseClock;
    DBOUT(("initTimer: baseClock %d, div %d\n", baseClock, div));

    timer0_.init(div, true);
}

void startTimer()
{
    timer0_.start();
}

void stopTimer()
{
    timer0_.stop();
}

void enableTimerInterrupt()
{
    timer0_.enableInt();
}

void disableTimerInterrupt()
{
    timer0_.disableInt();
}

void setTimerPeriod(int v, bool autoUpdate)
{
    timer0_.setPeriod(v);
    timer0_.setAutoReload(autoUpdate);
}

void setTimerCallback(std::function<void()> &&f)
{
    timer0_.setCallback(std::move(f));
}

} // namespace sys
