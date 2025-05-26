#ifndef SRC_HW_HWCONF
#define SRC_HW_HWCONF

/**
 * @brief Pins
 *
 */
#define HWCONF_PIN_RES2_ALT 0           // Reserved alternative
#define HWCONF_PIN_RES1_ALT 1           // Reserved alternative
#define HWCONF_PIN_SBC_POWERED_SIG_IN 2 // DIN
#define HWCONF_PIN_RC_CH1 3             //         peripheral PIO1 SM0
#define HWCONF_PIN_RC_CH2 4             //         peripheral PIO1 SM1
#define HWCONF_PIN_RC_CH3 5             //         peripheral PIO1 SM2
#define HWCONF_PIN_RC_CH4 6             //         peripheral PIO1 SM3
#define HWCONF_PIN_EMO_SWITCH 7         // DIN
#define HWCONF_PIN_SERIAL1_TX 8         //         peripheral UART1
#define HWCONF_PIN_SERIAL1_RX 9         //         peripheral UART1
#define HWCONF_PIN_I2C1_SDA 10          //         peripheral I2C1
#define HWCONF_PIN_I2C1_SCL 11          //         peripheral I2C1
#define HWCONF_PIN_RGBW_LED 12          //         peripheral PIO0 (serialized data OUT)
#define HWCONF_PIN_LED_RC_MODE 13       //      DOUT
#define HWCONF_PIN_LED_ROS_MODE 14      //      DOUT
#define HWCONF_PIN_LED_FAULT 15         //      DOUT
#define HWCONF_PIN_LED_STATUS 16        //      DOUT
#define HWCONF_PIN_PIEZO 17             //         peripheral PWM0 B
#define HWCONF_PIN_POWER_ACT_EN 18      //      DOUT
#define HWCONF_PIN_POWER_SBC_EN 19      //      DOUT
#define HWCONF_PIN_POWER_MAIN_EN 20     //      DOUT
#define HWCONF_PIN_SWITCH_POWER 21      // DIN
#define HWCONF_PIN_PWM_SERVO 22         //         peripheral PWM3 A
#define HWCONF_PIN_PWM_ESC 23           //         peripheral PWM3 B
#define HWCONF_PIN_RES1_PRI 24          //         peripheral PWM4 A
#define HWCONF_PIN_RES2_PRI 25          //         peripheral PWM4 B
#define HWCONF_PIN_SHIFT_LATCH 26       //      DOUT
#define HWCONF_PIN_SHIFT_CLK 27         //      DOUT
#define HWCONF_PIN_SHIFT_DATA 28        // DIN (serialized data IN)
#define HWCONF_PIN_VOLTAGE_BATTERY 29   //         peripheral ADC3

/**
 * @brief Peripherals
 *
 */
#define HWCONF_SERIAL_DEBUG_ID uart1
#define HWCONF_SERIAL_DEBUG_BAUDRATE 115200
#define HWCONF_SERIAL_DEBUG_PIN_TX HWCONF_PIN_SERIAL1_TX // default
#define HWCONF_SERIAL_DEBUG_PIN_RX HWCONF_PIN_SERIAL1_RX // default

/**
 * @brief battery
 *
 */
#define HWCONF_BATTERY_CELL_CNT 3                                              // 배터리 셀 수
#define HWCONF_BATTERY_TOTAL_LOW_ALERT_VOLTAGE (3.1 * HWCONF_BATTERY_CELL_CNT) // 저전압 컷 전압 (전원 강제 종료)
#define HWCONF_BATTERY_CELL_VOLTAGE_DEVIDER_HIGH_SIDE 33.0                     // 셀 분배저항 +측
#define HWCONF_BATTERY_CELL_VOLTAGE_DEVIDER_LOW_SIDE 10.0                      // 셀 분배저항 -측

#define HWCONF_BATTERY_VOLTAGE_DEVIDER ((HWCONF_BATTERY_CELL_VOLTAGE_DEVIDER_HIGH_SIDE + HWCONF_BATTERY_CELL_VOLTAGE_DEVIDER_LOW_SIDE) / HWCONF_BATTERY_CELL_VOLTAGE_DEVIDER_LOW_SIDE)

/**
 * @brief RGBW LED
 *
 */
#define HWCONF_RGBW_LED_PIXEL_CNT 9

/**
 * @brief RC Receiver
 *
 */
#define HWCONF_RC_PIO_ID pio1

#define HWCONF_RC_THROTTLE_PULSE_MARGIN 20
#define HWCONF_RC_THROTTLE_MAX_PULSE (2000 - HWCONF_RC_THROTTLE_PULSE_MARGIN)
#define HWCONF_RC_THROTTLE_MIN_PULSE (1000 + HWCONF_RC_THROTTLE_PULSE_MARGIN)
#define HWCONF_RC_THROTTLE_MIDDLE ((HWCONF_RC_THROTTLE_MAX_PULSE + HWCONF_RC_THROTTLE_MIN_PULSE) / 2)

#define HWCONF_RC_KEY_PULSE_MARGIN 5
#define HWCONF_RC_KEY_MAX_PULSE (2100 - HWCONF_RC_KEY_PULSE_MARGIN)
#define HWCONF_RC_KEY_MIN_PULSE (900 + HWCONF_RC_KEY_PULSE_MARGIN)
#define HWCONF_RC_KEY_MIDDLE ((HWCONF_RC_KEY_MAX_PULSE + HWCONF_RC_KEY_MIN_PULSE) / 2)

#define HWCONF_RC_CH3_MAX_PULSE (2000)
#define HWCONF_RC_CH3_MIN_PULSE (1000)
#define HWCONF_RC_CH3_MIDDLE ((HWCONF_RC_CH3_MAX_PULSE + HWCONF_RC_CH3_MIN_PULSE) / 2)

#define HWCONF_RC_CH4_DEFAULT_PULSE (1000)
#define HWCONF_RC_CH4_CLICKED_PULSE (2000)

#endif /* SRC_HW_HWCONF */
