#ifndef _IR_TEST_H
#define _IR_TEST_H
#include "c_types.h"
#include "driver/ringbuf.h"



//#define ping_server "{\"path\": \"/v1/ping/\", \"method\": \"POST\", \"meta\": {\"Authorization\": \"token "device_token"\"}}\n\n"
//#define active_devices "{\"path\": \"/v1/device/activate/\", \"method\": \"POST\", \"meta\": {\"Authorization\":\"token %s\"}, \"body\": {\"encrypt_method\": \"PLAIN\",\"bssid\": \"%s\",\"token\": \"a64480513d0937104a2897d73634c6aa37e7ff38\"}}\n\n"
//#define identify "{\"nonce\": 560192812, \"path\": \"/v1/device/identify\", \"method\": \"GET\", \"meta\":{\"Authorization\": \"token %s\"}}\n\n"
//#define up_status "{\"status\": 200, \"datapoint\": {\"x\": %d}, \"nonce\": %s, \"is_query_device\":true}\n\n"
//#define return_action "{\"status\": 200, \"nonce\": %s, \"deliver_to_device\":true}\n\n"

typedef enum {
	FRC1_SOURCE = 0, NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;

//===============================================
//    IR TX MODE CONFIG
//-------------------------------------------
/*There are 2 ways to generate 38k clk signal*/
/*CLK SOURCE MODE*/
/*MODE 1. generated from IIS clk. 
 In this mode :MTMS/GPIO2/MTCK/MTDO can be choosed.
 MTMS and GPIO2 are recommended to get more accurate 38k clk*/
/*MODE 2. generated from GPIO sigma-delta module*/
/* 
 IF  GEN_IR_CLK_FROM_IIS==1:  MODE 1
 ELSE GEN_IR_CLK_FROM_IIS==0:  MODE 2
 */

#define GEN_IR_CLK_FROM_IIS 1

#define IR_GPIO_OUT_MUX PERIPHS_IO_MUX_GPIO2_U
#define IR_GPIO_OUT_NUM 2
#define IR_GPIO_OUT_FUNC  FUNC_GPIO2

//-------------------------------------------
//================================================

//===============================================
//    IR RX MODE CONFIG
//-------------------------------------------
extern RINGBUF IR_RX_BUFF;
#define RX_RCV_LEN 128

#define IR_GPIO_IN_NUM 4
#define IR_GPIO_IN_MUX PERIPHS_IO_MUX_GPIO4_U
#define IR_GPIO_IN_FUNC  FUNC_GPIO4
//-------------------------------------------
//================================================

#define IR_POTOCOL_NEC 0
#define IR_POTOCOL_RC5 1  //not support yet 

#define IR_NEC_STATE_IDLE 0
#define IR_NEC_STATE_PRE 1
#define IR_NEC_STATE_CMD 2
#define IR_NEC_STATE_REPEAT 3

typedef enum {
	IR_TX_IDLE,
	IR_TX_IDLE_1,
	IR_TX_HEADER,
	IR_TX_HEADER_1,
	IR_TX_DATA,
	IR_TX_DATA_1,
	IR_TX_REP,
	IR_TX_LINK_N,
	IR_TX_LINK_Y,
} IR_TX_STATE;

#define IR_NEC_BIT_NUM 		8
#define IR_NEC_MAGIN_US 		200
#define IR_NEC_TM_PRE_US 		13500
#define IR_NEC_D1_TM_US 		2250
#define IR_NEC_D0_TM_US 		1280
#define IR_NEC_REP_TM1_US 	20000
#define IR_NEC_REP_TM2_US 	11250
#define IR_NEC_REP_LOW_US 	2350
#define IR_NEC_REP_CYCLE 	110000

#define IR_NEC_HEADER_HIGH_US 9000
#define IR_NEC_HEADER_LOW_US 	4500
#define IR_NEC_DATA_HIGH_US 	560
#define IR_NEC_DATA_LOW_1_US 	1680
#define IR_NEC_DATA_LOW_0_US 	560

void ir_rx_init(void);
void ir_rx_disable();
void ir_rx_enable();
uint8 SET_Temp(uint8 data);
uint8 SET_Moshi(uint8 moshi);
uint8 SET_Fengsu(uint8 fs);
void SET_Power(uint8 xx);
void set_check(void);

void ir_tx_init();
uint8 get_ir_tx_status();
void ir_tx_handler();

#endif
