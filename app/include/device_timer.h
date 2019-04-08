/*
 * device_timer.h
 *
 *  Created on: 2018年2月22日
 *      Author: Administrator
 */

#ifndef APP_INCLUDE_DEVICE_TIMER_H_
#define APP_INCLUDE_DEVICE_TIMER_H_
#define TIMER_MAX 150
#define LEN sizeof(struct Alarm_timer)
//#define FS1_FLASH_SEC      (2560*1024)
//#define FS2_FLASH_SEC      (4000*1024)
#define MAX_WRITE      		50000
#define SECTOR_SIZE         (4*1024)
#define 	SUN  0x40//周日
#define 	MON  0x01//周一
#define 	TUE  0x02//周二
#define 	WED  0x04//周三
#define 	THU  0x08//周四
#define 	FRI  0x10//周五
#define 	SAT  0x20//周六

#include "get_time.h"
typedef enum {
	FIXEDTIME=10,
	LOOP_PERIOD,
	LOOP_IN_WEEK
}timer_type;

typedef enum {
	second=20,
	minute,
	hour,
	day
}timer_period;

typedef enum {
	on_switch=30,
	off_switch,
	on_off_switch
}timer_action;


struct Alarm_timer{// 24byte

	uint32_t timer_id;// 4byte
	uint32_t wait_second;//4byte
	uint32_t mark_time;//4byte
	uint8_t timer_type;// 1byte
	uint8_t timer_period;// 1byte
	uint8_t flag;// 1byte
	uint8_t timer_weekday;// 1 byte
	uint8_t hour;//1
	uint8_t mint;//1
	uint8_t action;// 1byte
	uint8_t second;// 1byte
	struct Alarm_timer *next;//4

};

extern struct Alarm_timer *PHEAD;//
struct Alarm_timer *create_list(void);
struct Alarm_timer *insert_list(struct Alarm_timer *head ,struct Alarm_timer *stud);
struct Alarm_timer *del_list(struct Alarm_timer  *head,uint32_t id);
bool save_list(struct Alarm_timer *head);
void read_timer_list(void);
void traverse_list(struct Alarm_timer *head);
void plug_on(void);
void plug_off(void);
void plug_on_off(void);
void timer_loop(void *arg);
void espressif_timer(void *arg, char *pusrdata, unsigned short length);
void del_timer_from_server(void);
uint32_t format_timestamp(char* time);
bool timer_list_init(void);
#endif /* APP_INCLUDE_ESP_TIMER_H_ */
