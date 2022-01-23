#include <nvs_flash.h>

#include "bluetooth.h"

static const char *TAG = "ruuvi-gw";

void app_main(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(ruuvi_gw_bluetooth_init());
}
