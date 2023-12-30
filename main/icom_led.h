/*
 * icom, Copyright (c) 2023
 */

#ifndef _ICOM_LED_H_
#define _ICOM_LED_H_

#ifdef __cplusplus
extern "C" {
#endif


void icom_led_config(void);


void icom_led_VOIP_set(void);
void icom_led_VOIP_reset(void);

void icom_led_CALL_set(void);
void icom_led_CALL_reset(void);

void icom_led_ANSW_set(void);
void icom_led_ANSW_reset(void);

void icom_led_OPEN_set(void);
void icom_led_OPEN_reset(void);


void icom_led_WiFi_init();
void icom_led_WiFi_On();
void icom_led_WiFi_Off();
void icom_led_WiFi_Toggle();


#ifdef __cplusplus
}
#endif

#endif