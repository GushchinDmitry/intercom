/* 

    SIP Intercom system

*/

#include <string.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "audio_mem.h"
#include "i2s_stream.h"
#include "esp_peripherals.h"
#include "periph_wifi.h"
#include "board.h"
#include "es8374.h"
#include "input_key_service.h"
 
#include "raw_stream.h"
#include "filter_resample.h"
#include "esp_sip.h"
#include "g711_decoder.h"
#include "g711_encoder.h"
#include "algorithm_stream.h"
#include "fatfs_stream.h"
#include "wav_encoder.h"

#include "wifi_service.h"
#include "smart_config.h"

// #include "audio_tone_uri.h"
// #include "audio_player_int_tone.h"
#include "media_lib_adapter.h"
#include "audio_idf_version.h"

#include "esp_netif.h"


#include "icom_led.h"
#include "icom_ctrl.h"
#include "icom_wifi.h"
#include "icom_voip.h"


#include "test.h"


#define WIFI_SSID       "WiFi.net"
#define WIFI_PASSWORD   "Pass"

//  udp://1004:1234@10.10.34.9:5060
//  udp://1005:1234@sip.intercom.host:7060


static const char *TAG = "Icom";



/* The AEC internal buffering mechanism requires that the recording signal
   is delayed by around 0 - 10 ms compared to the corresponding reference (playback) signal. */
#define DEFAULT_REF_DELAY_MS    0
#define DEFAULT_REC_DELAY_MS    50

// static sip_handle_t sip;
// static audio_element_handle_t raw_read, raw_write, element_algo;
// static audio_pipeline_handle_t recorder, player;
static bool mute;
// static bool is_smart_config;
// static display_service_handle_t disp;
// static periph_service_handle_t wifi_serv;
// static display_service_handle_t disp_led_WiFi;                        // LED WiFi


static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    /*
    audio_board_handle_t board_handle = (audio_board_handle_t) ctx;
    int player_volume;
    */

    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
        ESP_LOGD(TAG, "[ * ] input key id is %d", (int)evt->data);
        sip_state_t sip_state = sip_state_get();                        // = esp_sip_get_state(sip);
        if (sip_state < SIP_STATE_REGISTERED) {
            return ESP_OK;
        }
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_REC:
            // case INPUT_KEY_USER_ID_MUTE:
                ESP_LOGI(TAG, "[ * ] [Rec] Set MIC Mute or not");
                if (mute) {
                    mute = false;
                    // display_service_set_pattern(disp, DISPLAY_PATTERN_TURN_OFF, 0);
                } else {
                    mute = true;
                    // display_service_set_pattern(disp, DISPLAY_PATTERN_TURN_ON, 0);
                }
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
                if (sip_state == SIP_STATE_RINGING) {
                    // audio_player_int_tone_stop();
//                    esp_sip_uas_answer(sip, true);
                } else if (sip_state == SIP_STATE_REGISTERED) {
//                    esp_sip_uac_invite(sip, "101");
                }
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
                is_smart_config = true;
//                esp_sip_destroy(sip);
                // wifi_service_setting_start(wifi_serv, 0);

                // audio_player_int_tone_play(tone_uri[TONE_TYPE_UNDER_SMARTCONFIG]);
                break;
        }
    }

    return ESP_OK;
}



/////       TEST        /////


/////       END TEST        /////


void app_main()
{

    esp_log_level_set("*", ESP_LOG_INFO);
    media_lib_add_default_adapter();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
// #if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
    ESP_ERROR_CHECK(esp_netif_init());
// #else
//     tcpip_adapter_init();
// #endif


    ESP_LOGI(TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[1.1] Initialize and start peripherals");
    audio_board_key_init(set);                                                      // инициализация кнопок управления (REC BTN & MODE BTN ++ TOUCH_PAD_SELх)

    ESP_LOGI(TAG, "[1.2] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();           //  передается перечень кнопок управления
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = set;
    input_cfg.based_cfg.task_stack = 4 * 1024;
    periph_service_handle_t input_ser = input_key_service_create(&input_cfg);       //  создание задачи обработки кнопок


    ESP_LOGI(TAG, "[ 1.3 ] Create display service instance");
    ESP_LOGI(TAG, "[ 1.4 ] Configure LEDs and Create LED service instance");
    icom_led_config();                                                              //  Инициализация светодиодов индикации режимов работы
    icom_led_WiFi_init();                                                           //  Cоздание задачи WiFi LED индикатора


    ESP_LOGI(TAG, "[ 1.5 ] Configure IOs");
    icom_ctrl_config();                                                             // Инициализация дискретных входов/выходов


    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();

    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
    audio_hal_set_volume(board_handle->audio_hal, 80);

    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, input_key_service_cb, (void *)board_handle);

    // ESP_LOGI(TAG, "[ 3 ] Initialize tone player");
    // audio_player_int_tone_init();

    ESP_LOGI(TAG, "[ 4 ] Create Wi-Fi service instance");
    setup_wifi();

    ESP_LOGI(TAG, "[ XXXXX ] TEST");
    ESP_LOGI(TAG, "[ XXXXX ] TEST");
    ESP_LOGI(TAG, "[ XXXXX ] TEST");
    ESP_LOGI(TAG, "[ XXXXX ] TEST");
    ESP_LOGI(TAG, "[ XXXXX ] TEST");



    int vol;
    //int vol=65;
    //es8374_codec_set_voice_volume(vol);
    es8374_codec_get_voice_volume(&vol);
    ESP_LOGI(TAG, "[ XXXXX ] Volume = %d", vol);


    //  test();




    test();

}

