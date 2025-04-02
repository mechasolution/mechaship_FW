// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from mechaship_interfaces:msg/RgbwLedColor.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "mechaship_interfaces/msg/rgbw_led_color.h"


#ifndef MECHASHIP_INTERFACES__MSG__DETAIL__RGBW_LED_COLOR__FUNCTIONS_H_
#define MECHASHIP_INTERFACES__MSG__DETAIL__RGBW_LED_COLOR__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/action_type_support_struct.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_runtime_c/service_type_support_struct.h"
#include "rosidl_runtime_c/type_description/type_description__struct.h"
#include "rosidl_runtime_c/type_description/type_source__struct.h"
#include "rosidl_runtime_c/type_hash.h"
#include "rosidl_runtime_c/visibility_control.h"
#include "mechaship_interfaces/msg/rosidl_generator_c__visibility_control.h"

#include "mechaship_interfaces/msg/detail/rgbw_led_color__struct.h"

/// Initialize msg/RgbwLedColor message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * mechaship_interfaces__msg__RgbwLedColor
 * )) before or use
 * mechaship_interfaces__msg__RgbwLedColor__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__RgbwLedColor__init(mechaship_interfaces__msg__RgbwLedColor * msg);

/// Finalize msg/RgbwLedColor message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
void
mechaship_interfaces__msg__RgbwLedColor__fini(mechaship_interfaces__msg__RgbwLedColor * msg);

/// Create msg/RgbwLedColor message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * mechaship_interfaces__msg__RgbwLedColor__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
mechaship_interfaces__msg__RgbwLedColor *
mechaship_interfaces__msg__RgbwLedColor__create(void);

/// Destroy msg/RgbwLedColor message.
/**
 * It calls
 * mechaship_interfaces__msg__RgbwLedColor__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
void
mechaship_interfaces__msg__RgbwLedColor__destroy(mechaship_interfaces__msg__RgbwLedColor * msg);

/// Check for msg/RgbwLedColor message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__RgbwLedColor__are_equal(const mechaship_interfaces__msg__RgbwLedColor * lhs, const mechaship_interfaces__msg__RgbwLedColor * rhs);

/// Copy a msg/RgbwLedColor message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__RgbwLedColor__copy(
  const mechaship_interfaces__msg__RgbwLedColor * input,
  mechaship_interfaces__msg__RgbwLedColor * output);

/// Retrieve pointer to the hash of the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
const rosidl_type_hash_t *
mechaship_interfaces__msg__RgbwLedColor__get_type_hash(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
const rosidl_runtime_c__type_description__TypeDescription *
mechaship_interfaces__msg__RgbwLedColor__get_type_description(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the single raw source text that defined this type.
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
const rosidl_runtime_c__type_description__TypeSource *
mechaship_interfaces__msg__RgbwLedColor__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the recursive raw sources that defined the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
const rosidl_runtime_c__type_description__TypeSource__Sequence *
mechaship_interfaces__msg__RgbwLedColor__get_type_description_sources(
  const rosidl_message_type_support_t * type_support);

/// Initialize array of msg/RgbwLedColor messages.
/**
 * It allocates the memory for the number of elements and calls
 * mechaship_interfaces__msg__RgbwLedColor__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__RgbwLedColor__Sequence__init(mechaship_interfaces__msg__RgbwLedColor__Sequence * array, size_t size);

/// Finalize array of msg/RgbwLedColor messages.
/**
 * It calls
 * mechaship_interfaces__msg__RgbwLedColor__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
void
mechaship_interfaces__msg__RgbwLedColor__Sequence__fini(mechaship_interfaces__msg__RgbwLedColor__Sequence * array);

/// Create array of msg/RgbwLedColor messages.
/**
 * It allocates the memory for the array and calls
 * mechaship_interfaces__msg__RgbwLedColor__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
mechaship_interfaces__msg__RgbwLedColor__Sequence *
mechaship_interfaces__msg__RgbwLedColor__Sequence__create(size_t size);

/// Destroy array of msg/RgbwLedColor messages.
/**
 * It calls
 * mechaship_interfaces__msg__RgbwLedColor__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
void
mechaship_interfaces__msg__RgbwLedColor__Sequence__destroy(mechaship_interfaces__msg__RgbwLedColor__Sequence * array);

/// Check for msg/RgbwLedColor message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__RgbwLedColor__Sequence__are_equal(const mechaship_interfaces__msg__RgbwLedColor__Sequence * lhs, const mechaship_interfaces__msg__RgbwLedColor__Sequence * rhs);

/// Copy an array of msg/RgbwLedColor messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__RgbwLedColor__Sequence__copy(
  const mechaship_interfaces__msg__RgbwLedColor__Sequence * input,
  mechaship_interfaces__msg__RgbwLedColor__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // MECHASHIP_INTERFACES__MSG__DETAIL__RGBW_LED_COLOR__FUNCTIONS_H_
