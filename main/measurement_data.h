#ifndef RUUVI_GW_MEASUREMENT_DATA_H
#define RUUVI_GW_MEASUREMENT_DATA_H

typedef struct measurement_data_t {
  char bda[18];
  float temperature;
  float humidity;
} measurement_data_t;

#endif
