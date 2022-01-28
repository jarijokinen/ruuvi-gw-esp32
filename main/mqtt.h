#ifndef RUUVI_GW_MQTT_H
#define RUUVI_GW_MQTT_H

#include <esp_err.h>

#include "measurement_data.h"

esp_err_t ruuvi_gw_mqtt_init(void);
void ruuvi_gw_mqtt_add_measurement(measurement_data_t measurement);

#endif
