/*
 * device_upgrade.c
 *
 *  Created on: 2018年2月23日
 *      Author: Administrator
 */

#include "device_upgrade.h"
#include "user_config.h"
os_timer_t reboot_timer;
extern os_timer_t ping_timer;
extern struct espconn espressif_tcp;
extern struct user_saved_param user_config_flash;
/******************************************************************************
 * FunctionName : user_platform_rpc_set_rsp
 * Description  : response the message to server to show setting info is received
 * Parameters   : pespconn -- the espconn used to connetion with the host
 *                nonce -- mark the message received from server
 * Returns      : none
*******************************************************************************/
void user_platform_rpc_set_rsp(struct espconn *pespconn, int nonce)
{
    char *pbuf = (char *)os_zalloc(2048);

    if (pespconn == NULL) {
        return;
    }

    sprintf(pbuf, RPC_RESPONSE_FRAME, nonce);
    ESP_DBG("%s\n", pbuf);
    espconn_send(pespconn, pbuf, strlen(pbuf));
    os_free(pbuf);
}


/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_cb
 * Description  : Processing the downloaded data from the server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void user_esp_platform_upgrade_rsp(void *arg)
{
    struct upgrade_server_info *server = arg;
    struct espconn *pespconn = server->pespconn;
    uint8 *pbuf = NULL;
    char *action = NULL;
    pbuf = (char *)os_zalloc(2048);

    if (server->upgrade_flag == true) {
        ESP_DBG("user_esp_platform_upgarde_successfully\n");
        action = "device_upgrade_success";
        sprintf(pbuf, UPGRADE_FRAME, user_config_flash.devkey, action, server->pre_version, server->upgrade_version);
        ESP_DBG("%s\n",pbuf);
        ESP_DBG("upgrade err_code %d!\n",espconn_send(&espressif_tcp, pbuf, strlen(pbuf)));

        if (pbuf != NULL) {
            os_free(pbuf);
            pbuf = NULL;
        }
        os_timer_disarm(&reboot_timer);
        os_timer_setfn(&reboot_timer, (os_timer_func_t *)system_upgrade_reboot, NULL);
        os_timer_arm(&reboot_timer, 1000, 0);

    } else {
        ESP_DBG("user_esp_platform_upgrade_failed\n");
        action = "device_upgrade_failed";
        sprintf(pbuf, UPGRADE_FRAME, user_config_flash.devkey, action,server->pre_version, server->upgrade_version);
        ESP_DBG("%s\n",pbuf);

        espconn_send(&espressif_tcp, pbuf, strlen(pbuf));
        system_restart();
        if (pbuf != NULL) {
            os_free(pbuf);
            pbuf = NULL;
        }
    }

    os_free(server->url);
    server->url = NULL;
    os_free(server);
    server = NULL;
}




/******************************************************************************
 * FunctionName : fota_begin
 * Description  : ota_task function
 * Parameters   : task param
 * Returns      : none
*******************************************************************************/
void fota_begin(void *pvParameters)
{
	struct upgrade_server_info *server = pvParameters;
	uint8 user_bin[9] = {0};
	printf("Hello, welcome to client!\r\n");
	server->check_cb = user_esp_platform_upgrade_rsp;
	server->check_times = OTA_TIMEOUT;
	bzero(&server->sockaddrin,sizeof(struct sockaddr_in));
	server->sockaddrin.sin_family=AF_INET;
	server->sockaddrin.sin_addr.s_addr=inet_addr(UPGRADE_SERVER);
	server->sockaddrin.sin_port=htons(UPGRADE_SERVER_PORT);
	if (server->url == NULL) {
		server->url = (uint8 *)zalloc(512);
	}
	if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1) {
		memcpy(user_bin, "user2.bin", 10);
	} else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2) {
		memcpy(user_bin, "user1.bin", 10);
	}
#if USER_UPGRADE_TEST
	sprintf(server->url, "GET /%s HTTP/1.0\r\n\r\n",user_bin);
#else
	sprintf(server->url, "GET /v1/device/rom/?action=download_rom&version=%s&filename=%s HTTP/1.0\r\nHost: "UPGRADE_SERVER":%d\r\n"pheadbuffer"",
			   server->upgrade_version, user_bin,ntohs(server->sockaddrin.sin_port), user_config_flash.devkey);//  IPSTR  IP2STR(server->sockaddrin.sin_addr.s_addr)

#endif


	ESP_DBG("%s\n",server->url);
	if (system_upgrade_start(server) == true)
	{
		ESP_DBG("upgrade is already started\n");
	}


}


/******************************************************************************
 * FunctionName : user_esp_sys_upgrade
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
 void user_esp_sys_upgrade(void *arg, char *pusrdata, unsigned short length)
{
    char *pstr = NULL;
    LOCAL char pbuffer[1024 * 2] = {0};
    ESP_DBG("user_esp_sys_upgrade %s\n", pusrdata);

    if (length == 1460) {
        memcpy(pbuffer, pusrdata, length);
    } else {
        struct espconn *pespconn = (struct espconn *)arg;
        memcpy(pbuffer + strlen(pbuffer), pusrdata, length);
        if ((pstr = (char *)strstr(pbuffer, "\"action\": \"sys_upgrade\"")) != NULL) {
            if ((pstr = (char *)strstr(pbuffer, "\"version\":")) != NULL) {
                struct upgrade_server_info *server = NULL;
                int nonce = parse_nonce(pbuffer);
                user_platform_rpc_set_rsp(pespconn, nonce);
                server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
                memcpy(server->upgrade_version, pstr + 12, 6);
                server->upgrade_version[15] = '\0';
                sprintf(server->pre_version,"%s%d.%d.%d","v",1,1,1);
                os_timer_disarm(&ping_timer);//取消ping
                fota_begin(server);
            }
        } else if ((pstr = (char *)strstr(pbuffer, "\"action\": \"sys_reboot\"")) != NULL) {
            os_timer_disarm(&reboot_timer);
            os_timer_setfn(&reboot_timer, (os_timer_func_t *)system_upgrade_reboot, NULL);
            os_timer_arm(&reboot_timer, 1000, 0);
        }

    }
    memset(pbuffer, 0, sizeof(pbuffer));
}

