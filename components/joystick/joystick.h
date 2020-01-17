#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/adc.h"


/* -------------------------------------------------------------------------- */
/*                                  TYPEDEFS                                  */
/* -------------------------------------------------------------------------- */

typedef enum {
    JOYSTICK_POS_NEUTRAL,
    JOYSTICK_POS_1,
    JOYSTICK_POS_2
} joystick_ch_position_t;

typedef struct {
    uint32_t x_value;
    uint32_t y_value;
    uint32_t arm_value;
    uint32_t thr_up_value;
    uint32_t thr_down_value;

    uint32_t x_origin;
    uint32_t y_origin;

    struct {
        adc1_channel_t x_adc_channel;
        adc1_channel_t y_adc_channel;
        gpio_num_t arm_gpio_num;
        gpio_num_t thr_up_gpio_num;
        gpio_num_t thr_down_gpio_num;
    } config;

    bool _initialized;
} joystick_t;

typedef struct {
    uint32_t x_delta;
    uint32_t y_delta;
    joystick_ch_position_t x_position;
    joystick_ch_position_t y_position;
} joystick_position_t;


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initialize the peripherals to which the joystick is connected
 * 
 * @param joystick Structure which holds the joystick information
 * @return esp_err_t 
 */
esp_err_t joystick_configure(joystick_t *joystick);

/**
 * @brief Read raw values of the joystick and store it in the given structure
 * 
 * @param joystick Structure which holds the joystick information
 * @return esp_err_t 
 */
esp_err_t joystick_read_raw(joystick_t *joystick);

/**
 * @brief Calculate the position of the joystick with the values stored in the given structure
 * 
 * @param joystick Structure which holds the joystick information
 * @param threshold Threshold for out-of-center detection
 * @param position Resulting joystick position structure
 * @return esp_err_t 
 */
esp_err_t joystick_calc_pos(joystick_t *joystick, uint32_t threshold, joystick_position_t *position);


#endif // __JOYSTICK_H__
