target_sources(app PRIVATE
  app.c

  temp_task/temp_task.c
)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
