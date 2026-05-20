target_sources(app PRIVATE
  bsp.c
)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(led)
add_subdirectory(power)
add_subdirectory(tone)
add_subdirectory(actuator)