#ifndef __MODULES_ERROR_LOOP_H__
#define __MODULES_ERROR_LOOP_H__

#include <rcl/rcl.h>

void error_loop(rcl_ret_t error, const char *file_name_, const char *function_name_, int line_cnt_);

#endif