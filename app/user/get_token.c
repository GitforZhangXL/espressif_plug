
#include "esp_common.h"
#include "user_esp_platform.h"
#include "espconn.h"
#include "user_config.h"
#include "cJSON.h"
#include "get_token.h"
#include "driver/gpio.h"
struct espconn local_tcp;
extern struct user_saved_param user_config_flash;

/******************************************************************************
 * FunctionName : data_send
 * Description  : processing the data as http format and send to the client or server
 * Parameters   : arg -- argument to set for client or server
 *                responseOK -- true or false
 *                psend -- The send data
 * Returns      :
*******************************************************************************/
void data_send(void *arg,bool responseOK, char *psend)
{
	struct espconn *pespconn = arg;
    uint16 length = 0;
    char *pbuf = NULL;
    char httphead[256];
    memset(httphead, 0, 256);

    if (responseOK) {
        sprintf(httphead,
                   "HTTP/1.0 200 OK\r\nContent-Length: %d\r\nServer: lwIP/1.4.0\r\n",
                   psend ? strlen(psend) : 0);

        if (psend) {
            sprintf(httphead + strlen(httphead),
                       "Content-type: application/json\r\nExpires: Fri, 10 Apr 2008 14:00:00 GMT\r\nPragma: no-cache\r\n\r\n");
            length = strlen(httphead) + strlen(psend);
            printf("data_send length %d %d  %d\n",length,strlen(psend),strlen(httphead));
            pbuf = (char *)zalloc(length + 1);
            memcpy(pbuf, httphead, strlen(httphead));
            memcpy(pbuf + strlen(httphead), psend, strlen(psend));

        } else {
            sprintf(httphead + strlen(httphead), "\n");
            length = strlen(httphead);
        }

    } else {
        sprintf(httphead, "HTTP/1.0 400 BadRequest\r\n\
Content-Length: 0\r\nServer: lwIP/1.4.0\r\n\n");
        length = strlen(httphead);
    }

    if (psend) {
        /* there are user data need be sent */
        printf("socket %s\ndata to be sent %d bytes\n",pbuf,length);
        espconn_send(pespconn, pbuf, length);
    } else {
        /* no user app data need be sent */
        printf("socket %s\ndata to be send %d bytes\n",httphead,length);
        espconn_send(pespconn, httphead, length);
    }

    if (pbuf) {
        free(pbuf);
        pbuf = NULL;
    }
    printf("data send over\n");
}

Http_Type get_request_type(char *pusrdata){
	char *user_get=NULL,*user_post=NULL;
	user_get = (char *) strstr(pusrdata, "GET");
	user_post = (char *) strstr(pusrdata, "POST");
	if(user_post!=NULL){
		return POST;
	}
	else if(user_get!=NULL){
		return GET;
	}
	return -1;
}
void parse_app_data(void *arg,char *config, char *pusrdata, unsigned short length){
	struct espconn *pespconn = arg;
	char *user_token;
	char *pbuf = (char *) zalloc(256);
	char *data_length=NULL;
	data_length = (char *) strstr(pusrdata, "Content-Length:");
	data_length = strtok(data_length + 15, "\r\n");
	os_printf("Content-Length: %d\n",atoi(data_length));

	switch(get_request_type(pusrdata)){
		case GET:{
			os_printf("We have a get\n");
			char *tmp=(char*)zalloc(8);
			bzero(tmp,8);
			strncpy(tmp,config+16,6);
			os_printf("tmp:%s\n",tmp);
			if(0==strcmp(tmp,"switch")){
				os_printf("get switch success\n");

#if ON_STATUS==0
				sprintf(pbuf,updata_app,!((uint8_t)GPIO_INPUT_GET(NUM_IO)));
#else
				sprintf(pbuf,updata_app,GPIO_INPUT_GET(NUM_IO));
#endif
				os_printf("updata_app:%s\n",pbuf);
				data_send(pespconn,true,pbuf);
			}
			free(tmp);
			tmp=NULL;
		}
		break;
		case POST:{
			os_printf("We have a post\n");
			memcpy(pbuf,pusrdata+(length-atoi(data_length)),atoi(data_length));//cut data except http header
			os_printf("post data: %s\n",pbuf);
			cJSON *json=cJSON_Parse(pbuf);
			if (!json) {
				os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
				}
			else{
			os_printf("JSON : %s \r\n", pbuf);
			cJSON *Request =cJSON_GetObjectItem(json,"Request");
			cJSON *Response=cJSON_GetObjectItem(json,"Response");
			if(Request){
				cJSON *Station=cJSON_GetObjectItem(Request,"Station");
				cJSON *Connect_Station=cJSON_GetObjectItem(Station, "Connect_Station");
				cJSON *token=cJSON_GetObjectItem(Connect_Station, "token");
				if(token){
					memcpy(user_config_flash.token,token->valuestring,40);
					user_config_flash.token[40]='\0';
					os_printf("token is :%s\n", user_config_flash.token);
					bzero(pbuf,256);
					sprintf(pbuf,"%s",user_config_flash.token);
					os_printf("token is :%s\n", pbuf);
				}
				data_send(pespconn,true,NULL);
				xTaskCreate(connect_espressif, "connect_espressif", 256, NULL, 3, NULL);
				xTaskCreate(esp_active_devices, "esp_active_devices", 256, NULL, 3, NULL);
				}
			else if(Response){
				cJSON *status=cJSON_GetObjectItem(Response,"status");
				if(status){
					os_printf("status:%d\n",status->valueint);

		#if ON_STATUS==0
					GPIO_OUTPUT_SET(NUM_IO,!status->valueint);
		#else
					GPIO_OUTPUT_SET(NUM_IO,status->valueint);
		#endif
					bzero(pbuf,256);

#if ON_STATUS==0
					os_printf("io status:%d\n",!((uint8_t)GPIO_INPUT_GET(NUM_IO)));
#else
					os_printf("io status:%d\n",GPIO_INPUT_GET(NUM_IO));
#endif
#if ON_STATUS==0
					sprintf(pbuf,updata_app,!((uint8_t)GPIO_INPUT_GET(NUM_IO)));
#else
					sprintf(pbuf,updata_app,GPIO_INPUT_GET(NUM_IO));
#endif

					os_printf("updata_app:%s\n",pbuf);
					data_send(pespconn,true,pbuf);
					}
				}

			}
			if(json!=NULL)cJSON_Delete(json);
		}
		break;
	}
	os_free(pbuf);
	os_printf("free pbuf\n");
	pbuf=NULL;
	os_printf("pbuf NULl\n");
}

LOCAL void local_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length) {
	os_printf("TCP recv !!! %s \r\n", pusrdata);
	struct espconn *pespconn = arg;
	char *command = NULL;
	command=(char *) strstr(pusrdata, "/config?command=");
	if(command != NULL){
		os_printf("config\n");
		parse_app_data(pespconn,command, pusrdata, length);
	}
	os_printf("Free Heap Size:%d\n",system_get_free_heap_size());
}

/******************************************************************************
 * FunctionName : user_tcp_sent_cb
 * Description  : data sent callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
LOCAL void local_tcp_sent_cb(void *arg) {
	//data sent successfully
	os_printf("tcp sent succeed !!! \r\n");

}
/******************************************************************************
 * FunctionName : user_tcp_discon_cb
 * Description  : disconnect callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
LOCAL void local_tcp_discon_cb(void *arg) {
	//tcp disconnect successfully
	os_printf("tcp disconnect succeed !!! \r\n");
}

enum espconn_level esp_keep;
LOCAL void local_tcp_cb(void *arg) {
	struct espconn *pespconn = arg;
	os_printf("connect succeed !!! \r\n");
	espconn_set_opt(pespconn,ESPCONN_REUSEADDR);
	espconn_regist_recvcb(pespconn, local_tcp_recv_cb);
	espconn_regist_sentcb(pespconn, local_tcp_sent_cb);
	espconn_regist_disconcb(pespconn, local_tcp_discon_cb);

}

void user_local_server_start(void){
	struct ip_info ipconfig;
	wifi_get_ip_info(STATION_IF, &ipconfig);
	espconn_disconnect(&local_tcp);
	espconn_delete(&local_tcp);
	os_printf("esp8266 get token from app \r\n");
	os_printf(IPSTR"\n",IP2STR(&ipconfig.ip));
	local_tcp.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));;
	local_tcp.type = ESPCONN_TCP;
	local_tcp.state = ESPCONN_NONE;
	local_tcp.proto.tcp->local_ip[0]=ip4_addr1_16(&ipconfig.ip);
	local_tcp.proto.tcp->local_ip[1]=ip4_addr2_16(&ipconfig.ip);
	local_tcp.proto.tcp->local_ip[2]=ip4_addr3_16(&ipconfig.ip);
	local_tcp.proto.tcp->local_ip[3]=ip4_addr4_16(&ipconfig.ip);
	local_tcp.proto.tcp->local_port = 80; //local port of ESP8266
	espconn_regist_connectcb(&local_tcp, local_tcp_cb); // register connect callback
	espconn_accept(&local_tcp);
	os_printf("espconn_accept ok  \r\n");
}

void user_local_server_stop(void){
	espconn_disconnect(&local_tcp);
	espconn_delete(&local_tcp);
}

