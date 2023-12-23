

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "input_key_service.h"

#include "icom_pins.h"
#include "icom_ctrl.h"

static const char *TAG = "Icom_Ctrl";


/**
 * @brief Configure IO peripheral
 */
void icom_ctrl_config()
{
   //  Configure the icom peripheral (GPIO)
    ESP_LOGI(TAG, "Configured the GPIO peripheral");

    gpio_reset_pin(ICOM_IN_CALL);
    gpio_set_direction(ICOM_IN_CALL, GPIO_MODE_INPUT);

    gpio_reset_pin(ICOM_OUT_ANSW);
    gpio_set_direction(ICOM_OUT_ANSW, GPIO_MODE_OUTPUT);
    gpio_set_level(ICOM_OUT_ANSW, false);

    gpio_reset_pin(ICOM_OUT_OPEN);
    gpio_set_direction(ICOM_OUT_OPEN, GPIO_MODE_OUTPUT);
    gpio_set_level(ICOM_OUT_OPEN, false);
}

int icom_input_CALL_get()       {   return gpio_get_level(ICOM_IN_CALL);    }

void icom_output_ANSW_set()     {   gpio_set_level(ICOM_OUT_ANSW, true);    }
void icom_output_ANSW_reset()   {   gpio_set_level(ICOM_OUT_ANSW, false);   }

void icom_output_OPEN_set()     {   gpio_set_level(ICOM_OUT_OPEN, true);    }
void icom_output_OPEN_reset()   {   gpio_set_level(ICOM_OUT_OPEN, false);   }


#if 0
__attribute__ ((used)) static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
/*    
    audio_board_handle_t board_handle = (audio_board_handle_t) ctx;
    int player_volume;
*/
    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
        ESP_LOGD(TAG, "[ * ] input key id is %d", (int)evt->data);
/*        
        sip_state_t sip_state = esp_sip_get_state(sip);
        if (sip_state < SIP_STATE_REGISTERED) {
            return ESP_OK;
        }
*/
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_REC:
            // case INPUT_KEY_USER_ID_MUTE:
                ESP_LOGI(TAG, "[ * ] [Rec] Set MIC Mute or not");
/*
                if (mute) {
                    mute = false;
                    // display_service_set_pattern(disp, DISPLAY_PATTERN_TURN_OFF, 0);
                } else {
                    mute = true;
                    // display_service_set_pattern(disp, DISPLAY_PATTERN_TURN_ON, 0);
                }
*/                
                break;
/*                
            case INPUT_KEY_USER_ID_PLAY:
                ESP_LOGI(TAG, "[ * ] [Play] input key event (RecBtn 1)");
                if (sip_state == SIP_STATE_RINGING) {
                    // audio_player_int_tone_stop();
                    esp_sip_uas_answer(sip, true);
                } else if (sip_state == SIP_STATE_REGISTERED) {
                    esp_sip_uac_invite(sip, "101");
                }
                break;
*/
            case INPUT_KEY_USER_ID_MODE:
            /*case INPUT_KEY_USER_ID_PLAY:*/
                ESP_LOGI(TAG, "[ * ] [Mode] input key event (RecBtn)");
/*                
                if (sip_state == SIP_STATE_RINGING) {
                    // audio_player_int_tone_stop();
                    esp_sip_uas_answer(sip, true);
                } else if (sip_state == SIP_STATE_REGISTERED) {
                    esp_sip_uac_invite(sip, "101");
                }
*/
                break;
/*
            case INPUT_KEY_USER_ID_SET:
                if (sip_state & SIP_STATE_RINGING) {
                    // audio_player_int_tone_stop();
                    esp_sip_uas_answer(sip, false);
                } else if (sip_state & SIP_STATE_ON_CALL) {
                    esp_sip_uac_bye(sip);
                } else if ((sip_state & SIP_STATE_CALLING) || (sip_state & SIP_STATE_SESS_PROGRESS)) {
                    esp_sip_uac_cancel(sip);
                }
                break;
            case INPUT_KEY_USER_ID_VOLUP:
                ESP_LOGD(TAG, "[ * ] [Vol+] input key event");
                audio_hal_get_volume(board_handle->audio_hal, &player_volume);
                player_volume += 10;
                if (player_volume > 100) {
                    player_volume = 100;
                }
                audio_hal_set_volume(board_handle->audio_hal, player_volume);
                ESP_LOGI(TAG, "[ * ] Volume set to %d %%", player_volume);
                break;
            case INPUT_KEY_USER_ID_VOLDOWN:
                ESP_LOGD(TAG, "[ * ] [Vol-] input key event");
                audio_hal_get_volume(board_handle->audio_hal, &player_volume);
                player_volume -= 10;
                if (player_volume < 0) {
                    player_volume = 0;
                }
                audio_hal_set_volume(board_handle->audio_hal, player_volume);
                ESP_LOGI(TAG, "[ * ] Volume set to %d %%", player_volume);
                break;
*/        
        }
    } else if (evt->type == INPUT_KEY_SERVICE_ACTION_PRESS) {
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_SET:
/*
                is_smart_config = true;
                esp_sip_destroy(sip);
*/
                // wifi_service_setting_start(wifi_serv, 0);

                // audio_player_int_tone_play(tone_uri[TONE_TYPE_UNDER_SMARTCONFIG]);
                break;
        }
    }

    return ESP_OK;
}
#endif