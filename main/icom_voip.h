/*
 * icom, Copyright (c) 2023
 */

#ifndef _ICOM_VOIP_H_
#define _ICOM_VOIP_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "esp_sip.h"


void sip_service_create();
sip_state_t sip_state_get();


#ifdef __cplusplus
}
#endif

#endif