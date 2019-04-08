#ifndef __USER_DEVICE_H__
#define __USER_DEVICE_H__
#include "c_types.h"
#include "esp_common.h"
#include "ipv4/lwip/ip4_addr.h"
#include "espconn.h"
#define packet_size   (2 * 1024)
#define updata_app "{\"Response\":{\"status\":%d}}\n\n"
#define ping_server "{\"path\": \"/v1/ping/\", \"method\": \"POST\", \"meta\": {\"Authorization\": \"token %s\"}}\n\n"
#define active_devices "{\"nonce\": %d,\"path\": \"/v1/device/activate/\", \"method\": \"POST\", \"body\": {\"encrypt_method\": \"PLAIN\", \"token\": \"%s\", \"bssid\": \""MACSTR"\"}, \"meta\": {\"Authorization\": \"token %s\"}}\n"
#define identify "{\"nonce\": 560192812, \"path\": \"/v1/device/identify\", \"method\": \"GET\", \"meta\":{\"Authorization\": \"token %s\"}}\n\n"
#define up_status "{\"status\": 200, \"datapoint\": {\"x\": %d}, \"nonce\": %s, \"is_query_device\":true}\n\n"
#define return_action "{\"status\": 200, \"nonce\": %s, \"deliver_to_device\":true}\n\n"
#define return_data "{\"status\": 200, \"nonce\": %d, \"datapoint\": {\"x\": %d},\"deliver_to_device\":true}\n\n"
//#define DEBUG_MOD

char sta_addr[6];

enum {
    DEVICE_CONNECTING = 40,
    DEVICE_ACTIVE_DONE,
    DEVICE_ACTIVE_FAIL,
    DEVICE_CONNECT_SERVER_FAIL
};

struct dhcp_client_info {
	ip_addr_t ip_addr;
	ip_addr_t netmask;
	ip_addr_t gw;
	uint8 flag;
	uint8 pad[3];
};
typedef enum {
	ping_beagin=0,
	ping_suc=1,
	ping_time_out=2

}STATUS_P;
extern struct _esp_tcp user_tcp;
extern  struct espconn espressif_tcp;
void wifi_event_cb(System_Event_t *event);
void user_ap_change(void);
void check_ip();
void user_check_ip(void);
void esp_active_devices(void *pvParameter);
void connect_espressif(void *pvParameters);
int  parse_nonce(char *pbuffer);
void send_ping_pack(void);
#endif
