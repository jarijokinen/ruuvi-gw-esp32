cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ruuvi-gw)

target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "ssl/client.crt.pem" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "ssl/client.key.pem" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "ssl/server.crt.pem" TEXT)
