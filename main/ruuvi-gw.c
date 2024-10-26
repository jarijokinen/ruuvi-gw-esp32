#include <esp_log.h>
#include <esp_sleep.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include "bluetooth.h"
#include "mqtt.h"
#include "sdkconfig.h"
#include "wifi.h"
#include "ethernet.h"

static const char *TAG = "ruuvi-gw";

void app_main(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());

#if CONFIG_RUUVI_GW_USE_WIFI
  ESP_ERROR_CHECK(ruuvi_gw_wifi_init());
  ESP_LOGI(TAG, "WiFi initialized");
#endif

#if CONFIG_RUUVI_GW_USE_ETHERNET
  ESP_ERROR_CHECK(ruuvi_gw_ethernet_init());
  ESP_LOGI(TAG, "Ethernet initialized");
#endif

  ESP_ERROR_CHECK(ruuvi_gw_bluetooth_init());
  ESP_LOGI(TAG, "Bluetooth initialized");

  ESP_ERROR_CHECK(ruuvi_gw_mqtt_init());

  ESP_ERROR_CHECK(ruuvi_gw_bluetooth_destroy());

#if CONFIG_RUUVI_GW_USE_WIFI
  ESP_ERROR_CHECK(ruuvi_gw_wifi_destroy());
#endif

  ESP_LOGI(TAG, "Entering deep-sleep mode...");
  vTaskDelay(3 * 1000 / portTICK_PERIOD_MS);
  esp_deep_sleep(1000000 * 60 * CONFIG_RUUVI_GW_POLLING_INTERVAL);
}
