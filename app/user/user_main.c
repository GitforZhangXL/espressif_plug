#include "esp_common.h"
#include "driver/18b20.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/key.h"
#include "espconn.h"
#include "user_config.h"
#include "device_timer.h"
#include "user_esp_platform.h"
extern struct user_saved_param user_config_flash;
extern os_timer_t change_ap_timer;
/******************************************************************************
 * FunctionName : user_set_station_config
 * Description  : set the router info which ESP8266 station will connect to
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_station_config(void) {
	os_printf("config wifi\n");
	struct station_config stationConf;
	wifi_set_opmode(STATION_MODE);
	//need not mac address
	stationConf.bssid_set = 0;
#if USER_UPGRADE_TEST
	memcpy(&stationConf.ssid, "ssid", 32);
	memcpy(&stationConf.password, "yourpasswd", 64);
#else
	memcpy(&stationConf.ssid, "ssid", 32);
	memcpy(&stationConf.password, "yourpasswd", 64);
#endif
	wifi_station_set_config(&stationConf);
	wifi_station_disconnect();
	wifi_station_connect();
}


uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


void key_short(void){
	os_printf("key_short\n");
}

void key_long(void){
	os_printf("key_long\n");
	user_config_flash.activeflag=DEVICE_ACTIVE_FAIL;
	user_devicefind_start();
	user_local_server_start();
	ReadData();
}


void user_key_init(void){
	struct keys_param *keys=(struct keys_param *)zalloc(sizeof(struct keys_param ));
	struct single_key_param **single=(struct single_key_param **)zalloc(sizeof(struct single_key_param ));
	single[0]=key_init_single(key_gpio_id,key_gpio_name,key_gpio_func,key_long,key_short);
	keys->single_key=single;
	keys->key_num=1;
	key_init(keys);
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/



void user_init(void) {

	wifi_station_ap_number_set(5);
	os_timer_setfn(&change_ap_timer, (os_timer_func_t *) user_check_ip, NULL);//user_ap_change
	os_timer_arm(&change_ap_timer, 10000, 0);
	wifi_set_opmode(STATION_MODE);
	if (system_upgrade_userbin_check() == 0x00) {//user1.bin
        os_printf("\nJump to user1.bin\n");
    } else if (system_upgrade_userbin_check() == 0x01) {//user2.bin
        os_printf("\nJump to user2.bin\n");
    }
	espconn_init();
	wifi_set_event_handler_cb(wifi_event_cb);
#ifdef DEBUG_MOD
	gpio16_output_conf();
	gpio16_output_set(1);
	gpio16_input_conf();
	printf("gpio16_input_get:%d\r\n",gpio16_input_get());
#else
	PIN_FUNC_SELECT(PERIPHS_IO, FUNC_IO);
	GPIO_OUTPUT_SET(NUM_IO,DEFAULT_STATUS);
#endif
//	user_key_init();


	os_printf("Unix:%d\r\n",LEN);
	os_printf("Unix:%d\r\n",format_timestamp("20180222134000"));
	os_printf("Free Heap Size:%d\n",system_get_free_heap_size());
}
