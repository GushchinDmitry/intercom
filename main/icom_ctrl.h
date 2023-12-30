/*
 * icom, Copyright (c) 2023
 */

#ifndef _ICOM_CTRL_H_
#define _ICOM_CTRL_H_

#ifdef __cplusplus
extern "C" {
#endif


void icom_ctrl_config(void);

int icom_input_CALL_get(void);

void icom_output_ANSW_set(void);
void icom_output_ANSW_reset(void);

void icom_output_OPEN_set(void);
void icom_output_OPEN_reset(void);


//static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx);


int8_t get_input_rec_id(void);
int8_t get_input_mode_id(void);


esp_err_t audio_board_key_init(esp_periph_set_handle_t set);


#ifdef __cplusplus
}
#endif

#endif