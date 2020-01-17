#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "esp_err.h"

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
#define THROTTLE_STEP 50U

#define BT_DEVICE_DRONE "XMC-Bluetooth"

#define CTR_TASK_TAG "CTR_TASK"
#define BT_TASK_TAG "BT_TASK"
#define APP_MAIN_TAG "APP_MAIN"

static const esp_bt_inq_mode_t inq_mode = ESP_BT_INQ_MODE_GENERAL_INQUIRY;
static const uint8_t inq_len = 30;
static const uint8_t inq_num_rsps = 0;

static esp_bd_addr_t peer_bd_addr;
static uint8_t peer_bdname_len;
static char peer_bdname[ESP_BT_GAP_MAX_BDNAME_LEN + 1];

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_NONE;
static const esp_spp_role_t role_master = ESP_SPP_ROLE_SLAVE;

static  uint32_t* spp_handle = NULL;


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (INTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

static bool get_name_from_eir(uint8_t *eir, char *bdname, uint8_t *bdname_len)
{
    bool status = false;
    uint8_t *rmt_bdname = NULL;
    uint8_t rmt_bdname_len = 0;

    if (!eir) {
        status = false;
    }else{
        rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &rmt_bdname_len);
        if (!rmt_bdname) {
            rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &rmt_bdname_len);
        }

        if (rmt_bdname) {
            if (rmt_bdname_len > ESP_BT_GAP_MAX_BDNAME_LEN) {
                rmt_bdname_len = ESP_BT_GAP_MAX_BDNAME_LEN;
            }

            if (bdname) {
                memcpy(bdname, rmt_bdname, rmt_bdname_len);
                bdname[rmt_bdname_len] = '\0';
            }
            if (bdname_len) {
                *bdname_len = rmt_bdname_len;
            }
            status = true;
        }
    }

    return status;
}

//GAP callback search for drone and conects t it
static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch(event){
    case ESP_BT_GAP_DISC_RES_EVT:
        for (int i = 0; i < param->disc_res.num_prop; i++){
            // Check if gat type 
            if (param->disc_res.prop[i].type == ESP_BT_GAP_DEV_PROP_EIR
                && get_name_from_eir(param->disc_res.prop[i].val, peer_bdname, &peer_bdname_len)){
                esp_log_buffer_char(BT_TASK_TAG, peer_bdname, peer_bdname_len);
                //Check if name is XMC_Bluetooth
                if (strlen(BT_DEVICE_DRONE) == peer_bdname_len
                    && strncmp(peer_bdname, BT_DEVICE_DRONE, peer_bdname_len) == 0) {
                    // Start spp discovery
                    memcpy(peer_bd_addr, param->disc_res.bda, ESP_BD_ADDR_LEN);
                    esp_spp_start_discovery(peer_bd_addr);
                    esp_bt_gap_cancel_discovery();
                }
            }
        }
        break;
    default:
        break;
    }
}

// Serial port profile callback
static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{   
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(BT_TASK_TAG, "ESP_SPP_INIT_EVT");
        esp_bt_dev_set_device_name("ESP32");
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        esp_bt_gap_start_discovery(inq_mode, inq_len, inq_num_rsps);

        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        // Connect to SPP server
        ESP_LOGI(BT_TASK_TAG, "ESP_SPP_DISCOVERY_COMP_EVT");
        if (param->disc_comp.status == ESP_SPP_SUCCESS) {
            esp_spp_connect(sec_mask, role_master, param->disc_comp.scn[0], peer_bd_addr);
        }
        break;
    case ESP_SPP_OPEN_EVT:
        // Set handle
        ESP_LOGI(BT_TASK_TAG, "ESP_SPP_OPEN_EVT");
        spp_handle = &param->open.handle;
        
        break;
    case ESP_SPP_CLOSE_EVT:
        // Set handle to NULL
        ESP_LOGI(BT_TASK_TAG, "ESP_SPP_CLOSE_EVT");
        if(*spp_handle == param->close.handle)
            spp_handle = NULL;

        break;
    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(BT_TASK_TAG, "ESP_SPP_WRITE_EVT");
        break;
    default:
        break;
    }
}

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
            ESP_LOGI(CTR_TASK_TAG, "ARM pressed!");
            ESP_ERROR_CHECK(mw_toggle_arm(mw_frame));
        }
        if(controller.thr_down_value && !last_thr_down_value){
            ESP_LOGI(CTR_TASK_TAG, "THR_DOWN pressed!");
            if((throttle - THROTTLE_STEP) <= MW_MIN_VALUE){
                throttle = MW_MIN_VALUE;
            }else{
                throttle -= THROTTLE_STEP;
            }
            ESP_ERROR_CHECK(mw_set_throttle(throttle, mw_frame));
        }
        if(controller.thr_up_value && !last_thr_up_value){
            ESP_LOGI(CTR_TASK_TAG, "THR_UP pressed!");
            if((throttle + THROTTLE_STEP) >= MW_MAX_VALUE){
                throttle = MW_MAX_VALUE;
            }else{
                throttle += THROTTLE_STEP;
            }
            ESP_ERROR_CHECK(mw_set_throttle(throttle, mw_frame));
        }
        last_arm_value = controller.arm_value;
        last_thr_down_value = controller.thr_down_value;
        last_thr_up_value = controller.thr_up_value;

        // Check joystick values
        if(position.x_position == 0U){
            roll = MW_MID_VALUE;
        }else if(position.x_position == 1U){
            roll = MW_MID_VALUE - position.x_delta / 3U;
        }else if(position.x_position == 2U){
            roll = MW_MID_VALUE + position.x_delta / 3U;
        }
        ESP_ERROR_CHECK(mw_set_roll(roll, mw_frame));

        if(position.y_position == 0U){
            pitch = MW_MID_VALUE;
        }else if(position.y_position == 1U){
            pitch = MW_MID_VALUE - position.y_delta / 3U;
        }else if(position.y_position == 2U){
            pitch = MW_MID_VALUE + position.y_delta / 3U;
        }
        ESP_ERROR_CHECK(mw_set_pitch(pitch, mw_frame));

        ESP_LOGI(CTR_TASK_TAG, "ROLL: %d, PITCH: %d, THOTTLE: %d\n", roll, pitch, throttle);

        vTaskDelay(pdMS_TO_TICKS(100U));
    }

    vTaskDelete(NULL);
}

static void task_bt(void *args)
{
    mw_frame_t *mw_frame = (mw_frame_t*) args;
    esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

    // Init bluetooth

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));

    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));

    ESP_ERROR_CHECK(esp_bluedroid_init());

    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_bt_gap_register_callback(esp_bt_gap_cb));

    ESP_ERROR_CHECK(esp_spp_register_callback(esp_spp_cb));

    ESP_ERROR_CHECK(esp_spp_init(esp_spp_mode));
    

    while (1U) 
    {
        if(spp_handle != NULL){
            ESP_ERROR_CHECK(esp_spp_write(*spp_handle, MW_PROTO_FRAME_LEN, mw_frame->data));
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

    ESP_ERROR_CHECK(init_mw_frame(&mw_frame));

    xTaskCreate(
        task_controller,
        "CONTROLLER",
        2048U,
        (void*) &mw_frame,
        tskIDLE_PRIORITY,
        &handle_task_controller
    );

    xTaskCreate(
        task_bt,
        "BT",
        2048U,
        (void*) &mw_frame,
        tskIDLE_PRIORITY,
        &handle_task_bt
    );
}
