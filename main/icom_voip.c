/* 

    SIP Intercom system

*/

#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
//#include "display_service.h"
#include "wifi_service.h"
#include "periph_service.h"
#include "esp_peripherals.h"

#include <string.h>



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

// #include "icom_led.h"
// #include "icom_wifi.h"
#include "icom_voip.h"



#define WIFI_SSID "WiFi.net"
#define WIFI_PASSWORD "Pass"


static const char *TAG = "Icom_VoIP";


/* Debug original input data for AEC feature*/
// #define DEBUG_AEC_INPUT

#define I2S_SAMPLE_RATE     16000
#define I2S_CHANNELS        2
#define I2S_BITS            16

#define CODEC_SAMPLE_RATE    8000
#define CODEC_CHANNELS       1

static sip_handle_t sip;
static audio_element_handle_t raw_read, raw_write, element_algo;
static audio_pipeline_handle_t recorder, player;


static esp_err_t recorder_pipeline_open()
{
    audio_element_handle_t i2s_stream_reader;
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    recorder = audio_pipeline_init(&pipeline_cfg);
    AUDIO_NULL_CHECK(TAG, recorder, return ESP_FAIL);

    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_READER;
    i2s_cfg.uninstall_drv = false;
    i2s_cfg.task_core = 1;
    i2s_cfg.i2s_config.sample_rate = I2S_SAMPLE_RATE;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);

    rsp_filter_cfg_t rsp_cfg_r = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg_r.src_rate = I2S_SAMPLE_RATE;
    rsp_cfg_r.src_ch = I2S_CHANNELS;
    rsp_cfg_r.dest_rate = I2S_SAMPLE_RATE;
    rsp_cfg_r.complexity = 5;
    rsp_cfg_r.task_core = 1;
    rsp_cfg_r.out_rb_size = 10 * 1024;
    audio_element_handle_t filter_r = rsp_filter_init(&rsp_cfg_r);

    algorithm_stream_cfg_t algo_config = ALGORITHM_STREAM_CFG_DEFAULT();
    algo_config.input_type = ALGORITHM_STREAM_INPUT_TYPE2;
    algo_config.task_core = 1;
    element_algo = algo_stream_init(&algo_config);
    audio_element_set_music_info(element_algo, I2S_SAMPLE_RATE, 1, I2S_BITS);

    audio_pipeline_register(recorder, i2s_stream_reader, "i2s");
    audio_pipeline_register(recorder, filter_r, "filter_r");
    audio_pipeline_register(recorder, element_algo, "algo");

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
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    audio_pipeline_register(player, raw_write, "raw");
    audio_pipeline_register(player, sip_decoder, "sip_dec");
    audio_pipeline_register(player, filter, "filter");
    audio_pipeline_register(player, i2s_stream_writer, "i2s");
    const char *link_tag[4] = {"raw", "sip_dec", "filter", "i2s"};
    audio_pipeline_link(player, &link_tag[0], 4);

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

    ESP_LOGI(TAG, "Called the SIP event handler");

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
            return raw_stream_read(raw_read, (char *)event->data, event->data_len);
        case SIP_EVENT_WRITE_AUDIO_DATA:
            return raw_stream_write(raw_write, (char *)event->data, event->data_len);
        case SIP_EVENT_READ_DTMF:
            ESP_LOGI(TAG, "SIP_EVENT_READ_DTMF ID : %d ", ((char *)event->data)[0]);
            break;
    }
    return 0;
}



void sip_service_create()
{
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
}


sip_state_t sip_state_get()
{
    sip_state_t sip_state = esp_sip_get_state(sip);
    return sip_state;
}
