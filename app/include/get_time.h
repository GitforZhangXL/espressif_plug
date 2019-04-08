/*
 * get_time.h
 *
 *  Created on: 2017年4月17日
 *      Author: Administrator
 */

#ifndef APP_INCLUDE_GET_TIME_H_
#define APP_INCLUDE_GET_TIME_H_
#include "c_types.h"
#include "esp_libc.h"
#include "cJSON.h"

typedef struct
{
	uint32_t timestamp;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	//公历日月年周
	uint16_t w_year;
	uint8_t  w_month;
	uint8_t  w_day;
	uint8_t  week;
}_calendar_obj;




#define GET_WEEK(y,m,d) (0<=((d + 2*m + 3*(m+1)/5 + y + y/4 - y/100 + y/400) % 7)&&((d + 2*m + 3*(m+1)/5 + y + y/4 - y/100 + y/400) % 7)<=5?((d + 2*m + 3*(m+1)/5 + y + y/4 - y/100 + y/400) % 7)+1:0)

//星期天 -星期六   （0-6）
void set_time(char *pusrdata);
uint8_t get_weekday(void);
void tick_time(void);
#endif /* APP_INCLUDE_GET_TIME_H_ */

