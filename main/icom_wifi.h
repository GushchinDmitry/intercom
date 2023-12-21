/*
 * icom, Copyright (c) 2023
 */

#ifndef _ICOM_WIFI_H_
#define _ICOM_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif


static bool is_smart_config;



/**
 * @brief Initialize WiFi
 *
 * @return The display service handle
 */
void setup_wifi();


#ifdef __cplusplus
}
#endif

#endif