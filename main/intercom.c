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

#include "audio_tone_uri.h"
// #include "audio_player_int_tone.h"
#include "media_lib_adapter.h"
#include "audio_idf_version.h"

#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
#include "esp_netif.h"
#else
#include "tcpip_adapter.h"
#endif


#include "icom_led.h"
#include "icom_ctrl.h"


#include "test.h"

#define WIFI_SSID "WiFi.net"
#define WIFI_PASSWORD "Pass"


static const char *TAG = "Icom";

/* Debug original input data for AEC feature*/
// #define DEBUG_AEC_INPUT

#define I2S_SAMPLE_RATE     16000
#define I2S_CHANNELS        2
#define I2S_BITS            16

#define CODEC_SAMPLE_RATE    8000
#define CODEC_CHANNELS       1

/* The AEC internal buffering mechanism requires that the recording signal
   is delayed by around 0 - 10 ms compared to the corresponding reference (playback) signal. */
#define DEFAULT_REF_DELAY_MS    0
#define DEFAULT_REC_DELAY_MS    50

static sip_handle_t sip;
static audio_element_handle_t raw_read, raw_write, element_algo;
static audio_pipeline_handle_t recorder, player;
static bool mute, is_smart_config;
static display_service_handle_t disp;
static periph_service_handle_t wifi_serv;
static display_service_handle_t disp_led_22;                        // LED 22

static esp_err_t recorder_pipeline_open()
{
    audio_element_handle_t i2s_stream_reader;
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    recorder = audio_pipeline_init(&pipeline_cfg);
    AUDIO_NULL_CHECK(TAG, recorder, return ESP_FAIL);

    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_READER;
    i2s_cfg.uninstall_drv = false;
// #ifdef CONFIG_ESP_LYRAT_MINI_V1_1_BOARD
//     i2s_cfg.i2s_port = 1;
// #endif
    i2s_cfg.task_core = 1;
    i2s_cfg.i2s_config.sample_rate = I2S_SAMPLE_RATE;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);

    rsp_filter_cfg_t rsp_cfg_r = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg_r.src_rate = I2S_SAMPLE_RATE;
    rsp_cfg_r.src_ch = I2S_CHANNELS;
    rsp_cfg_r.dest_rate = I2S_SAMPLE_RATE;
// #ifndef CONFIG_ESP_LYRAT_MINI_V1_1_BOARD
//     rsp_cfg_r.dest_ch = 1;
// #endif
    rsp_cfg_r.complexity = 5;
    rsp_cfg_r.task_core = 1;
    rsp_cfg_r.out_rb_size = 10 * 1024;
    audio_element_handle_t filter_r = rsp_filter_init(&rsp_cfg_r);

    algorithm_stream_cfg_t algo_config = ALGORITHM_STREAM_CFG_DEFAULT();
// #ifdef CONFIG_ESP_LYRAT_MINI_V1_1_BOARD
//     algo_config.input_type = ALGORITHM_STREAM_INPUT_TYPE1;
// #else
    algo_config.input_type = ALGORITHM_STREAM_INPUT_TYPE2;
// #endif
    algo_config.task_core = 1;
// #ifdef DEBUG_AEC_INPUT
//     algo_config.debug_input = true;
// #endif
    element_algo = algo_stream_init(&algo_config);
    audio_element_set_music_info(element_algo, I2S_SAMPLE_RATE, 1, I2S_BITS);

    audio_pipeline_register(recorder, i2s_stream_reader, "i2s");
    audio_pipeline_register(recorder, filter_r, "filter_r");
    audio_pipeline_register(recorder, element_algo, "algo");

// #ifdef DEBUG_AEC_INPUT
//     wav_encoder_cfg_t wav_cfg = DEFAULT_WAV_ENCODER_CONFIG();
//     wav_cfg.task_core = 1;
//     audio_element_handle_t wav_encoder = wav_encoder_init(&wav_cfg);

//     fatfs_stream_cfg_t fatfs_wd_cfg = FATFS_STREAM_CFG_DEFAULT();
//     fatfs_wd_cfg.type = AUDIO_STREAM_WRITER;
//     fatfs_wd_cfg.task_core = 1;
//     audio_element_handle_t fatfs_stream_writer = fatfs_stream_init(&fatfs_wd_cfg);

//     audio_pipeline_register(recorder, wav_encoder, "wav_enc");
//     audio_pipeline_register(recorder, fatfs_stream_writer, "fatfs_stream");

//     const char *link_tag[5] = {"i2s", "filter_r", "algo", "wav_enc", "fatfs_stream"};
//     audio_pipeline_link(recorder, &link_tag[0], 5);

//     audio_element_info_t fat_info = {0};
//     audio_element_getinfo(fatfs_stream_writer, &fat_info);
//     fat_info.sample_rates = ALGORITHM_STREAM_DEFAULT_SAMPLE_RATE_HZ;
//     fat_info.bits = ALGORITHM_STREAM_DEFAULT_SAMPLE_BIT;
//     fat_info.channels = 2;
//     audio_element_setinfo(fatfs_stream_writer, &fat_info);
//     audio_element_set_uri(fatfs_stream_writer, "/sdcard/aec_in.wav");
// #else
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = I2S_SAMPLE_RATE;
    rsp_cfg.src_ch = 1;
    rsp_cfg.dest_rate = CODEC_SAMPLE_RATE;
    rsp_cfg.dest_ch = CODEC_CHANNELS;
    rsp_cfg.complexity = 5;
    rsp_cfg.task_core = 1;
    audio_element_handle_t filter = rsp_filter_init(&rsp_cfg);

    g711_encoder_cfg_t g711_cfg = DEFAULT_G711_ENCODER_CONFIG();
    g711_cfg.task_core = 1;
    audio_element_handle_t sip_encoder = g711_encoder_init(&g711_cfg);

    raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_READER;
    raw_read = raw_stream_init(&raw_cfg);
    audio_element_set_output_timeout(raw_read, portMAX_DELAY);

    audio_pipeline_register(recorder, filter, "filter");
    audio_pipeline_register(recorder, sip_encoder, "sip_enc");
    audio_pipeline_register(recorder, raw_read, "raw");

    const char *link_tag[6] = {"i2s", "filter_r", "algo", "filter", "sip_enc", "raw"};
    audio_pipeline_link(recorder, &link_tag[0], 6);
// #endif

    ESP_LOGI(TAG, " SIP recorder has been created");
    return ESP_OK;
}

static esp_err_t player_pipeline_open()
{
    audio_element_handle_t i2s_stream_writer;
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    player = audio_pipeline_init(&pipeline_cfg);
    AUDIO_NULL_CHECK(TAG, player, return ESP_FAIL);

    raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_WRITER;
    raw_write = raw_stream_init(&raw_cfg);

    g711_decoder_cfg_t g711_cfg = DEFAULT_G711_DECODER_CONFIG();
    audio_element_handle_t sip_decoder = g711_decoder_init(&g711_cfg);

    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = CODEC_SAMPLE_RATE;
    rsp_cfg.src_ch = CODEC_CHANNELS;
    rsp_cfg.dest_rate = I2S_SAMPLE_RATE;
    rsp_cfg.dest_ch = I2S_CHANNELS;
    rsp_cfg.complexity = 5;
    audio_element_handle_t filter = rsp_filter_init(&rsp_cfg);

    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_cfg.uninstall_drv = false;
    i2s_cfg.i2s_config.sample_rate = I2S_SAMPLE_RATE;
// #ifndef CONFIG_ESP_LYRAT_MINI_V1_1_BOARD
//     i2s_cfg.multi_out_num = 1;
// #endif
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    audio_pipeline_register(player, raw_write, "raw");
    audio_pipeline_register(player, sip_decoder, "sip_dec");
    audio_pipeline_register(player, filter, "filter");
    audio_pipeline_register(player, i2s_stream_writer, "i2s");
    const char *link_tag[4] = {"raw", "sip_dec", "filter", "i2s"};
    audio_pipeline_link(player, &link_tag[0], 4);

// #ifndef CONFIG_ESP_LYRAT_MINI_V1_1_BOARD
//     //Please reference the way of ALGORITHM_STREAM_INPUT_TYPE2 in "algorithm_stream.h"
//     ringbuf_handle_t ringbuf_ref = rb_create(50 * 1024, 1);
//     audio_element_set_multi_input_ringbuf(element_algo, ringbuf_ref, 0);
//     audio_element_set_multi_output_ringbuf(i2s_stream_writer, ringbuf_ref, 0);

//     /* When the playback signal far ahead of the recording signal,
//         the playback signal needs to be delayed */
//     algo_stream_set_delay(i2s_stream_writer, ringbuf_ref, DEFAULT_REF_DELAY_MS);

//     /* When the playback signal after the recording signal,
//         the recording signal needs to be delayed */
//     algo_stream_set_delay(element_algo, audio_element_get_input_ringbuf(element_algo), DEFAULT_REC_DELAY_MS);
// #endif

    ESP_LOGI(TAG, "SIP player has been created");
    return ESP_OK;
}

static ip4_addr_t _get_network_ip()
{
    tcpip_adapter_ip_info_t ip;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip);
    return ip.ip;
}

static int _sip_event_handler(sip_event_msg_t *event)
{
    ip4_addr_t ip;
    switch ((int)event->type) {
        case SIP_EVENT_REQUEST_NETWORK_STATUS:
            ESP_LOGD(TAG, "SIP_EVENT_REQUEST_NETWORK_STATUS");
            ip = _get_network_ip();
            if (ip.addr) {
                return true;
            }
            return ESP_OK;
        case SIP_EVENT_REQUEST_NETWORK_IP:
            ESP_LOGD(TAG, "SIP_EVENT_REQUEST_NETWORK_IP");
            ip = _get_network_ip();
            int ip_len = sprintf((char *)event->data, "%s", ip4addr_ntoa(&ip));
            return ip_len;
        case SIP_EVENT_REGISTERED:
            ESP_LOGI(TAG, "SIP_EVENT_REGISTERED");
            // audio_player_int_tone_play(tone_uri[TONE_TYPE_SERVER_CONNECT]);
            break;
        case SIP_EVENT_RINGING:
            ESP_LOGI(TAG, "ringing... RemotePhoneNum %s", (char *)event->data);
            // audio_player_int_tone_play(tone_uri[TONE_TYPE_ALARM]);
            break;
        case SIP_EVENT_INVITING:
            ESP_LOGI(TAG, "SIP_EVENT_INVITING Remote Ring...");
            break;
        case SIP_EVENT_ERROR:
            ESP_LOGI(TAG, "SIP_EVENT_ERROR");
            break;
        case SIP_EVENT_HANGUP:
            ESP_LOGI(TAG, "SIP_EVENT_HANGUP");
            break;
        case SIP_EVENT_AUDIO_SESSION_BEGIN:
            ESP_LOGI(TAG, "SIP_EVENT_AUDIO_SESSION_BEGIN");
            recorder_pipeline_open();
            player_pipeline_open();
            audio_pipeline_run(player);
            audio_pipeline_run(recorder);
            break;
        case SIP_EVENT_AUDIO_SESSION_END:
            ESP_LOGI(TAG, "SIP_EVENT_AUDIO_SESSION_END");
            audio_pipeline_stop(player);
            audio_pipeline_wait_for_stop(player);
            audio_pipeline_deinit(player);
            audio_pipeline_stop(recorder);
            audio_pipeline_wait_for_stop(recorder);
            audio_pipeline_deinit(recorder);
            break;
        case SIP_EVENT_READ_AUDIO_DATA:
// #ifdef DEBUG_AEC_INPUT
//             vTaskDelay(20 / portTICK_PERIOD_MS);
//             return event->data_len;
// #else
            return raw_stream_read(raw_read, (char *)event->data, event->data_len);
// #endif
        case SIP_EVENT_WRITE_AUDIO_DATA:
            return raw_stream_write(raw_write, (char *)event->data, event->data_len);
        case SIP_EVENT_READ_DTMF:
            ESP_LOGI(TAG, "SIP_EVENT_READ_DTMF ID : %d ", ((char *)event->data)[0]);
            break;
    }
    return 0;
}

static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
/*    
    audio_board_handle_t board_handle = (audio_board_handle_t) ctx;
    int player_volume;
*/
    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
        ESP_LOGD(TAG, "[ * ] input key id is %d", (int)evt->data);
        sip_state_t sip_state = esp_sip_get_state(sip);
        if (sip_state < SIP_STATE_REGISTERED) {
            return ESP_OK;
        }
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_REC:
            case INPUT_KEY_USER_ID_MUTE:
                ESP_LOGI(TAG, "[ * ] [Rec] Set MIC Mute or not");
                if (mute) {
                    mute = false;
                    display_service_set_pattern(disp, DISPLAY_PATTERN_TURN_OFF, 0);
                } else {
                    mute = true;
                    display_service_set_pattern(disp, DISPLAY_PATTERN_TURN_ON, 0);
                }
#if defined CONFIG_ESP_LYRAT_MINI_V1_1_BOARD
                audio_hal_set_mute(board_handle->adc_hal, mute);
#endif                
                break;
            case INPUT_KEY_USER_ID_PLAY:
                ESP_LOGI(TAG, "[ * ] [Play] input key event (RecBtn)");
                if (sip_state == SIP_STATE_RINGING) {
                    // audio_player_int_tone_stop();
                    esp_sip_uas_answer(sip, true);
                } else if (sip_state == SIP_STATE_REGISTERED) {
                    esp_sip_uac_invite(sip, "101");
                }
                break;
            case INPUT_KEY_USER_ID_MODE:
            /*case INPUT_KEY_USER_ID_PLAY:*/
                ESP_LOGI(TAG, "[ * ] [Play] input key event (RecBtn)");
                if (sip_state == SIP_STATE_RINGING) {
                    // audio_player_int_tone_stop();
                    esp_sip_uas_answer(sip, true);
                } else if (sip_state == SIP_STATE_REGISTERED) {
                    esp_sip_uac_invite(sip, "101");
                }
                break;
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
/*
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
                esp_sip_destroy(sip);
                wifi_service_setting_start(wifi_serv, 0);
                // audio_player_int_tone_play(tone_uri[TONE_TYPE_UNDER_SMARTCONFIG]);
                break;
        }
    }

    return ESP_OK;
}

static esp_err_t wifi_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    ESP_LOGD(TAG, "event type:%d,source:%p, data:%p,len:%d,ctx:%p",
             evt->type, evt->source, evt->data, evt->len, ctx);
    if (evt->type == WIFI_SERV_EVENT_CONNECTED) 
    {
        //  LED индикация подключения WiFi
        ESP_LOGI(TAG, "Icom connected to WiFi");
        icom_led_WIFI_set();
    
        //  Запуск SIP сервиса
        ESP_LOGI(TAG, "PERIPH_WIFI_CONNECTED [%d]", __LINE__);
        is_smart_config = false;
#if 0
        ESP_LOGI(TAG, "[ 5 ] Create SIP Service");
        sip_config_t sip_cfg = {
            .uri = CONFIG_SIP_URI,
            .event_handler = _sip_event_handler,
            .send_options = true,
    #ifdef CONFIG_SIP_CODEC_G711A
            .acodec_type = SIP_ACODEC_G711A,
    #else
            .acodec_type = SIP_ACODEC_G711U,
    #endif
        };
        sip = esp_sip_init(&sip_cfg);
        esp_sip_start(sip);
#endif
    } 
    else if (evt->type == WIFI_SERV_EVENT_DISCONNECTED) 
    {
        //  LED индикация подключения WiFi
        ESP_LOGI(TAG, "Icom NOT connected to WiFi");
        icom_led_WIFI_reset();

        ESP_LOGI(TAG, "PERIPH_WIFI_DISCONNECTED [%d]", __LINE__);
        if (is_smart_config == false) {
            // audio_player_int_tone_play(tone_uri[TONE_TYPE_PLEASE_SETTING_WIFI]);
        }
    }
    else if (evt->type == WIFI_SERV_EVENT_SETTING_TIMEOUT) 
    {
        //  LED индикация подключения WiFi
        ESP_LOGI(TAG, "Icom connected OR NOT connected to WiFi    >>>>    Check status");

        ESP_LOGW(TAG, "WIFI_SERV_EVENT_SETTING_TIMEOUT [%d]", __LINE__);
        // audio_player_int_tone_play(tone_uri[TONE_TYPE_PLEASE_SETTING_WIFI]);
        is_smart_config = false;
    }

    return ESP_OK;
}

void setup_wifi()
{
    int reg_idx = 0;
    wifi_service_config_t cfg = WIFI_SERVICE_DEFAULT_CONFIG();
    cfg.evt_cb = wifi_service_cb;
    cfg.setting_timeout_s = 300;
    cfg.max_retry_time = 2;
    wifi_serv = wifi_service_create(&cfg);

    smart_config_info_t info = SMART_CONFIG_INFO_DEFAULT();
    esp_wifi_setting_handle_t h = smart_config_create(&info);
    esp_wifi_setting_regitster_notify_handle(h, (void *)wifi_serv);
    wifi_service_register_setting_handle(wifi_serv, h, &reg_idx);

    wifi_config_t sta_cfg = {0};
    //strncpy((char *)&sta_cfg.sta.ssid, CONFIG_WIFI_SSID, sizeof(sta_cfg.sta.ssid));
    //strncpy((char *)&sta_cfg.sta.password, CONFIG_WIFI_PASSWORD, sizeof(sta_cfg.sta.password));
    strncpy((char *)&sta_cfg.sta.ssid, WIFI_SSID, sizeof(sta_cfg.sta.ssid));
    strncpy((char *)&sta_cfg.sta.password, WIFI_PASSWORD, sizeof(sta_cfg.sta.password));
    wifi_service_set_sta_info(wifi_serv, &sta_cfg);
    wifi_service_connect(wifi_serv);

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    //  TODO
    //  Вывести параметры sta_cfg.sta
    ESP_LOGI(TAG, " ");
    ESP_LOGI(TAG, "----------  WiFi  ----------");
    ESP_LOGI(TAG, "WiFi SSID = %s", WIFI_SSID);
    ESP_LOGI(TAG, "WiFi PASS = %s", WIFI_PASSWORD);
    ESP_LOGI(TAG, " ");

    ESP_LOGI(TAG, "WiFi sta.ssid = %s", sta_cfg.sta.ssid);
    ESP_LOGI(TAG, "WiFi sta.pass = %s", sta_cfg.sta.password);
    ESP_LOGI(TAG, " ");

    ESP_LOGI(TAG, "WiFi ap.ssid = %s", sta_cfg.ap.ssid);
    ESP_LOGI(TAG, "WiFi ap.pass = %s", sta_cfg.ap.password);
    ESP_LOGI(TAG, " ");

    //ESP_LOGI(TAG, "WiFi: %s", wifi_serv->service_name);
    
            
                //wifi_service_t *serv = periph_service_get_data(wifi_serv);

                    // if (wifi_serv->reason != WIFI_SERV_STA_SET_INFO) {
                    //     ESP_LOGI(TAG, " ");
                    //     ESP_LOGI(TAG, "WiFi sta.ssid = %s", wifi_cfg.sta.ssid);
                    //     ESP_LOGI(TAG, "WiFi sta.pass = %s", wifi_cfg.sta.password);
                    //     ESP_LOGI(TAG, " ");
                        // if (wifi_ssid_manager_get_latest_config(serv->ssid_manager, &wifi_cfg) != ESP_OK) {
                    //         ESP_LOGW(TAG, "No ssid stored in flash, try to connect to wifi set by wifi_service_set_sta_info()");
                    //         if (serv->info.sta.ssid[0] == 0) {
                    //             ESP_LOGW(TAG, "There is no preset ssid, please set the wifi first");
                    //             continue;
                    //         }
                    //         memcpy(&wifi_cfg, &serv->info, sizeof(wifi_config_t));
                        // }
                    // }
                        ESP_LOGI(TAG, " ");
                        ESP_LOGI(TAG, "WiFi sta.ssid = %s", sta_cfg.sta.ssid);
                        ESP_LOGI(TAG, "WiFi sta.pass = %s", sta_cfg.sta.password);
                        ESP_LOGI(TAG, " ");






    //  Если не подключились, запускаем точку доступа
    // if(err == ESP_OK)
    // {
    //     ESP_LOGI(TAG, "Icom connected to WiFi");
    //     icom_led_WIFI_set();
    // }
    // else
    // {
    //     ESP_LOGI(TAG, "Icom NOT connected to WiFi");
    //     ESP_LOGI(TAG, "Starts Access Point (AP)");
    //     icom_led_WIFI_reset();

    //     // TODO:  
    //     //      Добавить запуск точки доступа
    //     //      

    // }

    //  Подключение LED индикации WiFi

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
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
    ESP_ERROR_CHECK(esp_netif_init());
#else
    tcpip_adapter_init();
#endif


    ESP_LOGI(TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[1.1] Initialize and start peripherals");
    audio_board_key_init(set);                                                      // инициализация кнопок управления (REC BTN & MODE BTN ++ TOUCH_PAD_SELх)
    
// #ifdef DEBUG_AEC_INPUT
//     audio_board_sdcard_init(set, SD_MODE_1_LINE);
// #endif

    ESP_LOGI(TAG, "[1.2] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();           //  передается перечень кнопок управления
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = set;
    input_cfg.based_cfg.task_stack = 4 * 1024;
    periph_service_handle_t input_ser = input_key_service_create(&input_cfg);       //  создание задачи обработки кнопок

#ifdef FUNC_SYS_LEN_EN
    ESP_LOGI(TAG, "[ 1.3 ] Create display service instance");
    //  disp = audio_board_led_init();                                                  //  создание задачи LED индикаторов
    //  disp_led_22 = icom_led_init();                                                  //  создание задачи LED индикаторов
#endif


    ESP_LOGI(TAG, "[ 1.4 ] Configure LEDs");
    icom_led_config();                                                              // Инициализация светодиодов индикации режимов работы

    ESP_LOGI(TAG, "[ 1.4 ] Create LED service instance");
    disp_led_22 = icom_led_init();                                                  //  создание задачи LED индикаторов


    ESP_LOGI(TAG, "[ 1.5 ] Configure IOs");
    icom_ctrl_config();                                                             // Инициализация дискретных входов/выходов


    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();

    //  AUDIO_HAL_CODEC_MODE_ENCODE = 1,  /*!< select adc */
    //  AUDIO_HAL_CODEC_MODE_DECODE,      /*!< select dac */
    //  AUDIO_HAL_CODEC_MODE_BOTH,        /*!< select both adc and dac */
    //  AUDIO_HAL_CODEC_MODE_LINE_IN,     /*!< set adc channel */

    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
    audio_hal_set_volume(board_handle->audio_hal, 80);

    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, input_key_service_cb, (void *)board_handle);

    ESP_LOGI(TAG, "[ 3 ] Initialize tone player");
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
    
    //es8374_read_all();

    // uint8_t reg = 0;
    // es8374_read_reg(0x1A, &reg);
    // ESP_LOGI(TAG, "[ XXXXX ] 1A : %d", reg);
    // es8374_read_reg(0x1B, &reg);
    // ESP_LOGI(TAG, "[ XXXXX ] 1B : %d", reg);
    // es8374_read_reg(0x1C, &reg);
    // ESP_LOGI(TAG, "[ XXXXX ] 1C : %d", reg);
    // es8374_read_reg(0x1C, &reg);
    // ESP_LOGI(TAG, "[ XXXXX ] 1D : %d", reg);
    // es8374_read_reg(0x1E, &reg);
    // ESP_LOGI(TAG, "[ XXXXX ] 1E : %d", reg);
    // es8374_read_reg(0x1F, &reg);
    // ESP_LOGI(TAG, "[ XXXXX ] 1F : %d", reg);

    // es8374_write_reg(0x17, 0x00);  // 23
    // es8374_write_reg(0x18, 0x06);  // 24
    // es8374_write_reg(0x19, 0x22);  // 25

    // es8374_write_reg(0x1A, 0xC0);  // 26
    // es8374_write_reg(0x1B, 0xC0);  // 27

    // es8374_write_reg(0x1C, 0x08);  // 28
    // es8374_write_reg(0x1D, 0x00);  // 29
    // es8374_write_reg(0x1E, 0x1F);  // 30
    // es8374_write_reg(0x1F, 0xF7);  // 31

    //  test();

    //  udp://1004:1234@10.10.34.9:5060
    //  udp://1005:1234@sip.intercom.host:7060

    display_service_set_pattern(disp_led_22, DISPLAY_PATTERN_TURN_ON, 0);


    test();

}

