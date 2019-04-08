# Smart Plug On ESP8266 RTOS SDK #

----------

FreeRTOS ESP8266 SDK (https://github.com/espressif/esp_iot_rtos_sdk) and adds on to it some commonly used functionalities, in an example application of a smart plug. This application uses the ESP-TOUCH protocol to realise smart configuration of the device. The communication protocols used are JSON and HTTP REST. An Android mobile APK (https://github.com/EspressifApp/IOT-Espressif-Android) is also included as a basic template for the users.
   
## Code Structure ##

## usr directory ##

user_main.c: The entry point for the main program. 


user_devicefind.c: Creates a UDP service, which recieves special finder message on port 1025 and allows the user to discover devices on the network. 

user_esp_platform.c: provides the Espressif Smart Configuration API (ESP-TOUCH) example; communicates with the Espressif Cloud servers (customize this to connect to your own servers); maintains the network status and data transmission to server. 



## upgrade directory ##


## include directory ##

The include directory includes the relevant headers needed for the project. Of interest, is "user_config.h", which can be used to configure or select the examples. By setting the MACROs, we can enable the relevant functionality, e.g. PLUG_DEVICE and LIGHT_DEVICE. 


user_esp_platform.h: #define ESP_PARAM_START_SEC 0x7D

user_light.h: #define PRIV_PARAM_START_SEC 0x7C

user_plug.h: #define PRIV_PARAM_START_SEC 0x7C

## Driver Directory ##

This contains the GPIO interface. 


## Usage ##

## Configuration ##


## Compiling the Code ##

First export the two parameters specifying the paths of  esp8266 RTOS SDK and compiler generated firmware.

parent_dir:=$(abspath $(shell pwd)/$(lastword $(MAKEFILE_LIST)))
parent_dir:=$(shell dirname $(parent_dir))
parent_dir:=$(shell dirname $(parent_dir))

SDK_PATH=$(parent_dir)
BIN_PATH=$(SDK_PATH)/bin



boot_v1.7.bin, downloads to flash 0x00000

user1.4096.new.6.bin, downloads to flash 0x001000

esp_init_data_default.bin, downloads to 0x3fc000

blank.bin, downloads to flash 0x3fe000