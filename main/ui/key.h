/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 3:11:36
 */
#ifndef A63C5245_6134_13F9_2F84_90719989A929
#define A63C5245_6134_13F9_2F84_90719989A929

#include <stdint.h>

namespace ui
{

class KeyState
{
    uint32_t current_ = 0;
    uint32_t prev_    = 0;
    // 正論理

    uint32_t startTimeUS_     = 0;
    uint32_t currentTimeUS_   = 0;
    uint32_t lastPulseTimeUS_ = 0;

    bool longPress_   = false;
    bool repeatPulse_ = false;

public:
    void update(bool b0, bool b1, bool b2);
    void updateNegative(bool b0, bool b1, bool b2) { update(!b0, !b1, !b2); }

    bool isPressed(int i) const { return current_ & (1 << i); }
    bool isEdge(int i) const { return (current_ ^ prev_) & (1 << i); }
    bool isPressEdge(int i) const { return isPressed(i) && isEdge(i); }
    bool isReleaseEdge(int i) const { return !isPressed(i) && isEdge(i); }
    bool isLongPress(int i) const { return isPressed(i) && longPress_; }

    // repeat付き
    bool isTrigger(int i) const;
};

} // namespace ui

#endif /* A63C5245_6134_13F9_2F84_90719989A929 */
