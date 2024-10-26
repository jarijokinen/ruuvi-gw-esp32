idf_component_register(
  SRCS "ruuvi-gw.c" "bluetooth.c" "mqtt.c" "wifi.c" "ethernet.c" "ethers.c"
  REQUIRES esp-tls nvs_flash esp_wifi
  PRIV_REQUIRES bt driver esp_eth mqtt
  INCLUDE_DIRS "."
)
