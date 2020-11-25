/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 21 2018 19:32:54
 */
#ifndef E674E0B2_E133_F008_126E_2B375441C8C2
#define E674E0B2_E133_F008_126E_2B375441C8C2

#include <stdint.h>

namespace target
{

namespace config
{

constexpr int D0 = 1;
constexpr int D1 = 2;
constexpr int D2 = 18;
constexpr int D3 = 19;
constexpr int D4 = 5;
constexpr int D5 = 21;
constexpr int D6 = 22;
constexpr int D7 = 23;

constexpr int CS  = 13;
constexpr int A0  = 12;
constexpr int A1  = 3;
constexpr int CLK = 0;

constexpr int SD_CLK  = 26;
constexpr int SD_DATA = 35;
constexpr int SD_WS   = 36;

constexpr int NEO_PIXEL_DATA = 15;

constexpr int BUTTON_A = 39;
constexpr int BUTTON_B = 38;
constexpr int BUTTON_C = 37;

} // namespace config

void initGPIO();
void setupBus(bool useA1);
void restoreBus(bool useA1);

void startFMClock(uint32_t freq);

void writeBusData(int d);
void setBusIdle();

void negateFMCS();
void assertFMCS();

void setFMA0(int i);
void setFMA1(int i);

bool getButtonA();
bool getButtonB();
bool getButtonC();

void lockBus();
void unlockBus();

void startI2C();
void endI2C();

} // namespace target

#endif /* E674E0B2_E133_F008_126E_2B375441C8C2 */
