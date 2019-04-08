# ESP8266_RTOS_SDK #

----------

ESP8266 SDK based on FreeRTOS.
   
## Note ##

APIs of "ESP8266_RTOS_SDK" are same as "ESP8266_NONOS_SDK"

More details in "Wiki" !

## Requrements ##

You can use both xcc and gcc to compile your project, gcc is recommended.
For gcc, please refer to [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk).

  
## Compile ##

Clone ESP8266_RTOS_SDK, e.g., to ~/ESP8266_RTOS_SDK.

    $git clone https://github.com/espressif/ESP8266_RTOS_SDK.git

Modify gen_misc.sh or gen_misc.bat:
For Linux:

    $export SDK_PATH=~/ESP8266_RTOS_SDK
    $export BIN_PATH=~/ESP8266_BIN

For Windows:

    set SDK_PATH=/c/ESP8266_RTOS_SDK
    set BIN_PATH=/c/ESP8266_BIN

ESP8266_RTOS_SDK/examples/project_template is a project template, you can copy this to anywhere, e.g., to ~/workspace/project_template.

Generate bin: 
For Linux:

    ./gen_misc.sh

For Windows:

    gen_misc.bat
   
Just follow the tips and steps.

## Download ##

根据下载工具提示或官方文档下载bin文件到对应位置，device_key.bin，下载位置由user_config.h中的定义决定




## NOTES ##



### Step 1

change upgrade.h,in struct upgrade_server_info add
    struct espconn *pespconn;


### Step 2

in app/driver/*.c changed include *.h to driver/*.h ,like this:
    #incude "gpio.h" changed to #include "driver/gpio.h"

### Step3 ###

    parent_dir:=$(abspath $(shell pwd)/$(lastword $(MAKEFILE_LIST)))
    parent_dir:=$(shell dirname $(parent_dir))
    parent_dir:=$(shell dirname $(parent_dir))
    
    SDK_PATH=$(parent_dir)
    BIN_PATH=$(SDK_PATH)/bin


