///*
// * user_template.c
// *
// *  Created on: 2017年3月23日
// *      Author: Administrator
// */
//
//#include "esp_common.h"
//#include "espconn.h"
//#include "lwip/mem.h"
//#include "user_esp_platform.h"
//#include "driver/18b20.h"
//os_timer_t updata_timer;
//#define template_active "{\"path\": \"/v1/device/activate/\", \"method\": \"POST\", \"meta\": {\"Authorization\":\"token %s\"}, \"body\": {\"encrypt_method\": \"PLAIN\",\"bssid\": \"%s\",\"token\": \"a64480513d0937104a2897d73634c6aa37e7ff38\"}}\n\n"
//#define template_identify "{\"nonce\": 560192812, \"path\": \"/v1/device/identify\", \"method\": \"GET\", \"meta\":{\"Authorization\": \"token %s\"}}\n\n"
//
//#define updata_temp "{\"nonce\": 1, \"path\": \"/v1/datastreams/tem_hum/datapoint/\", \"method\": \"POST\",\
//	\"body\": {\"datapoint\": {\"x\": %s}}, \"meta\": {\"Authorization\": \"token %s\"}}"
//void updata_template(void);
//void template_identify_devices(void);
//void template_active_devices(void){
//	os_timer_disarm(&updata_timer);
//	char *buff = (char *) os_zalloc(512);
//	sprintf(buff,template_active,master_device_token,sta_mac);
//	os_printf("active:%s\n",buff);
//	espconn_send(&espressif_tcp,buff,strlen(buff));//仅需第一次发送，以后无需发送
//	os_free(buff);
//	os_timer_setfn(&updata_timer, (os_timer_func_t *) template_identify_devices, NULL);
//	os_timer_arm(&updata_timer, 2000, 0);
//}
//
//uint8 user_device_template_token[41];
//void template_identify_devices(){
//	os_timer_disarm(&updata_timer);
//	os_timer_setfn(&updata_timer, (os_timer_func_t *) updata_template, NULL);
//	os_timer_arm(&updata_timer, 30000, 1);
//	char *buff = (char *) os_zalloc(512);
//	sprintf(buff,template_identify,user_device_template_token);
//	os_printf("identify:%s\n",buff);
//	espconn_send(&espressif_tcp,buff,strlen(buff));
//	os_free(buff);
//	buff=NULL;
//}
//
//
//void ICACHE_FLASH_ATTR
//updata_template(void){
//
//	char *buff = (char *) os_zalloc(512);
//	sprintf(buff,updata_temp,master_device_token);
//	espconn_send(&espressif_tcp, buff, strlen(buff));
//	os_free(buff);
//	buff=NULL;
//
//}
//
//
//
//// void ICACHE_FLASH_ATTR
////user_template_init(void)
////{
////	espconn_disconnect(&template_tcp);
////	espconn_delete(&template_tcp);
////	os_printf("GOT IP !!! \r\n");
////	template_tcp.proto.tcp = &user_template_tcp;
////	template_tcp.type = ESPCONN_TCP;
////	template_tcp.state = ESPCONN_NONE;
////	memcpy(template_tcp.proto.tcp->remote_ip, server_ip, 4);// remote IP of TCP server
////	template_tcp.proto.tcp->remote_port = 8000;  // remote port
////	template_tcp.proto.tcp->local_port = espconn_port(); //local port of ESP8266
////	espconn_regist_connectcb(&template_tcp, template_tcp_cb); // register connect callback
////	espconn_regist_reconcb(&template_tcp, template_tcp_recon_cb); // register reconnect callback as error handler
////	espconn_connect(&template_tcp);
////
////}
//
//
//
