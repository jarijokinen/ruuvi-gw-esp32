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

    I (6102) ruuvi-gw-bluetooth: Device Address: C0:09:D8:69:A8:01
    I (6102) ruuvi-gw-bluetooth: Device Name:    kitchen
    I (6112) ruuvi-gw-bluetooth: Temperature:    22.06 C
    I (6112) ruuvi-gw-bluetooth: Humidity:       26.26 %
    I (9412) ruuvi-gw-bluetooth: 
    I (9412) ruuvi-gw-bluetooth: Device Address: EE:57:0F:54:B7:94
    I (9412) ruuvi-gw-bluetooth: Device Name:    bedroom
    I (9412) ruuvi-gw-bluetooth: Temperature:    22.26 C
    I (9422) ruuvi-gw-bluetooth: Humidity:       24.96 %

## License

MIT License. Copyright (c) 2022 [Jari Jokinen](https://jarijokinen.com).  See
[LICENSE](https://github.com/jarijokinen/ruuvi-gw-esp32/blob/master/LICENSE.txt)
for further details.
