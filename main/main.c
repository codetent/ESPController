#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "controller.h"
#include "mw_proto.h"

/* -------------------------------------------------------------------------- */
/*                                   DEFINES                                  */
/* -------------------------------------------------------------------------- */

#define CONTROLLER_X_ADC_CH ADC_CHANNEL_0
#define CONTROLLER_Y_ADC_CH ADC_CHANNEL_3
#define CONTROLLER_ARM_GPIO GPIO_NUM_34
#define CONTROLLER_THR_DOWN_GPIO GPIO_NUM_35
#define CONTROLLER_THR_UP_GPIO GPIO_NUM_32

#define BUTTON_RELEASED 1U
#define BUTTON_PRESSED 0U
#define THROTTLE_STEP 50

/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (INTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

static void task_controller(void *args)
{
    controller_t controller = {
        .config = {
            .x_adc_channel = CONTROLLER_X_ADC_CH,
            .y_adc_channel = CONTROLLER_Y_ADC_CH,
            .arm_gpio_num = CONTROLLER_ARM_GPIO,
            .thr_up_gpio_num = CONTROLLER_THR_UP_GPIO,
            .thr_down_gpio_num = CONTROLLER_THR_DOWN_GPIO 
        }
    };

    controller_position_t position = {0U};

    uint8_t last_arm_value = BUTTON_RELEASED;
    uint8_t last_thr_down_value = BUTTON_RELEASED;
    uint8_t last_thr_up_value = BUTTON_RELEASED;

    uint16_t roll = MW_MID_VALUE;
    uint16_t pitch = MW_MID_VALUE;
    uint16_t throttle = MW_MIN_VALUE;

    // Configure periphery
    adc1_config_width(ADC_WIDTH_BIT_12);
    controller_configure(&controller);

    while (1U) 
    {
        // Get controller position
        controller_read_raw(&controller);
        controller_calc_pos(&controller, 100U, &position);

        /*// Print result
        printf("X: %d - %d Y: %d - %d ARM: %d, THR_DOWN: %d, THR_UP: %d\n", position.x_position, position.x_delta, position.y_position, position.y_delta,
                                                 controller.arm_value, controller.thr_down_value, controller.thr_up_value); */
        
        // Button check if pressed
        if(controller.arm_value && !last_arm_value){
            printf("ARM pressed!\n");
        }
        if(controller.thr_down_value && !last_thr_down_value){
            printf("THR_DOWN pressed!\n");
            if((throttle - THROTTLE_STEP) <= MW_MIN_VALUE){
                throttle = MW_MIN_VALUE;
            }else{
                throttle -= THROTTLE_STEP;
            }
            
        }
        if(controller.thr_up_value && !last_thr_up_value){
            printf("THR_UP pressed!\n");
            if((throttle + THROTTLE_STEP) >= MW_MAX_VALUE){
                throttle = MW_MAX_VALUE;
            }else{
                throttle += THROTTLE_STEP;
            }
        }

        // Set Flight parameter
        if(position.x_position == 0){
            roll = MW_MID_VALUE;
        }else if(position.x_position == 1){
            roll = MW_MID_VALUE - position.x_delta / 3;
        }else if(position.x_position == 2){
            roll = MW_MID_VALUE + position.x_delta / 3;
        }

        if(position.y_position == 0){
            pitch = MW_MID_VALUE;
        }else if(position.y_position == 1){
            pitch = MW_MID_VALUE - position.y_delta / 3;
        }else if(position.y_position == 2){
            pitch = MW_MID_VALUE + position.y_delta / 3;
        }

        printf("ROLL: %d, PITCH: %d, THOTTLE: %d\n", roll, pitch, throttle);
        
        last_arm_value = controller.arm_value;
        last_thr_down_value = controller.thr_down_value;
        last_thr_up_value = controller.thr_up_value;

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
        task_controller,
        "INTERFACE",
        2048U,
        NULL,
        tskIDLE_PRIORITY,
        &handle_task_interface
    );
}
