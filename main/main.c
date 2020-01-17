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
#define JOYSTICK_Y_ADC_CH ADC_CHANNEL_3
#define JOYSTICK_ARM_GPIO GPIO_NUM_34
#define JOYSTICK_THR_DOWN_GPIO GPIO_NUM_35
#define JOYSTICK_THR_UP_GPIO GPIO_NUM_32


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (INTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

static void task_interface(void *args)
{
    joystick_t joystick = {
        .config = {
            .x_adc_channel = JOYSTICK_X_ADC_CH,
            .y_adc_channel = JOYSTICK_Y_ADC_CH,
            .arm_gpio_num = JOYSTICK_ARM_GPIO,
            .thr_up_gpio_num = JOYSTICK_THR_UP_GPIO,
            .thr_down_gpio_num = JOYSTICK_THR_DOWN_GPIO 
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
        printf("X: %d - %d Y: %d - %d ARM: %d, THR_DOWN: %d, THR_UP: %d\n", position.x_position, position.x_delta, position.y_position, position.y_delta,
                                                 joystick.arm_value, joystick.thr_down_value, joystick.thr_up_value);
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
