#include <Adafruit_HMC5883_U.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include "include/gy87.h"

static Adafruit_MPU6050 s_mpu;
static sensors_event_t s_event_accel, s_event_gyro, s_event_temp, s_event_mag;

static Adafruit_HMC5883_Unified s_mag = Adafruit_HMC5883_Unified(12345);

static bool s_init = false;

static bsp_gy87_vec_t accel_offset, gyro_offset;

bool bsp_gy87_init(void) {
  if (s_init == true) {
    return false;
  }
  s_init = true;

  bool ret;
  ret = s_mpu.begin();
  if (ret == false) {
    return false;
  }
  s_mpu.setI2CBypass(true);
  s_mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  s_mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  ret = s_mag.begin();
  if (ret == false) {
    return false;
  }
  return true;
}

bool bsp_gy87_set_offset(void) {
  if (s_init == false) {
    return false;
  }

  bsp_gy87_rtn_data_t data;

  bsp_gy87_vec_t gyro_sum = {0}, accel_sum = {0};
  unsigned long time_stop = millis() + 1000 * 10; // 10초
  uint64_t cnt = 0;
  while (time_stop > millis()) {
    bsp_gy87_get_data(&data);
    accel_sum.x += data.accel.x;
    accel_sum.y += data.accel.y;
    accel_sum.z += data.accel.z;

    gyro_sum.x += data.gyro.x;
    gyro_sum.y += data.gyro.y;
    gyro_sum.z += data.gyro.z;
    cnt++;
  }
  accel_offset.x = accel_sum.x / cnt;
  accel_offset.y = accel_sum.y / cnt;
  accel_offset.z = accel_sum.z / cnt - 9.8;

  gyro_sum.x = gyro_sum.x / cnt;
  gyro_sum.y = gyro_sum.y / cnt;
  gyro_sum.z = gyro_sum.z / cnt;

  return true;
}

bool bsp_gy87_get_data(bsp_gy87_rtn_data_t *data_p_) {
  if (s_init == false) {
    return false;
  }

  if (data_p_ == NULL) {
    return false;
  }

  bool ret = true;
  ret &= s_mpu.getEvent(&s_event_accel, &s_event_gyro, &s_event_temp);
  ret &= s_mag.getEvent(&s_event_mag);
  if (ret == false) {
    return false;
  }

  data_p_->accel.x = s_event_accel.acceleration.x - accel_offset.x;
  data_p_->accel.y = s_event_accel.acceleration.y - accel_offset.y;
  data_p_->accel.z = s_event_accel.acceleration.z - accel_offset.z;

  data_p_->gyro.x = s_event_gyro.gyro.x - gyro_offset.x;
  data_p_->gyro.y = s_event_gyro.gyro.y - gyro_offset.y;
  data_p_->gyro.z = s_event_gyro.gyro.z - gyro_offset.z;

  data_p_->temp = s_event_temp.temperature;

  data_p_->mag.x = s_event_mag.magnetic.x;
  data_p_->mag.y = s_event_mag.magnetic.y;
  data_p_->mag.z = s_event_mag.magnetic.z;

  return true;
}
