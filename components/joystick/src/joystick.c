#include "joystick.h"


/* -------------------------------------------------------------------------- */
/*                                   DEFINES                                  */
/* -------------------------------------------------------------------------- */

#define JOYSTICK_ADC_ATTEN ADC_ATTEN_DB_11


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

esp_err_t joystick_configure(joystick_t *joystick)
{
    esp_err_t status = ESP_FAIL;
    gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0U,
        .pull_up_en = 1U
    };

    io_conf.pin_bit_mask = (1U << (uint32_t)joystick->config.z_gpio_num);

    if ((status = gpio_config(&io_conf)) == ESP_OK)
    {
        if ((status = adc1_config_channel_atten(joystick->config.x_adc_channel, JOYSTICK_ADC_ATTEN)) == ESP_OK)
        {
            if ((status = adc1_config_channel_atten(joystick->config.y_adc_channel, JOYSTICK_ADC_ATTEN)) == ESP_OK)
            {
                joystick->_initialized = true;

                if ((status = joystick_read_raw(joystick)) == ESP_OK)
                {
                    joystick->x_origin = joystick->x_value;
                    joystick->y_origin = joystick->y_value;
                    joystick->x_value = 0U;
                    joystick->y_value = 0U;
                    joystick->z_value = 0U;

                    status = ESP_OK;
                }
            }
        }
    }

    return status;
}

esp_err_t joystick_read_raw(joystick_t *joystick)
{
    esp_err_t status = ESP_FAIL;

    if (joystick->_initialized == true)
    {
        joystick->x_value = (uint32_t)adc1_get_raw(joystick->config.x_adc_channel);
        joystick->y_value = (uint32_t)adc1_get_raw(joystick->config.y_adc_channel);
        joystick->z_value = (uint32_t)gpio_get_level(joystick->config.z_gpio_num);

        status = ESP_OK;
    }

    return status;
}

esp_err_t joystick_calc_pos(joystick_t *joystick, uint32_t threshold, joystick_position_t *position)
{
    esp_err_t status = ESP_FAIL;
    uint32_t x_delta = (uint32_t)abs(joystick->x_value - joystick->x_origin);
    uint32_t y_delta = (uint32_t)abs(joystick->y_value - joystick->y_origin);

    if (joystick->_initialized == true)
    {
        if (x_delta > threshold)
        {
            position->x_position = (joystick->x_value < joystick->x_origin) ? JOYSTICK_POS_1 : JOYSTICK_POS_2;
            position->x_delta = x_delta;
        }
        else
        {
            position->x_position = JOYSTICK_POS_NEUTRAL;
            position->x_delta = 0U;
        }

        if (y_delta > threshold)
        {
            position->y_position = (joystick->y_value < joystick->y_origin) ? JOYSTICK_POS_1 : JOYSTICK_POS_2;
            position->y_delta = y_delta;
        }
        else
        {
            position->y_position = JOYSTICK_POS_NEUTRAL;
            position->y_delta = 0U;
        }

        status = ESP_OK;
    }

    return status;
}
