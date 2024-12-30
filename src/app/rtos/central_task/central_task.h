#ifndef CCAB0028_AB54_4F30_A5B6_BD5FC540BFDA_H_
#define CCAB0028_AB54_4F30_A5B6_BD5FC540BFDA_H_

#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <queue.h>

typedef struct {
  enum {
    CENTRAL_TASK_EVENT_NONE,

    CENTRAL_TASK_EVENT_POWER_OFF,
    CENTRAL_TASK_EVENT_UROS_CONNECTION,
    CENTRAL_TASK_EVENT_LOW_POWER,

  } event_type;

  union {
    struct {
      uint8_t _;
    } power_off;

    struct {
      bool status;
    } uros_connection;

    struct {
      float voltage;
    } low_power;
  } data;
} central_task_queue_data_t;

extern QueueHandle_t central_task_queue_hd;
extern TaskHandle_t central_task_hd;

#endif /* CCAB0028_AB54_4F30_A5B6_BD5FC540BFDA_H_ */
