// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from mechaship_interfaces:msg/Tone.idl
// generated code does not contain a copyright notice

#ifndef MECHASHIP_INTERFACES__MSG__DETAIL__TONE__FUNCTIONS_H_
#define MECHASHIP_INTERFACES__MSG__DETAIL__TONE__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "mechaship_interfaces/msg/rosidl_generator_c__visibility_control.h"

#include "mechaship_interfaces/msg/detail/tone__struct.h"

/// Initialize msg/Tone message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * mechaship_interfaces__msg__Tone
 * )) before or use
 * mechaship_interfaces__msg__Tone__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__Tone__init(mechaship_interfaces__msg__Tone * msg);

/// Finalize msg/Tone message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
void
mechaship_interfaces__msg__Tone__fini(mechaship_interfaces__msg__Tone * msg);

/// Create msg/Tone message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * mechaship_interfaces__msg__Tone__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
mechaship_interfaces__msg__Tone *
mechaship_interfaces__msg__Tone__create();

/// Destroy msg/Tone message.
/**
 * It calls
 * mechaship_interfaces__msg__Tone__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
void
mechaship_interfaces__msg__Tone__destroy(mechaship_interfaces__msg__Tone * msg);

/// Check for msg/Tone message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__Tone__are_equal(const mechaship_interfaces__msg__Tone * lhs, const mechaship_interfaces__msg__Tone * rhs);

/// Copy a msg/Tone message.
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
mechaship_interfaces__msg__Tone__copy(
  const mechaship_interfaces__msg__Tone * input,
  mechaship_interfaces__msg__Tone * output);

/// Initialize array of msg/Tone messages.
/**
 * It allocates the memory for the number of elements and calls
 * mechaship_interfaces__msg__Tone__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__Tone__Sequence__init(mechaship_interfaces__msg__Tone__Sequence * array, size_t size);

/// Finalize array of msg/Tone messages.
/**
 * It calls
 * mechaship_interfaces__msg__Tone__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
void
mechaship_interfaces__msg__Tone__Sequence__fini(mechaship_interfaces__msg__Tone__Sequence * array);

/// Create array of msg/Tone messages.
/**
 * It allocates the memory for the array and calls
 * mechaship_interfaces__msg__Tone__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
mechaship_interfaces__msg__Tone__Sequence *
mechaship_interfaces__msg__Tone__Sequence__create(size_t size);

/// Destroy array of msg/Tone messages.
/**
 * It calls
 * mechaship_interfaces__msg__Tone__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
void
mechaship_interfaces__msg__Tone__Sequence__destroy(mechaship_interfaces__msg__Tone__Sequence * array);

/// Check for msg/Tone message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_mechaship_interfaces
bool
mechaship_interfaces__msg__Tone__Sequence__are_equal(const mechaship_interfaces__msg__Tone__Sequence * lhs, const mechaship_interfaces__msg__Tone__Sequence * rhs);

/// Copy an array of msg/Tone messages.
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
mechaship_interfaces__msg__Tone__Sequence__copy(
  const mechaship_interfaces__msg__Tone__Sequence * input,
  mechaship_interfaces__msg__Tone__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // MECHASHIP_INTERFACES__MSG__DETAIL__TONE__FUNCTIONS_H_
