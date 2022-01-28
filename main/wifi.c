#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <sdkconfig.h>

static const char *TAG = "ruuvi-gw-wifi";

static EventGroupHandle_t s_ruuvi_gw_wifi_event_group;

static void ruuvi_gw_wifi_event_handler(void *arg, esp_event_base_t event_base,
    int32_t event_id, void *event_data)
{
  switch (event_id) {
    case WIFI_EVENT_STA_START: {
      esp_wifi_connect();
      break;
    }
    case WIFI_EVENT_STA_DISCONNECTED: {
      xEventGroupSetBits(s_ruuvi_gw_wifi_event_group, BIT1);
      ESP_LOGI(TAG, "Connection to the AP failed");
      break;
    }
    case IP_EVENT_STA_GOT_IP: {
      xEventGroupSetBits(s_ruuvi_gw_wifi_event_group, BIT0);
      ESP_LOGI(TAG, "Got IP address from the DHCP server");
      break;
    }
  }
}

esp_err_t ruuvi_gw_wifi_init(void)
{
  esp_err_t err;
  
  s_ruuvi_gw_wifi_event_group = xEventGroupCreate();
  
  err = esp_netif_init();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "TCP/IP stack initialization failed");
    return err;
  }
  
  err = esp_event_loop_create_default();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create default event loop");
    return err;
  }
  
  esp_netif_create_default_wifi_sta();
  
  wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
  err = esp_wifi_init(&wifi_init_cfg);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "WiFi initialization failed");
    return err;
  }

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;

  err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
      &ruuvi_gw_wifi_event_handler, NULL, &instance_any_id);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Event handler instance registration failed");
    return err;
  }
  
  err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
      &ruuvi_gw_wifi_event_handler, NULL, &instance_got_ip);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Event handler instance registration failed");
    return err;
  }

  err = esp_wifi_set_mode(WIFI_MODE_STA);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set WiFi mode");
    return err;
  }
  
  wifi_config_t wifi_cfg = {
    .sta = {
      .ssid = CONFIG_RUUVI_GW_WIFI_SSID,
      .password = CONFIG_RUUVI_GW_WIFI_PSK,
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
      .pmf_cfg = {
        .capable = 1,
        .required = 0
      }
    }
  };
  err = esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set Wifi configuration");
    return err;
  }

  err = esp_wifi_start();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start WiFi");
    return err;
  }

  EventBits_t bits = xEventGroupWaitBits(s_ruuvi_gw_wifi_event_group,
      BIT0 | BIT1,
      pdFALSE,
      pdFALSE,
      portMAX_DELAY);

  if (bits & BIT0) {
    ESP_LOGI(TAG, "Connected to AP");
  }
  else if (bits & BIT1) {
    ESP_LOGI(TAG, "Failed to connect to AP");
    return ESP_FAIL;
  }
  else {
    ESP_LOGE(TAG, "Unexpected event");
    return ESP_FAIL;
  }

  err = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, 
      instance_got_ip);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Event handler instance unregistration failed");
    return err;
  }

  err = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, 
      instance_any_id);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Event handler instance unregistration failed");
    return err;
  }
  
  vEventGroupDelete(s_ruuvi_gw_wifi_event_group);

  return ESP_OK;
}

esp_err_t ruuvi_gw_wifi_destroy(void)
{
  esp_err_t err;

  err = esp_wifi_stop();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to stop WiFi");
    return err;
  }

  err = esp_wifi_deinit();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "WiFi de-initialization failed");
    return err;
  }

  return ESP_OK;
}
