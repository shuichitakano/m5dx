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
KeyState::update(bool b0, bool b1, bool b2, bool b3, int dial)
{
    dial += dialMod_;
    auto d_4 = dial / 4;
    dialMod_ = dial - d_4 * 4;
    dial_    = d_4;

    prev_    = current_;
    current_ = ((b0 ? 1 << 0 : 0) | (b1 ? 1 << 1 : 0) | (b2 ? 1 << 2 : 0) |
                (b3 ? 1 << 3 : 0) | 0);
    if (current_)
    {
        if (!pressMask_)
        {
            current_ = 0;
        }
    }
    else
    {
        pressMask_ = true;
    }
    edge_ = pressMask_ ? prev_ ^ current_ : 0;
    //    DBOUT(("c:%x p:%x e:%x m:%d\n", current_, prev_, edge_, pressMask_));

    repeatPulse_   = false;
    prevLongPress_ = longPress_;
    longPress_     = false;

    currentTimeUS_ = sys::micros();
    if (current_)
    {
        if (edge_)
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
