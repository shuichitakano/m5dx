/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 04 2018 5:23:10
 */
#ifndef _23BCBED8_B133_F06C_4FD2_F79B30C9E6B7
#define _23BCBED8_B133_F06C_4FD2_F79B30C9E6B7

#include <functional>
#include <stdint.h>

namespace sys
{

void initTimer(int baseClock);

void startTimer();
void stopTimer();

void enableTimerInterrupt();
void disableTimerInterrupt();

void setTimerPeriod(int v, bool autoUpdate);
void setTimerCallback(std::function<void()>&& func);

} // namespace sys

#endif /* _23BCBED8_B133_F06C_4FD2_F79B30C9E6B7 */
