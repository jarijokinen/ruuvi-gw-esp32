#include <esp_err.h>
#include <esp_log.h>
#include <esp_tls.h>
#include <mqtt_client.h>
#include <sdkconfig.h>

static const char *TAG = "ruuvi-gw-mqtt";

extern const uint8_t server_crt_pem_start[] asm("_binary_server_crt_pem_start");
extern const uint8_t server_crt_pem_end[] asm("_binary_server_crt_pem_end");
extern const uint8_t client_crt_pem_start[] asm("_binary_client_crt_pem_start");
extern const uint8_t client_crt_pem_end[] asm("_binary_client_crt_pem_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_pem_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_pem_end");

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
    int32_t event_id, void *event_data)
{
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;

  switch (event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT connected");
      esp_mqtt_client_publish(client, CONFIG_RUUVI_GW_MQTT_TOPIC, "test data", 
          0, 1, 0);
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "MQTT disconnected");
      break;
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG, "MQTT published");
      break;
    case MQTT_EVENT_DATA:
      ESP_LOGI(TAG, "MQTT data");
      break;
    case MQTT_EVENT_ERROR:
      ESP_LOGI(TAG, "MQTT error");
      break;
    default:
      break;
  }
}

esp_err_t ruuvi_gw_mqtt_init()
{
  esp_mqtt_client_config_t mqtt_cfg = {
    .uri = CONFIG_RUUVI_GW_MQTT_ENDPOINT,
    .cert_pem = (const char *)server_crt_pem_start,
    .client_cert_pem = (const char *)client_crt_pem_start,
    .client_key_pem = (const char *)client_key_pem_start,
    .client_id = CONFIG_RUUVI_GW_MQTT_CLIENT_ID
  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, 
      NULL);
  esp_mqtt_client_start(client);

  return ESP_OK;
}
