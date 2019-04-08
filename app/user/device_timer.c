/*
 * esp_time.c
 *
 *  Created on: 2018年2月22日
 *      Author: Administrator
 */
#include "user_esp_platform.h"
#include "device_timer.h"
#include "get_time.h"
#include "esp_common.h"
#include "driver/gpio.h"
#include "cJSON.h"
#include "user_config.h"
#define TIMER_FRAME     "{\"status\":200, \"timers\":%s, \"nonce\": %d, \"deliver_to_device\": true}\n"
#define DELETE_TIMER_HEAD   "POST /v1/device/timers/?method=DELETE&is_humanize_format=true HTTP/1.1\r\n\
Host: iot.espressif.cn\r\n\
Connection: keep-alive\r\n\
Content-Length: %d\r\n\
User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 Safari/537.36\r\n\
Cache-Control: no-cache\r\n\
Content-Type: text/plain;charset=UTF-8\r\n\
Authorization: token %s\r\n\
Accept: */*\r\n\
Accept-Encoding: gzip, deflate\r\n\
Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7\r\n\r\n\
%s"

#define DELETE_TIMER_BODY "{\"timers\":[{\"id\":%d}]}"
extern _calendar_obj calendar;


extern struct user_saved_param user_config_flash;

volatile uint32 the_delete_timer_id;
struct espconn del_timer_tcp;


void del_timer_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length){

	printf("recv from http://iot.espressif.cn\r\n ");
	printf("%s\r\n ",pusrdata);
	espconn_disconnect(&del_timer_tcp);
	espconn_delete(&del_timer_tcp);

}

void del_timer_tcp_sent_cb(void *arg) {

	os_printf("send http://iot.espressif.cn successfully  \r\n");

}
void del_timer_tcp_cb(void *arg){

	struct espconn *pespconn = arg;
	char *pbuffer=(char*)malloc(1024);
	char *pbuffer_body=(char*)malloc(256);
	memset(pbuffer,0,1024);
	memset(pbuffer_body,0,256);
	sprintf(pbuffer_body,DELETE_TIMER_BODY,the_delete_timer_id);
	sprintf(pbuffer,DELETE_TIMER_HEAD,strlen(pbuffer_body),user_config_flash.devkey,pbuffer_body);
	os_printf("pbuffer:%s",pbuffer);
	espconn_send(&del_timer_tcp,pbuffer,strlen(pbuffer));
	free(pbuffer);
	free(pbuffer_body);
	pbuffer_body=NULL;
	pbuffer=NULL;
	espconn_regist_recvcb(pespconn, del_timer_tcp_recv_cb);
	espconn_regist_sentcb(pespconn, del_timer_tcp_sent_cb);


}
struct _esp_tcp timer_tcp;
extern char server_ip[4];
void del_timer_from_server(void){

	os_printf("del_timer_from_server\r\n");
	espconn_disconnect(&del_timer_tcp);
	espconn_delete(&del_timer_tcp);
	os_printf("Connect to http://iot.espressif.cn\r\n");
	del_timer_tcp.proto.tcp = &timer_tcp;
	del_timer_tcp.type = ESPCONN_TCP;
	del_timer_tcp.state = ESPCONN_NONE;
	memcpy(del_timer_tcp.proto.tcp->remote_ip, server_ip, 4);// remote IP of TCP server
	del_timer_tcp.proto.tcp->remote_port = 80;  // remote port
	del_timer_tcp.proto.tcp->local_port = espconn_port(); //local port of ESP8266
	espconn_regist_connectcb(&del_timer_tcp, del_timer_tcp_cb); // register connect callback
	espconn_connect(&del_timer_tcp);

}
struct Alarm_timer *PHEAD=NULL;
void Set_timestamp(uint32_t timestamp){

	calendar.timestamp=timestamp;

}

//判断是否是闰年函数
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
unsigned char Is_Leap_Year(unsigned short year)
{
	if(year%4==0) //必须能被4整除
	{
		if(year%100==0)
		{
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除
			else return 0;
		}else return 1;
	}else return 0;
}
//根据时间日期字符串格式化为 timestamp,
//examples:"20180222134000",2018年,2月22号,13:40:00
const unsigned char mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

uint32_t format_timestamp(char* time)
{
	uint16_t t;
	uint32_t seccount=0;
	char year[5],month[3],day[3],hour[3],min[3],sec[3];
	strncpy(year,time,4);
	strncpy(month,time+4,2);
	strncpy(day,time+6,2);
	strncpy(hour,time+8,2);
	strncpy(min,time+10,2);
	strncpy(sec,time+12,2);
	year[4]='\0';
	month[2]='\0';
	day[2]='\0';
	hour[2]='\0';
	min[2]='\0';
	sec[2]='\0';
	if(atoi(year)<1970||atoi(year)>2099)return false;
	for(t=1970;t<atoi(year);t++)	//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))seccount+=31622400;//闰年的秒钟数
		else seccount+=31536000;			  //平年的秒钟数
	}
	for(t=0;t<atoi(month)-1;t++)	   //把前面月份的秒钟数相加
	{
		seccount+=(uint32_t)mon_table[t]*86400;//月份秒钟数相加
		if(Is_Leap_Year(atoi(year))&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数
	}
	//printf("Set_Date seccount:%d\r\n",seccount);
	seccount+=(uint32_t)(atoi(day)-1)*86400;//把前面日期的秒钟数相加
	seccount+=(uint32_t)atoi(hour)*3600;//小时秒钟数
    seccount+=(uint32_t)atoi(min)*60;	 //分钟秒钟数
	seccount+=atoi(sec);//最后的秒钟加上去
	return seccount-28800;//8小时
}


void format_hour_mint(char* time,uint8_t *thour,uint8_t *tmint)
{

	char hour[3],min[3];
	strncpy(hour,time,2);
	strncpy(min,time+2,2);
	hour[2]='\0';
	min[2]='\0';
	*thour=atoi(hour);
	*tmint=atoi(min);

}

const char WEEK_CHINA[][7]={"星期天","星期一","星期二","星期三","星期四","星期五","星期六"};
void espressif_timer(void *arg, char *pusrdata, unsigned short length){
	char *hasbody = (char*) strstr(pusrdata, "body");
	if (hasbody==NULL)	return;
	if(PHEAD->timer_id>TIMER_MAX)return;
	cJSON *json=cJSON_Parse(pusrdata);//将字符串解析成Json object.
	if (!json) {
		os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(json);
	}
	else{
		struct espconn *pespconn = arg;
		int nonce = parse_nonce(pusrdata);
		char *_del = (char *) strstr(pusrdata, "DELETE");
		char *_post = (char *) strstr(pusrdata, "POST");
		char *_put = (char *) strstr(pusrdata, "PUT");
		char *_FIXEDTIME = (char *) strstr(pusrdata, "FIXEDTIME");
		char *_LOOP_PERIOD = (char *) strstr(pusrdata, "LOOP_PERIOD");
		char *_LOOP_IN_WEEK = (char *) strstr(pusrdata, "LOOP_IN_WEEK");
		char *pbuf = (char *) os_zalloc(packet_size);
		cJSON *body  =cJSON_GetObjectItem(json,"body");
		cJSON *timers=cJSON_GetObjectItem(body,"timers");
		char *temp=cJSON_PrintUnformatted(timers);
		cJSON *timer_body = cJSON_GetArrayItem(timers,0);
		if(_post!=NULL){//收到post，本地什么都不做，直接返回创建成功即可，因为没有ID，

			sprintf(pbuf,TIMER_FRAME,temp,nonce);
			os_printf("timers:%s\r\n",pbuf);
			espconn_send(pespconn, pbuf, strlen(pbuf));
		}
		else if(_put!=NULL){

			if(_FIXEDTIME!=NULL){
				cJSON *timer_action_arry_0 = cJSON_GetArrayItem(cJSON_GetObjectItem(timer_body,"time_actions"),0);
				if(calendar.timestamp>=format_timestamp(cJSON_GetObjectItem(timer_action_arry_0,"time")->valuestring))
					goto end;
				struct Alarm_timer *p_timer=create_list();
				p_timer->timer_id = cJSON_GetObjectItem(timer_body,"id")->valueint;
				p_timer->timer_type=FIXEDTIME;
				p_timer->wait_second=format_timestamp(cJSON_GetObjectItem(timer_action_arry_0,"time")->valuestring);
				if(strcmp("on_switch",cJSON_GetObjectItem(timer_action_arry_0,"action")->valuestring)==0){
					p_timer->action=on_switch;

				}
				else if(strcmp("off_switch",cJSON_GetObjectItem(timer_action_arry_0,"action")->valuestring)==0){
					p_timer->action=off_switch;

				}
				else if(strcmp("on_off_switch",cJSON_GetObjectItem(timer_action_arry_0,"action")->valuestring)==0){
					p_timer->action=on_off_switch;

				}

				if(insert_list(PHEAD,p_timer)){
					save_list(PHEAD);
					traverse_list(PHEAD);
					sprintf(pbuf,TIMER_FRAME,temp,nonce);
					espconn_send(pespconn, pbuf, strlen(pbuf));
					free(temp);
					temp=NULL;
				}
				cJSON_Delete(timer_action_arry_0);
				goto end;
			}
			else if(_LOOP_PERIOD!=NULL){
				struct Alarm_timer *p_timer=create_list();
				p_timer->timer_id = cJSON_GetObjectItem(timer_body,"id")->valueint;
				p_timer->timer_id = cJSON_GetObjectItem(timer_body,"id")->valueint;
				p_timer->timer_type=LOOP_PERIOD;
				//执行命令判断 开始
				if(strcmp("on_switch",cJSON_GetObjectItem(timer_body,"action")->valuestring)==0){
					p_timer->action=on_switch;

				}
				else if(strcmp("off_switch",cJSON_GetObjectItem(timer_body,"action")->valuestring)==0){
					p_timer->action=off_switch;

				}
				else if(strcmp("on_off_switch",cJSON_GetObjectItem(timer_body,"action")->valuestring)==0){
					p_timer->action=on_off_switch;

				}//执行命令判断 结束
				//时间单位判断开始
				if(strcmp("second",cJSON_GetObjectItem(timer_body,"period")->valuestring)==0){
					p_timer->wait_second=cJSON_GetObjectItem(timer_body,"time")->valueint;

				}
				else if(strcmp("minute",cJSON_GetObjectItem(timer_body,"period")->valuestring)==0){
					p_timer->wait_second=cJSON_GetObjectItem(timer_body,"time")->valueint*60;

				}
				else if(strcmp("hour",cJSON_GetObjectItem(timer_body,"period")->valuestring)==0){
					p_timer->wait_second=cJSON_GetObjectItem(timer_body,"time")->valueint*3600;

				}
				else if(strcmp("day",cJSON_GetObjectItem(timer_body,"period")->valuestring)==0){
					p_timer->wait_second=cJSON_GetObjectItem(timer_body,"time")->valueint*86400;

				}//时间单位判断结束
				p_timer->mark_time=calendar.timestamp;
				printf("wait_second:%d\r\n",p_timer->wait_second);
				if(insert_list(PHEAD,p_timer)){
					save_list(PHEAD);
					traverse_list(PHEAD);
					sprintf(pbuf,TIMER_FRAME,temp,nonce);
					espconn_send(pespconn, pbuf, strlen(pbuf));
					free(temp);
					temp=NULL;
				}
				goto end;
			}
			else if(_LOOP_IN_WEEK!=NULL){

				struct Alarm_timer *p_timer=create_list();
				cJSON *timer_action_arry_0 = cJSON_GetArrayItem(cJSON_GetObjectItem(timer_body,"time_actions"),0);
				p_timer->timer_id = cJSON_GetObjectItem(timer_body,"id")->valueint;
				int weekday_arry_len=cJSON_GetArraySize(cJSON_GetObjectItem(timer_body,"weekdays"));
				if(weekday_arry_len==0){free(p_timer);goto end;}
				cJSON *weekday_arry = cJSON_GetObjectItem(timer_body,"weekdays");
				p_timer->timer_type=LOOP_IN_WEEK;
				int i,t;
				//先清零星期标志
				p_timer->timer_weekday=0x00;
				for(i=0;i<weekday_arry_len;i++){
					t=cJSON_GetArrayItem(weekday_arry,i)->valueint;
					printf("\r\nday:%s\r\n",WEEK_CHINA[t]);
					switch(t){
						case 0:{
							p_timer->timer_weekday|=SUN;//周日
						}break;
						case 1:{
							p_timer->timer_weekday|=MON;//周一
						}break;
						case 2:{
							p_timer->timer_weekday|=TUE;//周二
						}break;
						case 3:{
							p_timer->timer_weekday|=WED;//周三
						}break;
						case 4:{
							p_timer->timer_weekday|=THU;//周四
						}break;
						case 5:{
							p_timer->timer_weekday|=FRI;//周五
						}break;
						case 6:{
							p_timer->timer_weekday|=SAT;//周六
						}break;

					}

				}
				printf("timer_weekday:%d\r\n",p_timer->timer_weekday);
				if(strcmp("on_switch",cJSON_GetObjectItem(timer_action_arry_0,"action")->valuestring)==0){
					p_timer->action=on_switch;

				}
				else if(strcmp("off_switch",cJSON_GetObjectItem(timer_action_arry_0,"action")->valuestring)==0){
					p_timer->action=off_switch;

				}
				else if(strcmp("on_off_switch",cJSON_GetObjectItem(timer_action_arry_0,"action")->valuestring)==0){
					p_timer->action=on_off_switch;

				}//执行命令判断 结束

				format_hour_mint(cJSON_GetObjectItem(timer_action_arry_0,"time")->valuestring,&p_timer->hour,&p_timer->mint);
				p_timer->second=0;
				printf("time %d:%d",p_timer->hour,p_timer->mint);
				if(insert_list(PHEAD,p_timer)){
					save_list(PHEAD);
					traverse_list(PHEAD);
					sprintf(pbuf,TIMER_FRAME,temp,nonce);
					espconn_send(pespconn, pbuf, strlen(pbuf));
					free(temp);
					temp=NULL;
					}
				cJSON_Delete(timer_action_arry_0);
				goto end;
			}
		}
		else if(_del!=NULL){
			if(false==del_list(PHEAD,cJSON_GetObjectItem(timer_body,"id")->valueint))goto end;
			save_list(PHEAD);
			traverse_list(PHEAD);
			sprintf(pbuf,TIMER_FRAME,temp,nonce);
			espconn_send(pespconn, pbuf, strlen(pbuf));
			goto end;
		}
end:{
	os_printf("\nEnd !\n");
	free(temp);
	free(pbuf);
	cJSON_Delete(timer_body);
	cJSON_Delete(json);}

	}


}

void plug_on(void){
#ifdef DEBUG_MOD
			gpio16_output_conf();
			gpio16_output_set(0);
#else
#if ON_STATUS==0
			GPIO_OUTPUT_SET(NUM_IO,0);
#else
			GPIO_OUTPUT_SET(NUM_IO,1);
#endif

#endif
}
void plug_off(void){
#ifdef DEBUG_MOD
			gpio16_output_conf();
			gpio16_output_set(1);
#else
#if ON_STATUS==0
			GPIO_OUTPUT_SET(NUM_IO,1);
#else
			GPIO_OUTPUT_SET(NUM_IO,0);
#endif
#endif

}
void plug_on_off(void){
	uint8_t status;
#ifdef DEBUG_MOD
	gpio16_input_conf();
	status=gpio16_input_get();
	gpio16_output_conf();
	if(status)//反向开关
		gpio16_output_set(0);
	else
		gpio16_output_set(1);
#else


#if ON_STATUS==0
		status=!((uint8_t)GPIO_INPUT_GET(NUM_IO));
#else
		status=GPIO_INPUT_GET(NUM_IO);
#endif

	if(status)//反向开关
		plug_off();
	else
		plug_on();

#endif

}

//建立链表
struct Alarm_timer *create_list(void)
{
    struct Alarm_timer *head;
    head=(struct Alarm_timer*)malloc(LEN);
    bzero(head,LEN);
    return (head);
}

//插入接点
 struct Alarm_timer *insert_list(struct Alarm_timer *head ,struct Alarm_timer *stud)
{
	struct Alarm_timer *p1;
	del_list(head, stud->timer_id);
	if (head == NULL||stud->timer_id<1000)     //原来的链表是空表
	{
		return NULL;
	} else {
		p1 = head;         //使p1指向第一个结点
		while (p1->next) {

			p1 = p1->next;    //p1后移一个结点

		}
		stud->next = NULL;    //插到最后的结点之后
		p1->next = stud;
		head->timer_id++;
	}

	return (head);
}


 //保存节点到文件
bool save_list(struct Alarm_timer *head)
{
	uint8_t i=0;
	int count=LEN*(head->timer_id+1);
	struct Alarm_timer *p0,*p;
	struct Alarm_timer *p1=(struct Alarm_timer*)malloc(LEN);

	if(SPI_FLASH_RESULT_OK!=spi_flash_erase_sector(TIMER_PARAM_SEC/SECTOR_SIZE))return false;

	p0=(struct Alarm_timer*)malloc(count);
	bzero(p0,count);
	p=head;
	if(p!=NULL)
	do
	{
		memcpy(p1,p,LEN);
		p1->next=NULL;//存储时必须清空该指针，不然下次读取会读取到异常地址
		memcpy(&p0[i++],p1,LEN);
		p=p->next;
	}
	while(p!=NULL);
	if(SPI_FLASH_RESULT_OK!=spi_flash_write(TIMER_PARAM_SEC,(uint32_t*)p0,count))
	free(p1);
	free(p0);
	p1=NULL;
	p0=NULL;
	return true;
}




//删除结点
struct Alarm_timer *del_list(struct Alarm_timer  *head,uint32_t id)
{
	struct Alarm_timer *p1,*p2;

	if(head==NULL)	{printf("\nlist null!\n");return NULL;}
	else if(head->next==NULL){printf("\nhead list null!\n");return NULL;}
	p1=head;
	while(id!=p1->timer_id && p1->next!=NULL)   //p1指向不是所要找的结点，并且后面还有结点
	{
		p2=p1;
		p1=p1->next;      //p1后移一个节点
	}       //p1后移一个结点
	if(id==p1->timer_id){     //找到了
		p2->next=p1->next;      //or 将下一个结点的地址赋给前一个结点地址
		printf("delete:%d\n",id);
		head->timer_id--;;
		free(p1);
		p1=NULL;
	}
	else
		printf("%d not been found !\n",id);

	return(head);


}

 //遍历链表
void traverse_list(struct Alarm_timer *head)
{
	struct Alarm_timer *p;
	p=head->next;
	if(p!=NULL)
	do
	{
		 printf("timer_id:%d\n",p->timer_id);
		 p=p->next;
	}
	while(p!=NULL);

}

void timer_loop(void *arg){

	 portTickType xLastWakeTime;
	 const portTickType xFrequency = 100;

	 // Initialise the xLastWakeTime variable with the current time.
	 xLastWakeTime = xTaskGetTickCount ();
	struct Alarm_timer *p;
	while(1){

		p=PHEAD->next;
		if(p!=NULL)
		do
		{
			if(p->timer_type==FIXEDTIME){
				if(p->wait_second==calendar.timestamp){
					if(p->action==on_switch){
						plug_on();
					}
					else if(p->action==off_switch){
						plug_off();
					}
					else if(p->action==on_off_switch){
						plug_on_off();
					}
					the_delete_timer_id=p->timer_id;
					del_timer_from_server();
					del_list(PHEAD,the_delete_timer_id);
					save_list(PHEAD);
				}

			}
			else if(p->timer_type==LOOP_PERIOD){
				//printf("in loop timestamp:%d %d\r\n",p->mark_time,p->wait_second);
				if(p->mark_time+p->wait_second==calendar.timestamp){
					if(p->action==on_switch){
						plug_on();
					}
					else if(p->action==off_switch){
						plug_off();
					}
					else if(p->action==on_off_switch){
						plug_on_off();
					}
					p->mark_time=calendar.timestamp;
				}

			}
			else if(p->timer_type==LOOP_IN_WEEK){
				uint8_t tz=0;
				switch(get_weekday()){
					case 0:{
						tz=SUN;
					}break;
					case 1:{
						tz=MON;
					}break;
					case 2:{
						tz=TUE;
					}break;
					case 3:{
						tz=WED;
					}break;
					case 4:{
						tz=THU;
					}break;
					case 5:{
						tz=FRI;
					}break;
					case 6:{
						tz=SAT;
					}break;

				}

				if((tz&p->timer_weekday)>0){
//					printf("星期满足： 0x%02X\r\n",tz&p->timer_weekday);
					if((p->hour==calendar.hour)&&(p->mint==calendar.min)&&(p->second==calendar.sec)){
					if(p->action==on_switch){
							plug_on();
						}
						else if(p->action==off_switch){
							plug_off();
						}
						else if(p->action==on_off_switch){
							plug_on_off();
						}
					}
				}

			}


			p=p->next;
		}
		while(p!=NULL);
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}

void read_timer_list(void){

	int fd;
	struct Alarm_timer *p1=create_list();
	u8_t count=0;
	PHEAD=create_list();//初始化链表头;
	bzero(PHEAD,LEN);
	bzero(p1,LEN);
	u8_t i=0;
	struct Alarm_timer *pbuf;
	struct Alarm_timer *p,*p2;
	if(SPI_FLASH_RESULT_OK==spi_flash_read(TIMER_PARAM_SEC,(uint32 *)p1,LEN)){
		count=p1->timer_id+1;
		if(count>TIMER_MAX)return;
		pbuf=(struct Alarm_timer *)malloc(count*LEN);
		if(SPI_FLASH_RESULT_OK==spi_flash_read(TIMER_PARAM_SEC,(uint32 *)pbuf,count*LEN)){
			while(count-->0){
				pbuf[i].mark_time=calendar.timestamp;
				insert_list(PHEAD,&pbuf[i++]);

			}
			traverse_list(PHEAD);
		}
		xTaskCreate(timer_loop, "timer_loop", 512, NULL, 9, NULL);
	}

}
