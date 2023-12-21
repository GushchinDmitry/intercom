

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "display_service.h"
#include "led_indicator.h"

#include "icom_ctrl.h"

static const char *TAG = "Icom_Ctrl";

#define ICOM_IN_CALL    GPIO_NUM_33

#define ICOM_OUT_ANSW   GPIO_NUM_21
#define ICOM_OUT_OPEN   GPIO_NUM_12


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
