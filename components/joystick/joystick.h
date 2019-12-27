#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "rettype.h"


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
    uint32_t z_value;

    uint32_t x_origin;
    uint32_t y_origin;

    struct {
        adc1_channel_t x_adc_channel;
        adc1_channel_t y_adc_channel;
        gpio_num_t z_gpio_num;
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

return_type_t joystick_configure(joystick_t *joystick);
return_type_t joystick_read_raw(joystick_t *joystick);
return_type_t joystick_calc_pos(joystick_t *joystick, uint32_t threshold, joystick_position_t *position);


#endif // __JOYSTICK_H__
