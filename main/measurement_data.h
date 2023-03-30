#ifndef RUUVI_GW_MEASUREMENT_DATA_H
#define RUUVI_GW_MEASUREMENT_DATA_H

typedef struct measurement_data_t {
  char name[25];
  char bda[18];
  float temperature;
  float humidity;
  unsigned int pressure;
  float acceleration_x;
  float acceleration_y;
  float acceleration_z;
  float battery;
  int txpower;
  unsigned int moves;
  unsigned int sequence;
} measurement_data_t;

#endif
