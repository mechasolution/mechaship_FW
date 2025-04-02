// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from mechaship_interfaces:msg/RgbwLedColor.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "mechaship_interfaces/msg/rgbw_led_color.h"


#ifndef MECHASHIP_INTERFACES__MSG__DETAIL__RGBW_LED_COLOR__STRUCT_H_
#define MECHASHIP_INTERFACES__MSG__DETAIL__RGBW_LED_COLOR__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

/// Struct defined in msg/RgbwLedColor in the package mechaship_interfaces.
typedef struct mechaship_interfaces__msg__RgbwLedColor
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white;
} mechaship_interfaces__msg__RgbwLedColor;

// Struct for a sequence of mechaship_interfaces__msg__RgbwLedColor.
typedef struct mechaship_interfaces__msg__RgbwLedColor__Sequence
{
  mechaship_interfaces__msg__RgbwLedColor * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__msg__RgbwLedColor__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MECHASHIP_INTERFACES__MSG__DETAIL__RGBW_LED_COLOR__STRUCT_H_
