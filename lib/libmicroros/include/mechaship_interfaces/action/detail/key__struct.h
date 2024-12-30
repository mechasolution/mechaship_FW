// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from mechaship_interfaces:action/Key.idl
// generated code does not contain a copyright notice

#ifndef MECHASHIP_INTERFACES__ACTION__DETAIL__KEY__STRUCT_H_
#define MECHASHIP_INTERFACES__ACTION__DETAIL__KEY__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in action/Key in the package mechaship_interfaces.
typedef struct mechaship_interfaces__action__Key_Goal
{
  float target_degree;
  float degrees_per_second;
} mechaship_interfaces__action__Key_Goal;

// Struct for a sequence of mechaship_interfaces__action__Key_Goal.
typedef struct mechaship_interfaces__action__Key_Goal__Sequence
{
  mechaship_interfaces__action__Key_Goal * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__action__Key_Goal__Sequence;


// Constants defined in the message

/// Struct defined in action/Key in the package mechaship_interfaces.
typedef struct mechaship_interfaces__action__Key_Result
{
  bool status;
} mechaship_interfaces__action__Key_Result;

// Struct for a sequence of mechaship_interfaces__action__Key_Result.
typedef struct mechaship_interfaces__action__Key_Result__Sequence
{
  mechaship_interfaces__action__Key_Result * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__action__Key_Result__Sequence;


// Constants defined in the message

/// Struct defined in action/Key in the package mechaship_interfaces.
typedef struct mechaship_interfaces__action__Key_Feedback
{
  float current_degree;
} mechaship_interfaces__action__Key_Feedback;

// Struct for a sequence of mechaship_interfaces__action__Key_Feedback.
typedef struct mechaship_interfaces__action__Key_Feedback__Sequence
{
  mechaship_interfaces__action__Key_Feedback * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__action__Key_Feedback__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
#include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'goal'
#include "mechaship_interfaces/action/detail/key__struct.h"

/// Struct defined in action/Key in the package mechaship_interfaces.
typedef struct mechaship_interfaces__action__Key_SendGoal_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
  mechaship_interfaces__action__Key_Goal goal;
} mechaship_interfaces__action__Key_SendGoal_Request;

// Struct for a sequence of mechaship_interfaces__action__Key_SendGoal_Request.
typedef struct mechaship_interfaces__action__Key_SendGoal_Request__Sequence
{
  mechaship_interfaces__action__Key_SendGoal_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__action__Key_SendGoal_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.h"

/// Struct defined in action/Key in the package mechaship_interfaces.
typedef struct mechaship_interfaces__action__Key_SendGoal_Response
{
  bool accepted;
  builtin_interfaces__msg__Time stamp;
} mechaship_interfaces__action__Key_SendGoal_Response;

// Struct for a sequence of mechaship_interfaces__action__Key_SendGoal_Response.
typedef struct mechaship_interfaces__action__Key_SendGoal_Response__Sequence
{
  mechaship_interfaces__action__Key_SendGoal_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__action__Key_SendGoal_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"

/// Struct defined in action/Key in the package mechaship_interfaces.
typedef struct mechaship_interfaces__action__Key_GetResult_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
} mechaship_interfaces__action__Key_GetResult_Request;

// Struct for a sequence of mechaship_interfaces__action__Key_GetResult_Request.
typedef struct mechaship_interfaces__action__Key_GetResult_Request__Sequence
{
  mechaship_interfaces__action__Key_GetResult_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__action__Key_GetResult_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'result'
// already included above
// #include "mechaship_interfaces/action/detail/key__struct.h"

/// Struct defined in action/Key in the package mechaship_interfaces.
typedef struct mechaship_interfaces__action__Key_GetResult_Response
{
  int8_t status;
  mechaship_interfaces__action__Key_Result result;
} mechaship_interfaces__action__Key_GetResult_Response;

// Struct for a sequence of mechaship_interfaces__action__Key_GetResult_Response.
typedef struct mechaship_interfaces__action__Key_GetResult_Response__Sequence
{
  mechaship_interfaces__action__Key_GetResult_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__action__Key_GetResult_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'feedback'
// already included above
// #include "mechaship_interfaces/action/detail/key__struct.h"

/// Struct defined in action/Key in the package mechaship_interfaces.
typedef struct mechaship_interfaces__action__Key_FeedbackMessage
{
  unique_identifier_msgs__msg__UUID goal_id;
  mechaship_interfaces__action__Key_Feedback feedback;
} mechaship_interfaces__action__Key_FeedbackMessage;

// Struct for a sequence of mechaship_interfaces__action__Key_FeedbackMessage.
typedef struct mechaship_interfaces__action__Key_FeedbackMessage__Sequence
{
  mechaship_interfaces__action__Key_FeedbackMessage * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} mechaship_interfaces__action__Key_FeedbackMessage__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MECHASHIP_INTERFACES__ACTION__DETAIL__KEY__STRUCT_H_
