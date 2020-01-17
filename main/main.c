#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "controller.h"
#include "mw_proto.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

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

#define BT_DEVICE_DRONE "XMC-Bluetooth"

#define CTR_TASK_TAG "CTR_TASK"
#define BT_TASK_TAG "BT_TASK"
#define APP_MAIN_TAG "APP_MAIN"

uint8_t bt_connected = 0U;

/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (INTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

static void task_controller(void *args)
{
    mw_frame_t *mw_frame = (mw_frame_t*) args;
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
        
        // Button check if pressed
        if(controller.arm_value && !last_arm_value){
            ESP_LOGI(CTR_TASK_TAG, "ARM pressed!\n");
            if( mw_toggle_arm(mw_frame) != ESP_OK){
                ESP_LOGE(CTR_TASK_TAG, "Initializing  mw_toggle_arm failed!\n");
                return;
            }
        }
        if(controller.thr_down_value && !last_thr_down_value){
            ESP_LOGI(CTR_TASK_TAG, "THR_DOWN pressed!\n");
            if((throttle - THROTTLE_STEP) <= MW_MIN_VALUE){
                throttle = MW_MIN_VALUE;
            }else{
                throttle -= THROTTLE_STEP;
            }

            if( mw_set_throttle(throttle, mw_frame) != ESP_OK){
                ESP_LOGE(CTR_TASK_TAG, "Initializing  mw_set_throttle failed!\n");
                return;
            }
        }
        if(controller.thr_up_value && !last_thr_up_value){
            ESP_LOGI(CTR_TASK_TAG, "THR_UP pressed!\n");
            if((throttle + THROTTLE_STEP) >= MW_MAX_VALUE){
                throttle = MW_MAX_VALUE;
            }else{
                throttle += THROTTLE_STEP;
            }

            if( mw_set_throttle(throttle, mw_frame) != ESP_OK){
                ESP_LOGE(CTR_TASK_TAG, "Initializing  mw_set_throttle failed!\n");
                return;
            }
        }

        // Set Flight parameter
        if(position.x_position == 0U){
            roll = MW_MID_VALUE;
        }else if(position.x_position == 1U){
            roll = MW_MID_VALUE - position.x_delta / 3U;
        }else if(position.x_position == 2U){
            roll = MW_MID_VALUE + position.x_delta / 3U;
        }

        if( mw_set_roll(roll, mw_frame) != ESP_OK){
            ESP_LOGE(CTR_TASK_TAG, "Initializing  mw_set_roll failed!\n");
            return;
        }

        if(position.y_position == 0U){
            pitch = MW_MID_VALUE;
        }else if(position.y_position == 1U){
            pitch = MW_MID_VALUE - position.y_delta / 3U;
        }else if(position.y_position == 2U){
            pitch = MW_MID_VALUE + position.y_delta / 3U;
        }

        if( mw_set_pitch(pitch, mw_frame) != ESP_OK){
            ESP_LOGE(CTR_TASK_TAG, "Initializing  mw_set_pitch failed!\n");
            return;
        }

        ESP_LOGI(CTR_TASK_TAG, "ROLL: %d, PITCH: %d, THOTTLE: %d\n", roll, pitch, throttle);
        
        last_arm_value = controller.arm_value;
        last_thr_down_value = controller.thr_down_value;
        last_thr_up_value = controller.thr_up_value;

        vTaskDelay(pdMS_TO_TICKS(100U));
    }

    vTaskDelete(NULL);
}

static void task_bt(void *args)
{
    mw_frame_t *mw_frame = (mw_frame_t*) args;

    // Init bluetooth

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(BT_TASK_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(BT_TASK_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(BT_TASK_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(BT_TASK_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        ESP_LOGE(BT_TASK_TAG, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(BT_TASK_TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
        ESP_LOGE(BT_TASK_TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    

    while (1U) 
    {
        if(bt_connected){
            if(esp_spp_write(my_hand, MW_PROTO_FRAME_LEN, mw_frame->data) != ESP_OK){
                ESP_LOGE(BT_TASK_TAG, "Sending frame failed!\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100U));
    }

    vTaskDelete(NULL);
}


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

void app_main()
{
    TaskHandle_t handle_task_controller = NULL;
    TaskHandle_t handle_task_bt = NULL;
    mw_frame_t mw_frame;

    if (init_mw_frame(&mw_frame) != ESP_OK){
        ESP_LOGE(APP_MAIN_TAG, "Initializing  mw-frame failed!\n");
        return;
    }

    xTaskCreate(
        task_controller,
        "INTERFACE",
        2048U,
        (void*) &mw_frame,
        tskIDLE_PRIORITY,
        &handle_task_controller
    );

    xTaskCreate(
        task_controller,
        "INTERFACE",
        2048U,
        (void*) &mw_frame,
        tskIDLE_PRIORITY,
        &handle_task_bt
    );
}
