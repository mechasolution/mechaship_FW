#ifndef DF049485_2815_48B5_AF4E_2156832B62E6_H_
#define DF049485_2815_48B5_AF4E_2156832B62E6_H_

#include <stdbool.h>

#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>

#include "uros_types.h"

#include "driver/log/log.h"

#define RCCHECK(fn)                \
  {                                \
    rcl_ret_t temp_rc = fn;        \
    if ((temp_rc != RCL_RET_OK)) { \
      return false;                \
    }                              \
  }

#define RCCHECK_LOG(TAG, fn)                                                                    \
  {                                                                                             \
    rcl_ret_t temp_rc = fn;                                                                     \
    if ((temp_rc != RCL_RET_OK)) {                                                              \
      log_error(TAG, "ERROR!! ret: %d, %s, %s, %d", temp_rc, __FILE__, __FUNCTION__, __LINE__); \
      return false;                                                                             \
    }                                                                                           \
  }

#define RCCHECK_GOTO(TAG, fn, LABEL)                                                            \
  {                                                                                             \
    rcl_ret_t temp_rc = fn;                                                                     \
    if ((temp_rc != RCL_RET_OK)) {                                                              \
      log_error(TAG, "ERROR!! ret: %d, %s, %s, %d", temp_rc, __FILE__, __FUNCTION__, __LINE__); \
      goto LABEL;                                                                               \
    }                                                                                           \
  }

#define RCCHECK_RETURN_FALSE(TAG, fn)                                                           \
  {                                                                                             \
    rcl_ret_t temp_rc = fn;                                                                     \
    if ((temp_rc != RCL_RET_OK)) {                                                              \
      log_error(TAG, "ERROR!! ret: %d, %s, %s, %d", temp_rc, __FILE__, __FUNCTION__, __LINE__); \
      return false;                                                                             \
    }                                                                                           \
  }

#define EXECUTE_EVERY_N_MS(MS, X)      \
  do {                                 \
    static volatile int64_t init = -1; \
    if (init == -1) {                  \
      init = uxr_millis();             \
    }                                  \
    if (uxr_millis() - init > MS) {    \
      X;                               \
      init = uxr_millis();             \
    }                                  \
  } while (0)

bool agent_init(void);
void agent_spin(void);
bool agent_is_connected(void);

bool entity_create(void);
bool entity_destroy(void);
void entity_spin(void);
void entity_set_domain_id(size_t domain_id);

bool subscriber_create(rcl_node_t *node);
bool subscriber_destroy(rcl_node_t *node);
bool subscriber_add_executor(rclc_executor_t *executor);
bool subscriber_set_callback(uros_sub_callback_t cb);

bool publisher_create(rcl_node_t *node);
bool publisher_destroy(rcl_node_t *node);
bool publisher_publish(uros_pub_data_flag_t data_flag, uros_pub_data_t *data);

bool service_create(rcl_node_t *node);
bool service_destroy(rcl_node_t *node);
bool service_add_executor(rclc_executor_t *executor);
bool service_set_callback(uros_srv_callback_t cb);

void action_worker(bool uart_mutex_get);
bool action_create(rcl_node_t *node, rclc_support_t *support);
bool action_destroy(rcl_node_t *node);
bool action_add_executor(rclc_executor_t *executor);
bool action_set_callback(uros_action_goal_callback_t cb_goal, uros_action_worker_callback_t cb_worker, uros_action_cancel_callback_t cb_cancel);

#endif /* DF049485_2815_48B5_AF4E_2156832B62E6_H_ */
