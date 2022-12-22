#ifndef __SUPPORTER_H__
#define __SUPPORTER_H__

#define RCCHECK(fn)                                          \
  {                                                          \
    rcl_ret_t temp_rc = fn;                                  \
    if ((temp_rc != RCL_RET_OK)) {                           \
      error_loop(temp_rc, __FILE__, __FUNCTION__, __LINE__); \
    }                                                        \
  }

#define RCSOFTCHECK(fn)            \
  {                                \
    rcl_ret_t temp_rc = fn;        \
    if ((temp_rc != RCL_RET_OK)) { \
    }                              \
  }

#define ERRCHECK(fn)                                         \
  {                                                          \
    bool temp_rc = fn;                                       \
    if ((temp_rc != true)) {                                 \
      error_loop(temp_rc, __FILE__, __FUNCTION__, __LINE__); \
    }                                                        \
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

#endif