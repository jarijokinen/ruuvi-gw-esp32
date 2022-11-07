cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ruuvi-gw)

if(CONFIG_RUUVI_GW_MQTT_CLIENTCERT)
  target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "ssl/client.crt.pem" TEXT)
  target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "ssl/client.key.pem" TEXT)
  target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "ssl/server.crt.pem" TEXT)
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/ethers")
  target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "ethers" TEXT)
endif()
