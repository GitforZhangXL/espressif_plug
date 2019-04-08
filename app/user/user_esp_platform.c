
#include "user_esp_platform.h"
#include "user_devicefind.h"
#include "driver/gpio.h"
#include "cJSON.h"
#include "user_config.h"
#include "get_time.h"
#include "device_timer.h"
#define BEACON_FRAME    "{\"path\": \"/v1/ping/\", \"method\": \"POST\",\"meta\": {\"Authorization\": \"token %s\"}}\n"
os_timer_t ping_timer;
os_timer_t change_ap_timer;
struct _esp_tcp user_tcp;
struct espconn espressif_tcp;
static STATUS_P ping_sucess=ping_suc;
ip_addr_t tcp_server_ip;
char server_ip[4] = {115,29,202,58};
uint32 active_nonce;
struct user_saved_param user_config_flash;
uint8_t tcp_broke=200;
uint8_t change_cnt=200;
uint8_t change_id=1;
uint8_t current_id;

/******************************************************************************
 * FunctionName : parse_nonce
 * Description  : parse the device nonce
 * Parameters   : pbuffer -- the recivce data point
 * Returns      : the nonce
*******************************************************************************/
int parse_nonce(char *pbuffer)
{
    char *pstr = NULL;
    char *pparse = NULL;
    char noncestr[11] = {0};
    int nonce = 0;
    pstr = (char *)strstr(pbuffer, "\"nonce\": ");

    if (pstr != NULL) {
        pstr += 9;
        pparse = (char *)strstr(pstr, ",");

        if (pparse != NULL) {
            memcpy(noncestr, pstr, pparse - pstr);
        } else {
            pparse = (char *)strstr(pstr, "}");

            if (pparse != NULL) {
                memcpy(noncestr, pstr, pparse - pstr);
            } else {
                pparse = (char *)strstr(pstr, "]");

                if (pparse != NULL) {
                    memcpy(noncestr, pstr, pparse - pstr);
                } else {
                    return 0;
                }
            }
        }

        nonce = atoi(noncestr);
    }

    return nonce;
}

LOCAL void ICACHE_FLASH_ATTR
espressif_datapoint(void *arg, char *pusrdata, unsigned short length){
	static int data_sta=0;
	struct espconn *pespconn = arg;
	char *pbuf = (char *) os_zalloc(packet_size);
	int nonce = parse_nonce(pusrdata);
	char *user_get=NULL,*user_post=NULL;
	//判断请求类型，get or post
	os_printf("We have a Datapoint\n");
	user_get = (char *) strstr(pusrdata, "GET");
	user_post = (char *) strstr(pusrdata, "POST");
	if(user_get!=NULL){
		os_printf("We have a GET\n");
	}
	else if(user_post!=NULL){
		os_printf("We have a POST\n");
		cJSON *json=cJSON_Parse(pusrdata);//将字符串解析成Json object.
		if (!json) {
			os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
			return;
		}
		else{
		os_printf("POST : %s \r\n", pusrdata);
		cJSON *body     =cJSON_GetObjectItem(json,"body");
		cJSON *datapoint=cJSON_GetObjectItem(body,"datapoint");
		cJSON *num      =cJSON_GetObjectItem(datapoint, "x");
		if(num){
			data_sta=num->valueint;
			os_printf("x:%d\n", data_sta);
#ifdef DEBUG_MOD
			gpio16_output_conf();
			gpio16_output_set(!data_sta);

#else
#if ON_STATUS==0
			GPIO_OUTPUT_SET(NUM_IO,!((uint8_t)data_sta));
#else
			GPIO_OUTPUT_SET(NUM_IO,data_sta);
#endif
#endif
		}
		//json delete
		cJSON_Delete(json);

		}
	}

#ifdef DEBUG_MOD
	gpio16_input_conf();
	sprintf(pbuf, return_data,nonce,!gpio16_input_get());
	gpio16_output_conf();
#else

#if ON_STATUS==0
			sprintf(pbuf, return_data,nonce,!((uint8_t)GPIO_INPUT_GET(NUM_IO)));
#else
			sprintf(pbuf, return_data,nonce,GPIO_INPUT_GET(NUM_IO));
#endif



#endif
	os_printf("pbuf:%s\n",pbuf);
	espconn_send(pespconn, pbuf, strlen(pbuf));
	os_free(pbuf);
	pbuf=NULL;

}
#include<unistd.h>
static uint8_t time_status=10;
LOCAL void espressif_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length) {

	struct espconn *pespconn = arg;
	char *action = NULL,*datapoint=NULL,*activate_status=NULL,*sys_upgrade=NULL,
			*sys_restore,*device_timer=NULL,*ping=NULL;
	ping = (char *) strstr(pusrdata,"\"ping success\"");
	if(ping!=NULL){
		os_printf("Free Heap Size:%d\n",system_get_free_heap_size());
		if(time_status==10){
			set_time(pusrdata);
			time_status=20;
		}
		ping_sucess=ping_suc;
		return;
	}
	activate_status=(char *) strstr(pusrdata,"\"activate_status\": ");
	if(activate_status!=NULL&&parse_nonce(pusrdata) == active_nonce){
		os_printf("activate_status\n");
		if (strncmp(activate_status + 19, "1", 1) == 0){
			user_config_flash.activeflag=DEVICE_ACTIVE_DONE;
			user_devicefind_stop();
			user_local_server_stop();
			os_printf("device activates successful.\n");
		}
		else if(strncmp(activate_status + 19, "0", 0) == 0){
			os_printf("activate fail\n");
			user_config_flash.activeflag=DEVICE_ACTIVE_FAIL;
		}
		system_param_save_with_protect((USER_PARAM_START_SEC/1024/4),&user_config_flash,sizeof(struct user_saved_param));
		return;
	}
	sys_restore = (char *) strstr(pusrdata, "\"action\": \"sys_restore\"");
	sys_upgrade = (char *) strstr(pusrdata, "\"action\": \"sys_upgrade\"");
	datapoint 	= (char *) strstr(pusrdata, "\"path\": \"/v1/datastreams/plug-status/datapoint/\"");
	device_timer= (char *) strstr(pusrdata, "\"path\": \"/v1/device/timers/\"");
	if(sys_upgrade != NULL){
		user_esp_sys_upgrade(pespconn, pusrdata, length);
	}
	else if(sys_restore != NULL){
		int nonce = parse_nonce(pusrdata);
		user_platform_rpc_set_rsp(pespconn, nonce);
		}
	else if(device_timer != NULL){
		os_printf("Devices timer !\n");
		espressif_timer(pespconn,pusrdata,length);

	}
	else if(datapoint != NULL){
		espressif_datapoint(pespconn, pusrdata, length);
	}
	os_printf("Free Heap Size:%d\n",system_get_free_heap_size());
}

/******************************************************************************
 * FunctionName : user_tcp_sent_cb
 * Description  : data sent callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
LOCAL void espressif_tcp_sent_cb(void *arg) {
	//data sent successfully
	os_printf("tcp sent successfully  \r\n");

}
/******************************************************************************
 * FunctionName : user_tcp_discon_cb
 * Description  : disconnect callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
LOCAL void espressif_tcp_discon_cb(void *arg) {
	//tcp disconnect successfully
	os_printf("tcp disconnect successfully\r\n");

}
void once_send_pack(void *parm){

	vTaskDelay(100);//1s,10ms 系统时钟10ms
	send_ping_pack();
	vTaskDelete(NULL);

}
void esp_identify_devices(){
	os_timer_disarm(&ping_timer);
	os_timer_setfn(&ping_timer, (os_timer_func_t *) check_ip, NULL);
	os_timer_arm(&ping_timer, 30000, 1);
	char *buff = (char *) os_zalloc(packet_size);
	sprintf(buff,identify,user_config_flash.devkey);
	//os_printf("identify:%s\n",buff);
	printf("identify\r\n");
	espconn_send(&espressif_tcp,buff,strlen(buff));
	os_free(buff);
	buff=NULL;
	xTaskCreate(once_send_pack, "once_send_pack", 256, NULL, 1, NULL);
}


void esp_active_devices(void *pvParameters){
	while(espressif_tcp.state!=ESPCONN_CONNECT){
		os_printf("wait connect \n");
		vTaskDelay(1000/portTICK_RATE_MS);
	}
	os_timer_disarm(&ping_timer);
	char *buff = (char *) os_zalloc(1024);
	active_nonce = rand() & 0xfffffff;
	sprintf(buff,active_devices,active_nonce,user_config_flash.token,MAC2STR(sta_addr),user_config_flash.devkey);
	os_printf("active:%s\n",buff);
	espconn_send(&espressif_tcp,buff,strlen(buff));
	os_free(buff);
	os_timer_setfn(&ping_timer, (os_timer_func_t *) esp_identify_devices, NULL);
	os_timer_arm(&ping_timer, 2000, 0);
	vTaskDelete(NULL);

}

enum espconn_level esp_keep;
LOCAL void espressif_tcp_cb(void *arg) {
	struct espconn *pespconn = arg;
	os_printf("connect succeed !!! \r\n");
	espconn_regist_recvcb(pespconn, espressif_tcp_recv_cb);
	espconn_regist_sentcb(pespconn, espressif_tcp_sent_cb);
	espconn_regist_disconcb(pespconn, espressif_tcp_discon_cb);
	esp_identify_devices();
}

/******************************************************************************
 * FunctionName : user_tcp_recon_cb
 * Description  : reconnect callback, error occured in TCP connection.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/

LOCAL void espressif_tcp_recon_cb(void *arg, sint8 err) {
	//error occured , tcp connection broke. user can try to reconnect here.
	if(tcp_broke==5){
		user_ap_change();
		return;
	}
	os_printf("reconnect callback, error code %d !!! \r\n", err);
	espconn_connect(&espressif_tcp);
	tcp_broke++;
}
void smartconfig_done(sc_status status, void *pdata) {
	switch (status) {
	case SC_STATUS_WAIT:
		os_printf("SC_STATUS_WAIT\n");
		break;
	case SC_STATUS_FIND_CHANNEL:
		os_printf("SC_STATUS_FIND_CHANNEL\n");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
		sc_type *type = pdata;
		if (*type == SC_TYPE_ESPTOUCH) {
			os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
		} else {
			os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
		}
		break;
	case SC_STATUS_LINK:
		os_printf("SC_STATUS_LINK\n");
		struct station_config *sta_conf = pdata;
		wifi_station_set_config(sta_conf);
		wifi_station_disconnect();
		wifi_station_connect();
		break;
	case SC_STATUS_LINK_OVER:
		os_printf("SC_STATUS_LINK_OVER\n");
		if (pdata != NULL) {
			uint8 phone_ip[4] = { 0 };
			memcpy(phone_ip, (uint8*) pdata, 4);
			os_printf("Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1],
					phone_ip[2], phone_ip[3]);
		}
		smartconfig_stop();
		os_timer_arm(&change_ap_timer, 1000, 0);
		break;
	}

}

void connect_espressif(void *pvParameters){
	espconn_disconnect(&espressif_tcp);
	espconn_delete(&espressif_tcp);
	os_printf("Connect to iot.espressif.cn \r\n");
	espressif_tcp.proto.tcp = &user_tcp;
	espressif_tcp.type = ESPCONN_TCP;
	espressif_tcp.state = ESPCONN_NONE;
	memcpy(espressif_tcp.proto.tcp->remote_ip, server_ip, 4);// remote IP of TCP server
	espressif_tcp.proto.tcp->remote_port = 8000;  // remote port
	espressif_tcp.proto.tcp->local_port = espconn_port(); //local port of ESP8266
	espconn_regist_connectcb(&espressif_tcp, espressif_tcp_cb); // register connect callback
	espconn_regist_reconcb(&espressif_tcp, espressif_tcp_recon_cb); // register reconnect callback as error handler
	espconn_connect(&espressif_tcp);
	vTaskDelete(NULL);
}

/******************************************************************************
 * FunctionName : user_check_ip
 * Description  : check whether get ip addr or not
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void user_check_ip(void) {
	os_timer_disarm(&change_ap_timer);
	os_printf("user_check_ip running!!\n");
	if(wifi_station_get_connect_status() == STATION_GOT_IP){
		return ;
	}
	else {;
		os_printf("Go user_ap_change \n");
   		user_ap_change();
	}
}

/******************************************************************************
 * FunctionName : user_ap_change
 * Description  : add the user interface for changing to next ap ID.
 * Parameters   :
 * Returns      : none
*******************************************************************************/

void user_ap_change(void)
{
	os_timer_disarm(&change_ap_timer);
	if(change_cnt==0&&tcp_broke==0){
		//connect to ap and got ip,
		return;
	}
	os_timer_disarm(&ping_timer);
	static struct station_config config[5];
    switch(change_id){
		case 1:
		{
			//get the AP  number first,if AP number=0;then ,run smartconfig
			change_id=2;
			current_id=wifi_station_get_ap_info(config);
			os_printf("AP Num is: %d\n",current_id);
			if(current_id==0){
				user_config_flash.activeflag=DEVICE_ACTIVE_FAIL;
				system_param_save_with_protect((USER_PARAM_START_SEC/1024/4),&user_config_flash,sizeof(struct user_saved_param));
			}
			os_timer_setfn(&change_ap_timer, (os_timer_func_t *)user_ap_change, NULL);
			os_timer_arm(&change_ap_timer, 1000, 0);
		}
			break;
		case 2:
		{
			smartconfig_stop();
			change_id=2;
			if(current_id>0){
			os_printf("user_esp_ap_is_changing\n");
			os_printf("current ap id =%d\n", current_id);
			wifi_station_ap_change(--current_id);
			/* just need to re-check ip while change AP */
			os_timer_setfn(&change_ap_timer, (os_timer_func_t *)user_ap_change, NULL);
			os_timer_arm(&change_ap_timer, 10000, 0);
			wifi_station_disconnect();
			wifi_station_connect();
			os_printf("user_esp_ap_is_over!!!\n");
			}
			else{
				smartconfig_stop();
				smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS);
				wifi_set_opmode(STATION_MODE);
				smartconfig_start(smartconfig_done);
				os_timer_arm(&change_ap_timer, 60000, 0);
				change_id=1;
			}
		}break;
	default:break;
    }
}

void send_ping_pack(void){

	char *buff = (char *) os_zalloc(512);
	sprintf(buff,ping_server,user_config_flash.devkey);
	espconn_send(&espressif_tcp, buff, strlen(buff));
	os_free(buff);
	buff=NULL;
}
void check_ip(void){

	send_ping_pack();
	if(ping_sucess==ping_time_out){
		user_ap_change();
	}
	else if(ping_sucess==ping_suc){
		ping_sucess=ping_beagin;
	}
	else{
		ping_sucess=ping_time_out;
	}
}

void ReadData(void){
	uint32 temp[11];

	os_printf("\nRead Device Key From Flash\n");
	spi_flash_read(MASTER_DEVICE_KEY,temp,40);
	memcpy(user_config_flash.devkey,temp,40);
	user_config_flash.devkey[40]='\0';
	os_printf("devkey is %s\n",user_config_flash.devkey);
	//512 secort,0x200000(device key),514sec user_config_flash 208000.520sec

}

 void wifi_event_cb(System_Event_t *event)
 {
     if (event == NULL) {
         return;
     }
     switch (event->event_id) {
         case EVENT_STAMODE_GOT_IP:{
            os_printf("sta got ip , creat fota task\n");
 		    os_timer_disarm(&change_ap_timer);
     		change_cnt=0;
     		change_id=1;
     		tcp_broke=0;
     		wifi_get_macaddr(STATION_IF, sta_addr);
     		system_param_load((USER_PARAM_START_SEC/1024/4),0,&user_config_flash,sizeof(struct user_saved_param));
     		if(user_config_flash.activeflag==DEVICE_ACTIVE_DONE){
     			os_printf("flash flag ok\n");
     			xTaskCreate(connect_espressif, "connect_espressif", 256, NULL, 3, NULL);
     		}
     		else{
     			os_printf("flash flag fail\n");
     			user_devicefind_start();
				user_local_server_start();
     			ReadData();
     		}
         }
 			break;
         case EVENT_STAMODE_DISCONNECTED:{
         	os_printf("EVENT_STAMODE_DISCONNECTED\n");
 			change_cnt++;
 			os_printf("Free Heap Size:%d\n",system_get_free_heap_size());
 			if(change_cnt==6){
 				os_printf("GO user_ap_change\n");
 				user_ap_change();
 				break;
 			}
         }
         	break;
         default:
             break;
     }
 }

