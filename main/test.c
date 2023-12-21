

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"


#include "icom_pins.h"
#include "icom_led.h"
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


/*
void blink_test_task()
{
    //  Configure the peripheral according to the LED type

    ESP_LOGI(TAG, "Configured to blink GPIO LED!");

    gpio_reset_pin(ICOM_LED_WIFI);

    //  Set the GPIO as a push/pull output
    gpio_set_direction(ICOM_LED_WIFI, GPIO_MODE_OUTPUT);


    //  Configure the icom peripheral (GPIO)
    ESP_LOGI(TAG, "Configured the GPIO peripheral");
    gpio_reset_pin(ICOM_ANSWER);
    gpio_set_direction(ICOM_ANSWER, GPIO_MODE_OUTPUT);
    //gpio_set_level(ICOM_ANSWER, true);

    gpio_reset_pin(ICOM_CALL);
    gpio_set_direction(ICOM_CALL, GPIO_MODE_INPUT);
    gpio_get_level(ICOM_CALL);


    while (1) 
    {
        //  Toggle the LED state

        //s_led_state = !s_led_state;

        //ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        
        //  Set the GPIO level according to the state (LOW or HIGH)
        //gpio_set_level(ICOM_LED_WIFI, true);
        //gpio_set_level(ICOM_ANSWER, s_led_state);

        if (gpio_get_level(!ICOM_CALL))
        {
            ESP_LOGI(TAG, "Input CALL !!!");
            gpio_set_level(ICOM_LED_CALL, true);
            gpio_set_level(ICOM_ANSWER, false);            
        }
        else
        {
            gpio_set_level(ICOM_LED_CALL, false);
        }

        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }

}
*/