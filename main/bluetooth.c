#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_err.h>
#include <esp_gap_ble_api.h>
#include <esp_log.h>
#include <freertos/ringbuf.h>

#include "measurement_data.h"
#include "mqtt.h"

static const char *TAG = "ruuvi-gw-bluetooth";

static esp_ble_scan_params_t ruuvi_gw_bluetooth_scan_params = {
  .scan_type = BLE_SCAN_TYPE_ACTIVE,
  .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
  .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
  .scan_interval = 0x50,
  .scan_window = 0x30
};

static void ruuvi_gw_bluetooth_gap_cb(esp_gap_ble_cb_event_t event,
    esp_ble_gap_cb_param_t *param)
{
  esp_err_t err;

  switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
      ESP_LOGI(TAG, "BLE scan params set");
      err = esp_ble_gap_start_scanning(10);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "BLE scan start failed");
      }
      break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: {
      ESP_LOGI(TAG, "BLE scan started");
      break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
      esp_ble_gap_cb_param_t *res = (esp_ble_gap_cb_param_t *)param;

      switch (res->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT: {
          if (res->scan_rst.adv_data_len == 0 ||
              res->scan_rst.ble_adv[5] != 0x99 ||
              res->scan_rst.ble_adv[6] != 0x04) {
            /* Data length zero or manufacturer does not match */
            break;
          }

          /* Data format */
          switch (res->scan_rst.ble_adv[7]) {
            case 0x05: {
              measurement_data_t measurement = {};

              sprintf(measurement.bda, "%02X:%02X:%02X:%02X:%02X:%02X",
                  res->scan_rst.bda[0],
                  res->scan_rst.bda[1],
                  res->scan_rst.bda[2],
                  res->scan_rst.bda[3],
                  res->scan_rst.bda[4],
                  res->scan_rst.bda[5]);
               measurement.temperature = ((res->scan_rst.ble_adv[8] << 8) |
                res->scan_rst.ble_adv[9]) * 0.005;
               measurement.humidity = ((res->scan_rst.ble_adv[10] << 8) |
                res->scan_rst.ble_adv[11]) * 0.0025;

              ruuvi_gw_mqtt_add_measurement(measurement);

              break;
            }
          }

          break;
        }
        case ESP_GAP_SEARCH_INQ_CMPL_EVT: {
          ESP_LOGI(TAG, "BLE scan stopped");
          break;
        }
        default: {
          break;
        }
      }
    }
    default: {
      break;
    }
  }
}

esp_err_t ruuvi_gw_bluetooth_init(void)
{
  esp_err_t err;

  err = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Bluetooth controller memory release failed");
    return err;
  }

  esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

  err = esp_bt_controller_init(&cfg);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Bluetooth controller initialization failed");
    return err;
  }

  err = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Bluetooth controller enable failed");
    return err;
  }
  
  err = esp_bluedroid_init();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Bluetooth resource initialization failed");
    return err;
  }
  
  err = esp_bluedroid_enable();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Bluetooth enable failed");
    return err;
  }

  err = esp_ble_gap_register_callback(ruuvi_gw_bluetooth_gap_cb);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Gap event callback registration failed");
    return err;
  }
  
  err = esp_ble_gap_set_scan_params(&ruuvi_gw_bluetooth_scan_params);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Gap scan params set failed");
    return err;
  }

  return ESP_OK;
}

esp_err_t ruuvi_gw_bluetooth_destroy(void)
{
  esp_err_t err;

  err = esp_bluedroid_disable();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Bluetooth disable failed");
    return err;
  }

  err = esp_bluedroid_deinit();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Bluetooth resource de-initialization failed");
    return err;
  }

  err = esp_bt_controller_disable();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Bluetooth controller disable failed");
    return err;
  }

  err = esp_bt_controller_deinit();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Bluetooth controller de-initialization failed");
    return err;
  }

  return ESP_OK;
}
