/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 5:29:24
 */

#include "key.h"
#include <debug.h>
#include <sys/util.h>

namespace ui
{

namespace
{
// us
constexpr int repeatBegin_    = 500000;
constexpr int repeatInterval_ = 100000;
constexpr int longPressTime_  = 1000000;
} // namespace

void
KeyState::update(bool b0, bool b1, bool b2)
{
    prev_    = current_;
    current_ = ((b0 ? 1 << 0 : 0) | (b1 ? 1 << 1 : 0) | (b2 ? 1 << 2 : 0) | 0);

    repeatPulse_ = false;
    longPress_   = false;

    currentTimeUS_ = sys::micros();
    if (current_)
    {
        if (current_ ^ prev_)
        {
            startTimeUS_     = currentTimeUS_;
            lastPulseTimeUS_ = currentTimeUS_ + repeatBegin_ - repeatInterval_;
        }

        int32_t d = currentTimeUS_ - startTimeUS_;
        //        if (d > repeatBegin_)
        {
            int32_t pd = currentTimeUS_ - lastPulseTimeUS_;
            if (pd >= repeatInterval_)
            {
                repeatPulse_ = true;
                lastPulseTimeUS_ += repeatInterval_;
            }
        }

        if (d > longPressTime_)
        {
            longPress_ = true;
        }
    }
}

bool
KeyState::isTrigger(int i) const
{
    return isPressEdge(i) || (isPressed(i) && repeatPulse_);
}

} // namespace ui
