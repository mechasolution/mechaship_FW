#include "include/gps.h"

#include <Arduino.h>

static Adafruit_GPS *s_gps_p;
static uint8_t s_status_led_pin;
static bool s_init;

bool bsp_gps_init(bsp_gps_config_t *config_p_) {
  if (s_init == true) {
    return false;
  }
  s_init = true;
  s_status_led_pin = config_p_->pin_status_led;
  pinMode(s_status_led_pin, OUTPUT);

  s_gps_p = new Adafruit_GPS(config_p_->serial);

  s_gps_p->begin(config_p_->baudrate);
  s_gps_p->sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  s_gps_p->sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  s_gps_p->sendCommand(PGCMD_ANTENNA);

  s_gps_p->latitudeDegrees = s_gps_p->longitudeDegrees = 0;

  return true;
}

bool bsp_gps_loop(void) {
  if (s_init == false) {
    return false;
  }

  s_gps_p->read();
  if (s_gps_p->newNMEAreceived()) {
    if (!s_gps_p->parse(s_gps_p->lastNMEA())) {
      return true;
    }
  }
  return true;
}

bool bsp_gps_get_data(bsp_gps_rtn_data_t *data_p_) {
  if (s_init == false) {
    return false;
  }

  if (data_p_ == NULL) {
    return false;
  }

  data_p_->fix = s_gps_p->fix;
  data_p_->latitude = s_gps_p->latitudeDegrees;
  data_p_->longitude = s_gps_p->longitudeDegrees;
  data_p_->altitude = s_gps_p->altitude;

  digitalWrite(s_status_led_pin, (data_p_->fix == true) ? HIGH : LOW);

  return true;
}