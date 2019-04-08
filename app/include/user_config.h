#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define USER_UPGRADE_TEST 0	//云升级测试
/* * IO 口定义     * */
#define PERIPHS_IO PERIPHS_IO_MUX_GPIO0_U
#define FUNC_IO FUNC_GPIO0
#define NUM_IO 0

#define FLASH_SIZE 4096
//各种参数保存地址定义
#if FLASH_SIZE==1024
//1M FLASH
#define USER_PARAM_START_SEC 0x7D000
#define MASTER_DEVICE_KEY 0x7C000
#define TIMER_PARAM_SEC 0x7B000


//4M FLASH
#elif FLASH_SIZE==4096
#define USER_PARAM_START_SEC 0x27D000
#define MASTER_DEVICE_KEY 0x27C000
#define TIMER_PARAM_SEC 0x27B000

#else
	#error "Please define flash size !"

#endif

struct user_saved_param {
    unsigned char devkey[41];
    unsigned char token[41];
    unsigned char activeflag;
    unsigned char pad;
};
#define ON_STATUS 0
#define DEFAULT_STATUS 1


#endif

