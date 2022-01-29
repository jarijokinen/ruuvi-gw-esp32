#include <esp_err.h>
#include <esp_log.h>
#include <esp_tls.h>
#include <freertos/ringbuf.h>
#include <mqtt_client.h>
#include <sdkconfig.h>

#include "mqtt.h"

#define RUUVI_GW_MQTT_TOPIC(bda) CONFIG_RUUVI_GW_MQTT_TOPIC "/" bda

static const char *TAG = "ruuvi-gw-mqtt";

enum {
  RUUVI_GW_MQTT_STATE_NULL,
  RUUVI_GW_MQTT_STATE_READY
};

static esp_mqtt_client_handle_t ruuvi_gw_mqtt_client;
static RingbufHandle_t measurements;
static int ruuvi_gw_mqtt_state = RUUVI_GW_MQTT_STATE_NULL;

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
      
      for (;;) {
        size_t measurement_size;
        measurement_data_t *measurement = xRingbufferReceive(measurements, 
            &measurement_size, 10 * 1000 / portTICK_PERIOD_MS);

        if (measurement == NULL) {
          ruuvi_gw_mqtt_state = RUUVI_GW_MQTT_STATE_READY;
          break;
        }

        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "Device Address: %s", measurement->bda);
        ESP_LOGI(TAG, "Temperature:    %.2f C", measurement->temperature);
        ESP_LOGI(TAG, "Humidity:       %.2f %%", measurement->humidity);
    
        char topic[100];
        char data[10];

        sprintf(topic, "%s/%s/temperature", CONFIG_RUUVI_GW_MQTT_TOPIC, 
            measurement->bda);
        sprintf(data, "%.2f", measurement->temperature);
        esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
        
        sprintf(topic, "%s/%s/humidity", CONFIG_RUUVI_GW_MQTT_TOPIC, 
            measurement->bda);
        sprintf(data, "%.2f", measurement->humidity);
        esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
      }
      
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

void ruuvi_gw_mqtt_add_measurement(measurement_data_t measurement)
{
  xRingbufferSend(measurements, &measurement, sizeof(measurement), 
      pdMS_TO_TICKS(1000));
}

esp_err_t ruuvi_gw_mqtt_destroy()
{
  esp_err_t err;
  err = esp_mqtt_client_destroy(ruuvi_gw_mqtt_client);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to destroy MQTT client");
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t ruuvi_gw_mqtt_init()
{
  esp_err_t err;
  measurements = xRingbufferCreate(1028, RINGBUF_TYPE_NOSPLIT);
  if (measurements == NULL) {
    ESP_LOGE(TAG, "Ring buffer creation failed");
    return ESP_FAIL;
  }

  esp_mqtt_client_config_t mqtt_cfg = {
    .uri = CONFIG_RUUVI_GW_MQTT_ENDPOINT,
    .cert_pem = (const char *)server_crt_pem_start,
    .client_cert_pem = (const char *)client_crt_pem_start,
    .client_key_pem = (const char *)client_key_pem_start,
    .client_id = CONFIG_RUUVI_GW_MQTT_CLIENT_ID
  };

  ruuvi_gw_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(ruuvi_gw_mqtt_client, ESP_EVENT_ANY_ID, 
      mqtt_event_handler, NULL);
  esp_mqtt_client_start(ruuvi_gw_mqtt_client);
  
  for (int i = 0;; i++) {
    vTaskDelay(1 * 1000 / portTICK_PERIOD_MS);
    if (ruuvi_gw_mqtt_state == RUUVI_GW_MQTT_STATE_READY) {
      ESP_LOGI(TAG, "MQTT finished");
      err = ruuvi_gw_mqtt_destroy();
      if (err != ESP_OK) {
        return err;
      }
      break;
    }
    else if (i == CONFIG_RUUVI_GW_MQTT_TIMEOUT) {
      ESP_LOGI(TAG, "MQTT timeout");
      err = ruuvi_gw_mqtt_destroy();
      if (err != ESP_OK) {
        return err;
      }
      break;
    }
  }
  
  return ESP_OK;
}
