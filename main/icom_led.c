/* 

    SIP Intercom system

*/

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "board_pins_def.h"

#include "display_service.h"
#include "led_indicator.h"

#include "icom_led.h"

static const char *TAG = "ICOM_LED";


void icom_led_config()
{
    //  Configure the peripheral according to the LED type

    ESP_LOGI(TAG, "Configured LEDs!");

    gpio_reset_pin(ICOM_LED_WIFI);
    gpio_set_direction(ICOM_LED_WIFI, GPIO_MODE_OUTPUT);

    gpio_reset_pin(ICOM_LED_VOIP);
    gpio_set_direction(ICOM_LED_VOIP, GPIO_MODE_OUTPUT);

    gpio_reset_pin(ICOM_LED_CALL);
    gpio_set_direction(ICOM_LED_CALL, GPIO_MODE_OUTPUT);

    gpio_reset_pin(ICOM_LED_ANSW);
    gpio_set_direction(ICOM_LED_ANSW, GPIO_MODE_OUTPUT);

    gpio_reset_pin(ICOM_LED_OPEN);
    gpio_set_direction(ICOM_LED_OPEN, GPIO_MODE_OUTPUT);

    gpio_set_level(ICOM_LED_WIFI, false);
    gpio_set_level(ICOM_LED_VOIP, false);
    gpio_set_level(ICOM_LED_CALL, false);
    gpio_set_level(ICOM_LED_ANSW, false);
    gpio_set_level(ICOM_LED_OPEN, false);

}

void icom_led_WIFI_set()    {   gpio_set_level(ICOM_LED_WIFI, true);    }
void icom_led_WIFI_reset()  {   gpio_set_level(ICOM_LED_WIFI, false);   }

void icom_led_VOIP_set()    {   gpio_set_level(ICOM_LED_VOIP, true);    }
void icom_led_VOIP_reset()  {   gpio_set_level(ICOM_LED_VOIP, false);   }

void icom_led_CALL_set()    {   gpio_set_level(ICOM_LED_CALL, true);    }
void icom_led_CALL_reset()  {   gpio_set_level(ICOM_LED_CALL, false);   }

void icom_led_ANSW_set()    {   gpio_set_level(ICOM_LED_ANSW, true);    }
void icom_led_ANSW_reset()  {   gpio_set_level(ICOM_LED_ANSW, false);   }

void icom_led_OPEN_set()    {   gpio_set_level(ICOM_LED_OPEN, true);    }
void icom_led_OPEN_reset()  {   gpio_set_level(ICOM_LED_OPEN, false);   }


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


#define LED_GPIO_22 GPIO_NUM_22

display_service_handle_t icom_led_init(void)
{
    led_indicator_handle_t led = led_indicator_init((gpio_num_t) GPIO_NUM_22);
    display_service_config_t display = {
        .based_cfg = {
            .task_stack = 0,
            .task_prio  = 0,
            .task_core  = 0,
            .task_func  = NULL,
            .service_start = NULL,
            .service_stop = NULL,
            .service_destroy = NULL,
            .service_ioctl = led_indicator_pattern,
            .service_name = "LED_22_serv",
            .user_data = NULL,
        },
        .instance = led,
    };

    return display_service_create(&display);
}
