/* 

    SIP Intercom system

*/

#include <string.h>
#include "esp_log.h"
#include "display_service.h"
#include "wifi_service.h"
#include "periph_service.h"
#include "esp_peripherals.h"

#include "icom_led.h"
#include "icom_wifi.h"
#include "icom_voip.h"

#include "smart_config.h"


#define WIFI_SSID "WiFi.net"
#define WIFI_PASSWORD "Pass"


static const char *TAG = "Icom_WiFi";


static periph_service_handle_t wifi_serv;

extern display_service_handle_t disp;



static esp_err_t wifi_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    ESP_LOGD(TAG, "event type:%d,source:%p, data:%p,len:%d,ctx:%p",
             evt->type, evt->source, evt->data, evt->len, ctx);
    if (evt->type == WIFI_SERV_EVENT_CONNECTED) 
    {
        //  LED индикация подключения WiFi
        ESP_LOGI(TAG, "Icom connected to WiFi");
        // display_service_set_pattern(disp_led_WiFi, DISPLAY_PATTERN_TURN_ON, 0);
        icom_led_WiFi_On();

    
        //  Запуск SIP сервиса
        ESP_LOGI(TAG, "PERIPH_WIFI_CONNECTED [%d]", __LINE__);
        is_smart_config = false;
#if 0
        ESP_LOGI(TAG, "[ 5 ] Create SIP Service");
        sip_service_create();
/*        
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
*/
#endif
    } 
    else if (evt->type == WIFI_SERV_EVENT_DISCONNECTED) 
    {
        //  LED индикация подключения WiFi
        ESP_LOGI(TAG, "Icom NOT connected to WiFi");
        // display_service_set_pattern(disp_led_WiFi, DISPLAY_PATTERN_TURN_OFF, 0);
        icom_led_WiFi_Off();

        ESP_LOGI(TAG, "PERIPH_WIFI_DISCONNECTED [%d]", __LINE__);
        if (is_smart_config == false) {
            // audio_player_int_tone_play(tone_uri[TONE_TYPE_PLEASE_SETTING_WIFI]);
        }
    }
    else if (evt->type == WIFI_SERV_EVENT_SETTING_TIMEOUT) 
    {
        //  LED индикация подключения WiFi
        ESP_LOGI(TAG, "Icom connected OR NOT connected to WiFi    >>>>    Check problem !!!");
        // display_service_set_pattern(disp_led_WiFi, DISPLAY_PATTERN_TURN_OFF, 0);
        icom_led_WiFi_Off();

        ESP_LOGW(TAG, "WIFI_SERV_EVENT_SETTING_TIMEOUT [%d]", __LINE__);
        // audio_player_int_tone_play(tone_uri[TONE_TYPE_PLEASE_SETTING_WIFI]);
        is_smart_config = false;
    }

    return ESP_OK;
}


/**
 * @brief Initialize WiFi
 */
void setup_wifi()
{
    ESP_LOGI(TAG, "Create Wi-Fi service instance");

    int reg_idx = 0;
    wifi_service_config_t cfg = WIFI_SERVICE_DEFAULT_CONFIG();
    cfg.evt_cb = wifi_service_cb;
    cfg.setting_timeout_s = 300;
    cfg.max_retry_time = 2;
    wifi_serv = wifi_service_create(&cfg);

    ESP_LOGI(TAG, "%p = %p", cfg.evt_cb, wifi_service_cb);

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

        // if (wifi_ssid_manager_get_latest_config(serv->ssid_manager, &wifi_cfg) != ESP_OK) {






        // TODO:  
        //      Добавить запуск точки доступа
        //      

    //  Подключение LED индикации WiFi

}