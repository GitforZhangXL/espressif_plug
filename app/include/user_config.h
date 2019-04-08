#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__
struct user_saved_param {
    unsigned char devkey[41];
    unsigned char token[41];
    unsigned char activeflag;
    unsigned char pad;
};
#define USER_UPGRADE_TEST 0
#define PERIPHS_IO PERIPHS_IO_MUX_GPIO0_U
#define FUNC_IO FUNC_GPIO0
#define NUM_IO 0
#define USER_PARAM_START_SEC 0x7D000
#define MASTER_DEVICE_KEY 0x7C000
#define TIMER_PARAM_SEC 0x7B000
#define ON_STATUS 0
#define DEFAULT_STATUS 1

#endif

