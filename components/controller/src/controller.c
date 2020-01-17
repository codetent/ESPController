#include "controller.h"


/* -------------------------------------------------------------------------- */
/*                                   DEFINES                                  */
/* -------------------------------------------------------------------------- */

#define CONTROLLER_ADC_ATTEN ADC_ATTEN_DB_11


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

esp_err_t controller_configure(controller_t *controller)
{
    esp_err_t status = ESP_FAIL;
    gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };

    io_conf.pin_bit_mask = (1ULL << (uint32_t)controller->config.arm_gpio_num) |
                        (1ULL << (uint32_t)controller->config.thr_down_gpio_num) |
                        (1ULL << (uint32_t)controller->config.thr_up_gpio_num);

    if ((status = gpio_config(&io_conf)) == ESP_OK)
    {
        if ((status = adc1_config_channel_atten(controller->config.x_adc_channel, CONTROLLER_ADC_ATTEN)) == ESP_OK)
        {
            if ((status = adc1_config_channel_atten(controller->config.y_adc_channel, CONTROLLER_ADC_ATTEN)) == ESP_OK)
            {
                controller->_initialized = true;

                if ((status = controller_read_raw(controller)) == ESP_OK)
                {
                    controller->x_origin = controller->x_value;
                    controller->y_origin = controller->y_value;
                    controller->x_value = 0U;
                    controller->y_value = 0U;
                    controller->arm_value = 0U;
                    controller->thr_up_value = 0U;
                    controller->thr_down_value = 0U;

                    status = ESP_OK;
                }
            }
        }
    }

    return status;
}

esp_err_t controller_read_raw(controller_t *controller)
{
    esp_err_t status = ESP_FAIL;

    if (controller->_initialized == true)
    {
        controller->x_value = (uint32_t)adc1_get_raw(controller->config.x_adc_channel);
        controller->y_value = (uint32_t)adc1_get_raw(controller->config.y_adc_channel);
        controller->arm_value = (uint32_t)gpio_get_level(controller->config.arm_gpio_num);
        controller->thr_down_value = (uint32_t)gpio_get_level(controller->config.thr_down_gpio_num);
        controller->thr_up_value = (uint32_t)gpio_get_level(controller->config.thr_up_gpio_num);

        status = ESP_OK;
    }

    return status;
}

esp_err_t controller_calc_pos(controller_t *controller, uint32_t threshold, controller_position_t *position)
{
    esp_err_t status = ESP_FAIL;
    uint32_t x_delta = (uint32_t)abs(controller->x_value - controller->x_origin);
    uint32_t y_delta = (uint32_t)abs(controller->y_value - controller->y_origin);

    if (controller->_initialized == true)
    {
        if (x_delta > threshold)
        {
            position->x_position = (controller->x_value < controller->x_origin) ? CONTROLLER_POS_1 : CONTROLLER_POS_2;
            position->x_delta = x_delta;
        }
        else
        {
            position->x_position = CONTROLLER_POS_NEUTRAL;
            position->x_delta = 0U;
        }

        if (y_delta > threshold)
        {
            position->y_position = (controller->y_value < controller->y_origin) ? CONTROLLER_POS_1 : CONTROLLER_POS_2;
            position->y_delta = y_delta;
        }
        else
        {
            position->y_position = CONTROLLER_POS_NEUTRAL;
            position->y_delta = 0U;
        }

        status = ESP_OK;
    }

    return status;
}
