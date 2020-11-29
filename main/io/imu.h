/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 29 2020 17:10:05
 */
#ifndef A4BAAF96_5134_3E8C_104E_D2E2AB355814
#define A4BAAF96_5134_3E8C_104E_D2E2AB355814

#include <array>
#include <stdint.h>

namespace io
{

class IMU
{
public:
    void initialize();
    void update(bool accel, bool gyro, bool mag, bool temp);

    const std::array<float, 3>& getAccel() const { return accel_; }
    const std::array<float, 3>& getGyro() const { return gyro_; }
    const std::array<float, 3>& getMag() const { return mag_; }
    float getTemperature() const { return temp_; }

private:
    std::array<float, 3> accel_;
    std::array<float, 3> gyro_;
    std::array<float, 3> mag_;
    float temp_;
};
} // namespace io

#endif /* A4BAAF96_5134_3E8C_104E_D2E2AB355814 */
