#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "joystick.h"


/* -------------------------------------------------------------------------- */
/*                                   DEFINES                                  */
/* -------------------------------------------------------------------------- */

#define JOYSTICK_X_ADC_CH ADC_CHANNEL_0
#define JOYSTICK_Y_ADC_CH ADC_CHANNEL_1
#define JOYSTICK_Z_GPIO GPIO_NUM_27


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (INTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

static void task_interface(void *args)
{
    joystick_t joystick = {
        .config = {
            .x_adc_channel = JOYSTICK_X_ADC_CH,
            .y_adc_channel = JOYSTICK_Y_ADC_CH,
            .z_gpio_num = JOYSTICK_Z_GPIO
        }
    };
    joystick_position_t position = {0U};

    // Configure periphery
    adc1_config_width(ADC_WIDTH_BIT_12);
    joystick_configure(&joystick);

    while (1U) 
    {
        // Get joystick position
        joystick_read_raw(&joystick);
        joystick_calc_pos(&joystick, 100U, &position);

        // Print result
        printf("X: %d - %d Y: %d - %d\n", position.x_position, position.x_delta, position.y_position, position.y_delta);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL);
}


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

void app_main()
{
    TaskHandle_t handle_task_interface = NULL;

    xTaskCreate(
        task_interface,
        "INTERFACE",
        2048U,
        NULL,
        tskIDLE_PRIORITY,
        &handle_task_interface
    );
}
