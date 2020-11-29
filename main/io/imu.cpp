/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 29 2020 17:14:27
 */

#include "imu.h"
#include "../debug.h"
#include <utility/MPU9250.h>

namespace io
{

namespace
{
MPU9250 mpu9250_;
}

void
IMU::initialize()
{
    mpu9250_.initMPU9250();
    mpu9250_.calibrateMPU9250(mpu9250_.gyroBias, mpu9250_.accelBias);
    mpu9250_.initAK8963(mpu9250_.magCalibration);

    DBOUT(("imu accel bias = %f, %f, %f\n",
           mpu9250_.accelBias[0],
           mpu9250_.accelBias[1],
           mpu9250_.accelBias[2]));
}

void
IMU::update(bool accel, bool gyro, bool mag, bool temp)
{
    if (mpu9250_.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
    {
        if (accel)
        {
            mpu9250_.readAccelData(mpu9250_.accelCount);
            mpu9250_.getAres();
            auto s        = mpu9250_.aRes;
            const auto& b = mpu9250_.accelBias;

            accel_[0] = mpu9250_.accelCount[0] * s + b[0];
            accel_[1] = mpu9250_.accelCount[1] * s + b[1];
            accel_[2] = mpu9250_.accelCount[2] * s + b[2];
        }
        if (gyro)
        {
            mpu9250_.readGyroData(mpu9250_.gyroCount);
            mpu9250_.getGres();
            auto s = mpu9250_.gRes;

            gyro_[0] = mpu9250_.gyroCount[0] * s;
            gyro_[1] = mpu9250_.gyroCount[1] * s;
            gyro_[2] = mpu9250_.gyroCount[2] * s;
        }
        if (mag)
        {
            auto& mc = mpu9250_.magCount;
            mpu9250_.readMagData(mc);
            mpu9250_.getMres();
            auto s           = mpu9250_.mRes;
            const auto& cal  = mpu9250_.magCalibration;
            const auto& bias = mpu9250_.magbias;

            mag_[0] = mc[0] * s * cal[0] - bias[0];
            mag_[1] = mc[1] * s * cal[1] - bias[1];
            mag_[2] = mc[2] * s * cal[2] - bias[2];
        }
        if (temp)
        {
            auto ct = mpu9250_.readTempData();
            temp_   = ct / 333.87 + 21.0;
        }
    }
}

} // namespace io
