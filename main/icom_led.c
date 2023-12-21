/* 

    SIP Intercom system

*/

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "icom_pins.h"

#include "display_service.h"
#include "led_indicator.h"

#include "icom_led.h"

static const char *TAG = "Icom_LED";


static display_service_handle_t disp_led_WiFi;                        // LED WiFi


/**
 * @brief Configure LED peripheral
 */
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


/**
 * @brief Initialize led peripheral and display service
 *
 * @return The display service handle
 */
display_service_handle_t disp_led_WiFi_init(void)
{
    led_indicator_handle_t led = led_indicator_init((gpio_num_t) ICOM_LED_WIFI);
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
            .service_name = "LED_WiFi_serv",
            .user_data = NULL,
        },
        .instance = led,
    };
    return display_service_create(&display);
}


/**
 * @brief Create WiFi LED task
 */
void icom_led_WiFi_init()
{
    ESP_LOGI(TAG, "Create LED WiFi service instance");
    disp_led_WiFi = disp_led_WiFi_init();
}


/**
 * @brief WiFi LED On
 */
void icom_led_WiFi_On()
{
    display_service_set_pattern(disp_led_WiFi, DISPLAY_PATTERN_TURN_ON, 0);
}


/**
 * @brief WiFi LED Off
 */
void icom_led_WiFi_Off()
{
    display_service_set_pattern(disp_led_WiFi, DISPLAY_PATTERN_TURN_OFF, 0);
}
