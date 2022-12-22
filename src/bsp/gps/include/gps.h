#ifndef __BSP_GPS_H__
#define __BSP_GPS_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <HardwareSerial.h>

#include <Adafruit_GPS.h>

typedef struct {
  bool fix;
  double latitude;
  double longitude;
  double altitude;
} bsp_gps_rtn_data_t;

typedef struct {
  HardwareSerial *serial;
  unsigned long baudrate;
} bsp_gps_config_t;

bool bsp_gps_init(bsp_gps_config_t *config_p_);
bool bsp_gps_loop(void);
bool bsp_gps_get_data(bsp_gps_rtn_data_t *data_p_);

#endif