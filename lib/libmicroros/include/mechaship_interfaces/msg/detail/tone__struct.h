// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from mechaship_interfaces:msg/Tone.idl
// generated code does not contain a copyright notice

#ifndef MECHASHIP_INTERFACES__MSG__DETAIL__TONE__STRUCT_H_
#define MECHASHIP_INTERFACES__MSG__DETAIL__TONE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/Tone in the package mechaship_interfaces.
typedef struct mechaship_interfaces__msg__Tone
{
  uint16_t hz;
  uint16_t duration_ms;
} mechaship_interfaces__msg__Tone;

// Struct for a sequence of mechaship_interfaces__msg__Tone.
typedef struct mechaship_interfaces__msg__Tone__Sequence
{
  mechaship_interfaces__msg__Tone * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__msg__Tone__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MECHASHIP_INTERFACES__MSG__DETAIL__TONE__STRUCT_H_
