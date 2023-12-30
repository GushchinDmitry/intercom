

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "input_key_service.h"
#include "periph_button.h"

#include "icom_pins.h"
#include "icom_ctrl.h"


static const char *TAG = "Icom_Ctrl";


/**
 * @brief Configure IO peripheral
 */
void icom_ctrl_config()
{
   //  Configure the icom peripheral (GPIO)
    ESP_LOGI(TAG, "Configured the GPIO peripheral");

    gpio_reset_pin(ICOM_IN_CALL);
    gpio_set_direction(ICOM_IN_CALL, GPIO_MODE_INPUT);

    gpio_reset_pin(ICOM_OUT_ANSW);
    gpio_set_direction(ICOM_OUT_ANSW, GPIO_MODE_OUTPUT);
    gpio_set_level(ICOM_OUT_ANSW, false);

    gpio_reset_pin(ICOM_OUT_OPEN);
    gpio_set_direction(ICOM_OUT_OPEN, GPIO_MODE_OUTPUT);
    gpio_set_level(ICOM_OUT_OPEN, false);
}

int icom_input_CALL_get()       {   return gpio_get_level(ICOM_IN_CALL);    }

void icom_output_ANSW_set()     {   gpio_set_level(ICOM_OUT_ANSW, true);    }
void icom_output_ANSW_reset()   {   gpio_set_level(ICOM_OUT_ANSW, false);   }

void icom_output_OPEN_set()     {   gpio_set_level(ICOM_OUT_OPEN, true);    }
void icom_output_OPEN_reset()   {   gpio_set_level(ICOM_OUT_OPEN, false);   }


#if 0
__attribute__ ((used)) static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
#endif




// button pins


/**
 * @brief  Get the record-button id for adc-button
 *
 * @return  -1      non-existent
 *          Others  button id
 */
int8_t get_input_rec_id(void)
{
    return BTN_REC_ID;
}

/**
 * @brief  Get the number for mode-button
 *
 * @return  -1      non-existent
 *          Others  number
 */
int8_t get_input_mode_id(void)
{
    return BTN_MODE_ID;
}


/**
 * @brief Initialize key peripheral
 *
 * @param set The handle of esp_periph_set_handle_t
 *
 * @return
 *     - ESP_OK, success
 *     - Others, fail
 */
esp_err_t audio_board_key_init(esp_periph_set_handle_t set)
{
    periph_button_cfg_t btn_cfg = {
        .gpio_mask = (1ULL << get_input_rec_id()) | (1ULL << get_input_mode_id()), //REC BTN & MODE BTN
    };
    esp_periph_handle_t button_handle = periph_button_init(&btn_cfg);
    AUDIO_NULL_CHECK(TAG, button_handle, return ESP_ERR_ADF_MEMORY_LACK);
    esp_err_t ret = ESP_OK;
    ret = esp_periph_start(set, button_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    return ret;
}