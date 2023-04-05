# ruuvi-gw-esp32

RuuviTag gateway for ESP32.

Firmware for ESP32 to collect measurement data from RuuviTag devices via BLE
and send the data to AWS IoT Core using MQTT over a WiFi connection.

This firmware currently supports only RuuviTag data format 5.

## Configuration

Use menuconfig to configure the firmware:

    idf.py menuconfig

Put SSL certificates and private key in the ssl directory at the project root:

    ssl/client.crt.pem
    ssl/client.key.pem
    ssl/server.crt.pem

Device address is used for the MQTT topic per default which can be replaced
by a more recognizable device name by adding a line to `ethers` file.


## Usage

    idf.py build
    idf.py -p /dev/ttyUSB0 flash monitor

The log should look like this:

    I (7317) ruuvi-gw-mqtt: Device Address: D4:2C:7F:62:4E:63
    I (7317) ruuvi-gw-mqtt: Device Name:    kitchen
    I (7327) ruuvi-gw-mqtt: Temperature:    16.71 C
    I (7327) ruuvi-gw-mqtt: Humidity:       44.80 %
    I (7337) ruuvi-gw-mqtt: Pressure:       973 hPa
    I (7337) ruuvi-gw-mqtt: Acceleration X: -0.016 G
    I (7347) ruuvi-gw-mqtt: Acceleration Y: -0.028 G
    I (7347) ruuvi-gw-mqtt: Acceleration Z: 1.028 G
    I (7357) ruuvi-gw-mqtt: Battery:        2.967 V
    I (7357) ruuvi-gw-mqtt: TX Power:       4 dBm
    I (7367) ruuvi-gw-mqtt: Moves:          70
    I (7367) ruuvi-gw-mqtt: Sequence:       49685
    I (7367) ruuvi-gw-mqtt:
    I (9427) ruuvi-gw-mqtt: Device Address: FE:5F:48:FB:C6:89
    I (9427) ruuvi-gw-mqtt: Device Name:    bedroom
    I (9427) ruuvi-gw-mqtt: Temperature:    20.42 C
    I (9437) ruuvi-gw-mqtt: Humidity:       45.36 %
    I (9437) ruuvi-gw-mqtt: Pressure:       973 hPa
    I (9447) ruuvi-gw-mqtt: Acceleration X: 0.048 G
    I (9447) ruuvi-gw-mqtt: Acceleration Y: 0.012 G
    I (9457) ruuvi-gw-mqtt: Acceleration Z: 0.968 G
    I (9457) ruuvi-gw-mqtt: Battery:        2.865 V
    I (9467) ruuvi-gw-mqtt: TX Power:       4 dBm
    I (9467) ruuvi-gw-mqtt: Moves:          211
    I (9477) ruuvi-gw-mqtt: Sequence:       56059


## License

MIT License. Copyright (c) 2022 [Jari Jokinen](https://jarijokinen.com).  See
[LICENSE](https://github.com/jarijokinen/ruuvi-gw-esp32/blob/master/LICENSE.txt)
for further details.
