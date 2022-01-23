#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_err.h>
#include <esp_gap_ble_api.h>
#include <esp_log.h>

static const char *TAG = "ruuvi-gw-bluetooth";

static esp_ble_scan_params_t ruuvi_gw_bluetooth_scan_params = {
};

static void ruuvi_gw_bluetooth_gap_cb(esp_gap_ble_cb_event_t event,
    esp_ble_gap_cb_param_t *param)
{
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

esp_err_t ruuvi_gw_bluetooth_deinit(void)
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
