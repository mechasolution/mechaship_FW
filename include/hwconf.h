#ifndef SRC_HW_HWCONF
#define SRC_HW_HWCONF

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

#define HWCONF_NODE DT_PATH(zephyr_user)
#define HWCONF_PIEZO_PWM_NODE DT_NODELABEL(piezo_pwm)
#define HWCONF_SERVO_PWM_NODE DT_NODELABEL(servo_pwm)
#define HWCONF_ESC_PWM_NODE DT_NODELABEL(esc_pwm)

#if !DT_NODE_EXISTS(HWCONF_NODE)
#error "Devicetree node '/zephyr,user' is missing"
#endif

#define HWCONF_PIN_RC_CH1 DT_GPIO_PIN(HWCONF_NODE, rc_ch1_gpios)
#define HWCONF_PIN_RC_CH2 DT_GPIO_PIN(HWCONF_NODE, rc_ch2_gpios)
#define HWCONF_PIN_RC_CH3 DT_GPIO_PIN(HWCONF_NODE, rc_ch3_gpios)
#define HWCONF_PIN_RC_CH4 DT_GPIO_PIN(HWCONF_NODE, rc_ch4_gpios)
#define HWCONF_PIN_EMO_SWITCH DT_GPIO_PIN(HWCONF_NODE, emo_switch_gpios)
#define HWCONF_PIN_RGBW_LED DT_GPIO_PIN(HWCONF_NODE, rgbw_led_gpios)
#define HWCONF_PIN_LED_RC_MODE DT_GPIO_PIN(HWCONF_NODE, led_rc_mode_gpios)
#define HWCONF_PIN_LED_ROS_MODE DT_GPIO_PIN(HWCONF_NODE, led_ros_mode_gpios)
#define HWCONF_PIN_LED_FAULT DT_GPIO_PIN(HWCONF_NODE, led_fault_gpios)
#define HWCONF_PIN_LED_STATUS DT_GPIO_PIN(HWCONF_NODE, led_status_gpios)
#define HWCONF_PIN_POWER_ACT_EN DT_GPIO_PIN(HWCONF_NODE, power_act_en_gpios)
#define HWCONF_PIN_POWER_SBC_EN DT_GPIO_PIN(HWCONF_NODE, power_sbc_en_gpios)
#define HWCONF_PIN_POWER_MAIN_EN DT_GPIO_PIN(HWCONF_NODE, power_main_en_gpios)
#define HWCONF_PIN_SWITCH_POWER DT_GPIO_PIN(HWCONF_NODE, switch_power_gpios)
#define HWCONF_PIN_SHIFT_LATCH DT_GPIO_PIN(HWCONF_NODE, shift_latch_gpios)
#define HWCONF_PIN_SHIFT_CLK DT_GPIO_PIN(HWCONF_NODE, shift_clk_gpios)
#define HWCONF_PIN_SHIFT_DATA DT_GPIO_PIN(HWCONF_NODE, shift_data_gpios)
#define HWCONF_PIN_VOLTAGE_BATTERY 29

#define HWCONF_SERIAL_DEBUG_BAUDRATE DT_PROP(HWCONF_NODE, debug_baudrate)

#define HWCONF_BATTERY_CELL_CNT DT_PROP(HWCONF_NODE, battery_cell_count)
#define HWCONF_BATTERY_TOTAL_LOW_ALERT_VOLTAGE ((float)DT_PROP(HWCONF_NODE, battery_total_low_alert_mv) / 1000.0f)
#define HWCONF_BATTERY_VOLTAGE_DEVIDER ((float)DT_PROP(HWCONF_NODE, battery_divider_milli) / 1000.0f)

#define HWCONF_RGBW_LED_PIXEL_CNT DT_PROP(HWCONF_NODE, rgbw_led_pixel_count)

#define HWCONF_RC_PIO_ID DT_PROP(HWCONF_NODE, rc_pio_id)
#define HWCONF_RC_THROTTLE_PULSE_MARGIN DT_PROP(HWCONF_NODE, rc_throttle_pulse_margin)
#define HWCONF_RC_THROTTLE_MAX_PULSE DT_PROP(HWCONF_NODE, rc_throttle_max_pulse)
#define HWCONF_RC_THROTTLE_MIN_PULSE DT_PROP(HWCONF_NODE, rc_throttle_min_pulse)
#define HWCONF_RC_THROTTLE_MIDDLE DT_PROP(HWCONF_NODE, rc_throttle_middle)

#define HWCONF_RC_KEY_PULSE_MARGIN DT_PROP(HWCONF_NODE, rc_key_pulse_margin)
#define HWCONF_RC_KEY_MAX_PULSE DT_PROP(HWCONF_NODE, rc_key_max_pulse)
#define HWCONF_RC_KEY_MIN_PULSE DT_PROP(HWCONF_NODE, rc_key_min_pulse)
#define HWCONF_RC_KEY_MIDDLE DT_PROP(HWCONF_NODE, rc_key_middle)

#define HWCONF_RC_CH3_MAX_PULSE DT_PROP(HWCONF_NODE, rc_ch3_max_pulse)
#define HWCONF_RC_CH3_MIN_PULSE DT_PROP(HWCONF_NODE, rc_ch3_min_pulse)
#define HWCONF_RC_CH3_MIDDLE DT_PROP(HWCONF_NODE, rc_ch3_middle)

#define HWCONF_RC_CH4_DEFAULT_PULSE DT_PROP(HWCONF_NODE, rc_ch4_default_pulse)
#define HWCONF_RC_CH4_CLICKED_PULSE DT_PROP(HWCONF_NODE, rc_ch4_clicked_pulse)

#define HWCONF_LED_STATUS_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, led_status_gpios)
#define HWCONF_LED_ROS_MODE_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, led_ros_mode_gpios)
#define HWCONF_LED_RC_MODE_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, led_rc_mode_gpios)
#define HWCONF_LED_FAULT_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, led_fault_gpios)

#define HWCONF_EMO_SWITCH_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, emo_switch_gpios)

#define HWCONF_POWER_MAIN_EN_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, power_main_en_gpios)
#define HWCONF_POWER_SBC_EN_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, power_sbc_en_gpios)
#define HWCONF_POWER_ACT_EN_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, power_act_en_gpios)
#define HWCONF_POWER_SWITCH_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, switch_power_gpios)

#define HWCONF_SHIFT_LATCH_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, shift_latch_gpios)
#define HWCONF_SHIFT_CLK_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, shift_clk_gpios)
#define HWCONF_SHIFT_DATA_SPEC GPIO_DT_SPEC_GET(HWCONF_NODE, shift_data_gpios)

#define HWCONF_PIEZO_PWM_SPEC PWM_DT_SPEC_GET(HWCONF_PIEZO_PWM_NODE)
#define HWCONF_SERVO_PWM_SPEC PWM_DT_SPEC_GET(HWCONF_SERVO_PWM_NODE)
#define HWCONF_ESC_PWM_SPEC PWM_DT_SPEC_GET(HWCONF_ESC_PWM_NODE)

#define HWCONF_BATTERY_ADC_SPEC ADC_DT_SPEC_GET_BY_IDX(HWCONF_NODE, 0)

#endif /* SRC_HW_HWCONF */
