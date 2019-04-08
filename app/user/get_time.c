/*
 * get_time.c
 *
 *  Created on: 2017年4月17日
 *      Author: Administrator
 */

#include "get_time.h"
#include "esp_timer.h"
#include "device_timer.h"
_calendar_obj calendar;
uint8_t get_weekday(void){

	return GET_WEEK(calendar.w_year,calendar.w_month,calendar.w_day);/*
	0		1		2		3		4		5		6

	星期天	星期一	星期二	星期三	星期四	星期五	星期六
																	      */
}
os_timer_t os_tick_timer;
void tick_time(){
	calendar.timestamp++;//
	calendar.sec++;
	if(calendar.sec>59){
		calendar.sec=0;
		calendar.min++;
		if(calendar.min>59){
			calendar.min=0;
			calendar.hour++;
			if(calendar.hour>23){
				calendar.hour=0;
				calendar.w_day++;
				if(calendar.w_month==1||calendar.w_month==3||calendar.w_month==5||calendar.w_month==7||calendar.w_month==8
						||calendar.w_month==10||calendar.w_month==12){
					if(calendar.w_day>31){
						calendar.w_day=1;
						calendar.w_month++;
					}

				}
				else if(calendar.w_month==4||calendar.w_month==6||calendar.w_month==9||calendar.w_month==11){
					if(calendar.w_day>30){
						calendar.w_day=1;
						calendar.w_month++;
					}

				}
				else{
					if(calendar.w_year/4==0){
						if(calendar.w_day>29){
							calendar.w_day=1;
							calendar.w_month++;
						}
					}
					else{
						if(calendar.w_day>28){
							calendar.w_day=1;
							calendar.w_month++;
						}

					}


				}

			}
		}
		if(calendar.w_month>12){
			calendar.w_month=1;
			calendar.w_year++;
		}
	}
	//printf("%04d-%02d-%02d %02d:%02d:%02d\n",calendar.w_year,calendar.w_month,calendar.w_day,calendar.hour,calendar.min,calendar.sec);
	//printf("timestamp:%d\r\n",calendar.timestamp);
}

void set_time(char *pusrdata){

	cJSON *time_json=cJSON_Parse(pusrdata);//将字符串解析成Json object.
	if (!time_json) {
		os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		return;
	}
	cJSON *date_time  =cJSON_GetObjectItem(time_json,"datetime");
	cJSON *epoch     =cJSON_GetObjectItem(time_json,"epoch");
	calendar.timestamp=(uint32)epoch->valueint;
	char year[5],month[3],day[3],hour[3],min[3],second[3];
	strncpy(year,date_time->valuestring,4);
	strncpy(month,date_time->valuestring+5,2);
	strncpy(day,date_time->valuestring+8,2);
	strncpy(hour,date_time->valuestring+11,2);
	strncpy(min,date_time->valuestring+14,2);
	strncpy(second,date_time->valuestring+17,2);
	year[4]='\0';
	month[2]='\0';
	day[2]='\0';
	hour[2]='\0';
	min[2]='\0';
	second[2]='\0';
	calendar.w_year=atoi(year);
	calendar.w_month=atoi(month);
	calendar.w_day=atoi(day);
	calendar.week=GET_WEEK(atoi(year),atoi(month),atoi(day));
	calendar.hour=atoi(hour);
	calendar.min=atoi(min);
	calendar.sec=atoi(second);
	printf("%04d-%02d-%02d %02d:%02d:%02d\n",calendar.w_year,calendar.w_month,calendar.w_day,calendar.hour,calendar.min,calendar.sec);
	printf("timestamp:%d\r\n",calendar.timestamp);
	cJSON_Delete(time_json);
	printf("Get Done \n");
	read_timer_list();
	os_timer_disarm(&os_tick_timer);
	os_timer_setfn(&os_tick_timer, (os_timer_func_t *) tick_time, NULL);
	os_timer_arm(&os_tick_timer, 1000, 1);

}


