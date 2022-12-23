#ifndef __BSP_GY87_H__
#define __BSP_GY87_H__

#include <stdio.h>

// typedef struct {
//   ;
// } bsp_gy87_conf_t;

typedef struct {
  double x;
  double y;
  double z;
} bsp_gy87_vec_t;

typedef struct {
  float temp;           // 센서 온도
  bsp_gy87_vec_t accel; // 가속도
  bsp_gy87_vec_t gyro;  // 자이로
  bsp_gy87_vec_t mag;   // 지자기 (uT)
} bsp_gy87_rtn_data_t;

bool bsp_gy87_init(void);
bool bsp_gy87_set_offset_auto(void);
bool bsp_gy87_set_offset(bsp_gy87_vec_t *offset_accel_, bsp_gy87_vec_t *offset_gyro_);

bool bsp_gy87_get_data(bsp_gy87_rtn_data_t *data_p_);
bool bsp_gy87_get_offset(bsp_gy87_vec_t *offset_accel_, bsp_gy87_vec_t *offset_gyro_);

#endif