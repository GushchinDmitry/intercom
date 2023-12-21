/*
 * icom, Copyright (c) 2023
 */

#ifndef _ICOM_CTRL_H_
#define _ICOM_CTRL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "display_service.h"


void icom_ctrl_config(void);

int icom_input_CALL_get(void);

void icom_output_ANSW_set(void);
void icom_output_ANSW_reset(void);

void icom_output_OPEN_set(void);
void icom_output_OPEN_reset(void);

#ifdef __cplusplus
}
#endif

#endif