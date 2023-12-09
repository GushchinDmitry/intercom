/*
 * icom, Copyright (c) 2023
 */

#ifndef _ICOM_LED_H_
#define _ICOM_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "display_service.h"

/**
 * @brief Configure LED peripheral
 */
void icom_led_config(void);


void icom_led_WIFI_set(void);
void icom_led_WIFI_reset(void);

void icom_led_VOIP_set(void);
void icom_led_VOIP_reset(void);

void icom_led_CALL_set(void);
void icom_led_CALL_reset(void);

void icom_led_ANSW_set(void);
void icom_led_ANSW_reset(void);

void icom_led_OPEN_set(void);
void icom_led_OPEN_reset(void);



/**
 * @brief Initialize led peripheral and display service
 *
 * @return The display service handle
 */
display_service_handle_t icom_led_init(void);

#ifdef __cplusplus
}
#endif

#endif