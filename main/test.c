

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"


#include "board_pins_def.h"
#include "test.h"


static const char *TAG = "test";


#define ICOM_IN_CALL    GPIO_NUM_33

#define ICOM_OUT_ANSW   GPIO_NUM_21
#define ICOM_OUT_OPEN   GPIO_NUM_12


#define CONFIG_BLINK_PERIOD 100

//static uint8_t s_led_state = 0;


void test()
{
    ESP_LOGI(TAG, "!!!!!     TEST     !!!!!");


    while (1) 
    {
        //  Toggle the LED state

        //s_led_state = !s_led_state;

        //ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        



        //wifi_service_set_sta_info(periph_service_handle_t handle, wifi_config_t *info);




/*
        if (gpio_get_level(ICOM_IN_CALL))
        {
            // обратная логика: если true, то нет вызова
            gpio_set_level(ICOM_LED_CALL, false);
        }
        else
        {
            ESP_LOGI(TAG, "Input CALL !!!");
            gpio_set_level(ICOM_LED_CALL, true);
        }
*/
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
    
}
