/*
 * device_upgrade.h
 *
 *  Created on: 2018Äê2ÔÂ23ÈÕ
 *      Author: Administrator
 */

#ifndef APP_INCLUDE_DEVICE_UPGRADE_H_
#define APP_INCLUDE_DEVICE_UPGRADE_H_
#include "upgrade.h"
#include "esp_common.h"
#define ESP_DBG os_printf
#define UPGRADE_FRAME  "{\"path\": \"/v1/messages/\", \"method\": \"POST\", \"meta\": {\"Authorization\": \"token %s\"},\
\"get\":{\"action\":\"%s\"},\"body\":{\"pre_rom_version\":\"%s\",\"rom_version\":\"%s\"}}\n"

#define RPC_RESPONSE_FRAME  "{\"status\": 200, \"nonce\": %d, \"deliver_to_device\": true}\n"

#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \r\n\
Accept: */*\r\n\
Authorization: token %s\r\n\
Accept-Encoding: deflate,sdch\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"

#define UPGRADE_SERVER "115.29.202.58"
#define UPGRADE_SERVER_PORT 80
#define DEMO_WIFI_SSID     ""
#define DEMO_WIFI_PASSWORD  ""
#define OTA_TIMEOUT 120000  //120s
void ICACHE_FLASH_ATTR user_esp_sys_upgrade(void *arg, char *pusrdata, unsigned short length);
void fota_begin(void *pvParameters);
void user_platform_rpc_set_rsp(struct espconn *pespconn, int nonce);
#endif /* APP_INCLUDE_DEVICE_UPGRADE_H_ */
