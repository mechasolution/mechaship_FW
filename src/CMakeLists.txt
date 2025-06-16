add_executable(mechaship_fw)

file(GLOB_RECURSE SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

file(GLOB_RECURSE PIO_FILES_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/*.pio
)

target_sources(mechaship_fw PRIVATE
  ${SRC_FILES}
)

target_include_directories(mechaship_fw PRIVATE
  hw
  app
  mw

  ../lib/libmicroros/include
  ../include
)

target_link_libraries(mechaship_fw
  pico_stdlib
  pico_unique_id

  hardware_gpio
  hardware_uart
  hardware_pwm
  hardware_i2c
  hardware_pio
  hardware_adc

  tinyusb_device
  tinyusb_board

  FreeRTOS-Kernel

  microros
)

set_target_properties(mechaship_fw PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions(-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\")

pico_generate_pio_header(mechaship_fw ${PIO_FILES_LIST})

pico_add_extra_outputs(mechaship_fw)
