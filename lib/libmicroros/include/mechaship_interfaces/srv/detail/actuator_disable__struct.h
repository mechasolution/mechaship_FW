// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from mechaship_interfaces:srv/ActuatorDisable.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "mechaship_interfaces/srv/actuator_disable.h"


#ifndef MECHASHIP_INTERFACES__SRV__DETAIL__ACTUATOR_DISABLE__STRUCT_H_
#define MECHASHIP_INTERFACES__SRV__DETAIL__ACTUATOR_DISABLE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in srv/ActuatorDisable in the package mechaship_interfaces.
typedef struct mechaship_interfaces__srv__ActuatorDisable_Request
{
  uint8_t structure_needs_at_least_one_member;
} mechaship_interfaces__srv__ActuatorDisable_Request;

// Struct for a sequence of mechaship_interfaces__srv__ActuatorDisable_Request.
typedef struct mechaship_interfaces__srv__ActuatorDisable_Request__Sequence
{
  mechaship_interfaces__srv__ActuatorDisable_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__srv__ActuatorDisable_Request__Sequence;

// Constants defined in the message

/// Struct defined in srv/ActuatorDisable in the package mechaship_interfaces.
typedef struct mechaship_interfaces__srv__ActuatorDisable_Response
{
  bool status;
} mechaship_interfaces__srv__ActuatorDisable_Response;

// Struct for a sequence of mechaship_interfaces__srv__ActuatorDisable_Response.
typedef struct mechaship_interfaces__srv__ActuatorDisable_Response__Sequence
{
  mechaship_interfaces__srv__ActuatorDisable_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__srv__ActuatorDisable_Response__Sequence;

// Constants defined in the message

// Include directives for member types
// Member 'info'
#include "service_msgs/msg/detail/service_event_info__struct.h"

// constants for array fields with an upper bound
// request
enum
{
  mechaship_interfaces__srv__ActuatorDisable_Event__request__MAX_SIZE = 1
};
// response
enum
{
  mechaship_interfaces__srv__ActuatorDisable_Event__response__MAX_SIZE = 1
};

/// Struct defined in srv/ActuatorDisable in the package mechaship_interfaces.
typedef struct mechaship_interfaces__srv__ActuatorDisable_Event
{
  service_msgs__msg__ServiceEventInfo info;
  mechaship_interfaces__srv__ActuatorDisable_Request__Sequence request;
  mechaship_interfaces__srv__ActuatorDisable_Response__Sequence response;
} mechaship_interfaces__srv__ActuatorDisable_Event;

// Struct for a sequence of mechaship_interfaces__srv__ActuatorDisable_Event.
typedef struct mechaship_interfaces__srv__ActuatorDisable_Event__Sequence
{
  mechaship_interfaces__srv__ActuatorDisable_Event * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__srv__ActuatorDisable_Event__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MECHASHIP_INTERFACES__SRV__DETAIL__ACTUATOR_DISABLE__STRUCT_H_
