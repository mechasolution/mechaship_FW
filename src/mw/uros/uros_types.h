#ifndef D7E166E9_E1CA_489F_8930_EC8E9B6F1294_H_
#define D7E166E9_E1CA_489F_8930_EC8E9B6F1294_H_

#include <stdbool.h>
#include <stdint.h>

// -------------------------------------------------------
// Topic - Publisher
typedef enum {
  UROS_PUB_NONE = 0x00,

  UROS_PUB_BATTERY_VOLTAGE,
  UROS_PUB_EMO_STATUS,

  UROS_PUB_MAX,
} uros_pub_data_flag_t;

typedef union {
  struct { // UROS_PUB_BATTERY_VOLTAGE
    float value;
  } battery_voltage;

  struct { // UROS_PUB_EMO_STATUS
    bool value;
  } emo_status;
} uros_pub_data_t;

// -------------------------------------------------------
// Topic - Subscriber
typedef enum {
  UROS_SUB_NONE = 0x00,

  UROS_SUB_KEY,
  UROS_SUB_THROTTLE,
  UROS_SUB_TONE,
  UROS_SUB_RGBW_LED,
  UROS_SUB_IP_ADDR,

  UROS_SUB_MAX,
} uros_sub_data_flag_t;

typedef union {
  struct { // UROS_SUB_KEY
    float degree;
  } key_degree;

  struct { // UROS_SUB_THROTTLE
    float percentage;
  } throttle_percentage;

  struct { // UROS_SUB_TONE
    uint16_t hz;
    uint16_t duration_ms;
  } tone;

  struct { // UROS_SUB_RGBW_LED
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;
  } rgbw_led;

  struct { // UROS_SUB_IP_ADDR
    uint32_t value;
  } ip_addr;
} uros_sub_data_t;

// -------------------------------------------------------
// Service - Server
typedef enum {
  UROS_SRV_NONE = 0x00,

  UROS_SRV_ACTUATOR_ENABLE,
  UROS_SRV_ACTUATOR_DISABLE,
  UROS_SRV_TONE,

  UROS_SRV_MAX,
} uros_srv_req_flag_t;

typedef union {
  struct {                // UROS_SRV_ACTUATOR_ENABLE
    float key_min_degree; // 키 최소 각도
    float key_max_degree; // 키 최대 각도

    uint16_t key_pulse_0_degree;   // 키 최소 각도 펄스
    uint16_t key_pulse_180_degree; // 키 최대 각도 펄스

    uint16_t thruster_pulse_0_percentage;   // ESC 중립 펄스
    uint16_t thruster_pulse_100_percentage; // ESC 최대속도 펄스
  } actuator_enable;

  struct { // UROS_SRV_ACTUATOR_DISABLE
    ;
  } actuator_disable;

  struct { // UROS_SRV_TONE
    uint16_t hz;
    uint16_t duration_ms;
  } tone;
} uros_srv_req_t;

typedef union {
  struct { // UROS_SRV_ACTUATOR_ENABLE
    bool status;
  } actuator_enable;

  struct { // UROS_SRV_ACTUATOR_DISABLE
    bool status;
  } actuator_disable;

  struct { // UROS_SRV_TONE
    bool status;
  } tone;
} uros_srv_res_t;

// -------------------------------------------------------
// Action - Server
typedef enum {
  UROS_ACTION_NONE = 0x00,

  UROS_ACTION_THRUSTER,
  UROS_ACTION_KEY,

  UROS_ACTION_MAX,
} uros_action_flag_t;

typedef union {
  struct {
    float target_percentage;
    float percentages_per_second;
  } thruster;

  struct {
    float target_degree;
    float degrees_per_second;
  } key;
} uros_action_goal_t;

typedef union {
  struct {
    float current_percentage;
  } thruster;

  struct {
    float current_degree;
  } key;
} uros_action_feedback_t;

typedef union {
  struct {
    bool status;
  } thruster;

  struct {
    bool status;
  } key;
} uros_action_res_t;

typedef enum {
  UROS_ACTION_WORKER_STATUS_NONE = 0x00,

  UROS_ACTION_WORKER_STATUS_ERROR,
  UROS_ACTION_WORKER_STATUS_FINISH,
  UROS_ACTION_WORKER_STATUS_WORKING,

  UROS_ACTION_WORKER_STATUS_MAX,
} uros_action_worker_status_t;

typedef void (*uros_sub_callback_t)(const uros_sub_data_flag_t data_flag, const uros_sub_data_t *data);
typedef void (*uros_srv_callback_t)(const uros_srv_req_flag_t req_flag, const uros_srv_req_t *req, uros_srv_res_t *res);
typedef bool (*uros_action_goal_callback_t)(const uros_action_flag_t action_flag, const uros_action_goal_t *goal);
typedef uros_action_worker_status_t (*uros_action_worker_callback_t)(const uros_action_flag_t action_flag, const uros_action_goal_t *goal, uros_action_feedback_t *fb, uros_action_res_t *res);
typedef void (*uros_action_cancel_callback_t)(const uros_action_flag_t action_flag);

#endif /* D7E166E9_E1CA_489F_8930_EC8E9B6F1294_H_ */
