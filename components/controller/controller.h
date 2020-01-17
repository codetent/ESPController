#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/adc.h"


/* -------------------------------------------------------------------------- */
/*                                  TYPEDEFS                                  */
/* -------------------------------------------------------------------------- */

typedef enum {
    CONTROLLER_POS_NEUTRAL,
    CONTROLLER_POS_1,
    CONTROLLER_POS_2
} controller_ch_position_t;

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
} controller_t;

typedef struct {
    uint32_t x_delta;
    uint32_t y_delta;
    controller_ch_position_t x_position;
    controller_ch_position_t y_position;
} controller_position_t;


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initialize the peripherals to which the controller is connected
 * 
 * @param controller Structure which holds the controller information
 * @return esp_err_t 
 */
esp_err_t controller_configure(controller_t *controller);

/**
 * @brief Read raw values of the controller and store it in the given structure
 * 
 * @param controller Structure which holds the controller information
 * @return esp_err_t 
 */
esp_err_t controller_read_raw(controller_t *controller);

/**
 * @brief Calculate the position of the controller with the values stored in the given structure
 * 
 * @param controller Structure which holds the controller information
 * @param threshold Threshold for out-of-center detection
 * @param position Resulting controller position structure
 * @return esp_err_t 
 */
esp_err_t controller_calc_pos(controller_t *controller, uint32_t threshold, controller_position_t *position);


#endif // __CONTROLLER_H__
