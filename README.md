# ruuvi-gw-esp32

RuuviTag gateway for ESP32.

Firmware for ESP32 to collect measurement data from RuuviTag devices via BLE
and send the data to AWS IoT Core using MQTT over a WiFi connection.

This firmware currently supports only RuuviTag data format 5.

## Usage

    idf.py build
    idf.py -p /dev/ttyUSB0 flash monitor

The log should look like this:

    I (1335) ruuvi-gw: ** RuuviTag: FE:5A:35:C2:05:DD
    I (1345) ruuvi-gw: Temperature: 24.93 C
    I (1345) ruuvi-gw: Humidity:    15.00 %
    I (3985) ruuvi-gw: 
    I (3985) ruuvi-gw: ** RuuviTag: E8:2B:AE:08:AA:F9
    I (3985) ruuvi-gw: Temperature: 22.34 C
    I (3995) ruuvi-gw: Humidity:    14.00 %

## License

MIT License. Copyright (c) 2022 [Jari Jokinen](https://jarijokinen.com).  See
[LICENSE](https://github.com/jarijokinen/ruuvi-gw-esp32/blob/master/LICENSE.txt)
for further details.
