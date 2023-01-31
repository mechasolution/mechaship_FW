#ifndef __HW_H__
#define __HW_H__

#include <Arduino.h>
#include <HardwareSerial.h>

// UART Serial2(8, 9, 0, 0);

//--------------micro ROS--------------//
#define HW_SERIAL_MICROROS Serial1 // UART 포트
#define HW_CFG_ROS_DOMAIN_ID 1     // 도메인 아이디 (0~101, 215~232)

//--------------네오픽셀--------------//
#define HW_PIN_NEOPIXEL 6           // 네오픽셀 핀 번호
#define HW_CFG_NEOPIXEL_PIXEL_CNT 8 // 네오픽셀 픽셀 수

//--------------디버깅--------------//
#define HW_PIN_STATUS_LED 25   // 상태 표시 led 핀 번호
#define HW_SERIAL_DEBUG Serial // 디버깅용 시리얼 포트

//--------------배터리--------------//
#define HW_PIN_BATTERY A2 // 배터리 전압 측정용 ADC 포트 번호

//--------------액추에이터--------------//
#define HW_PIN_THROTTLE 11 // 쓰로틀 PWM 핀 번호
#define HW_PIN_KEY 10      // 키 PWM 핀 번호

//--------------GPS--------------//
#define HW_SERIAL_GPS Serial2 // GPS 연결 시리얼 포트

//--------------기타--------------//
#define HW_PIN_RESET 22 // 리셋핀 연결된 GPIO

#endif