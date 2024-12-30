#ifndef BEE4BF92_C833_4F6E_8E9C_70ED87F16A0C_H_
#define BEE4BF92_C833_4F6E_8E9C_70ED87F16A0C_H_

#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <queue.h>

typedef struct {
  enum {
    UROS_TASK_PUB_NONE = 0x00,

    UROS_TASK_PUB_LOW_FREQ_SENSOR_DATA,
    UROS_TASK_PUB_POWER_OFF,
  } topic;

  union {
    struct {
      float battery_voltage;
      bool emo_status;
    } low_freq_sensor_data;

    struct {
      uint8_t _;
    } power_off;
  } data;
} uros_task_pub_queue_data_t;

extern QueueHandle_t uros_task_pub_queue_hd;
extern TaskHandle_t uros_task_hd;

#endif /* BEE4BF92_C833_4F6E_8E9C_70ED87F16A0C_H_ */
