/*
 * get_token.h
 *
 *  Created on: 2017��3��24��
 *      Author: Administrator
 */

#ifndef GET_TOKEN_H_
#define GET_TOKEN_H_
typedef enum{
	GET=1,
	POST=2,
}Http_Type;

void user_local_server_start(void);
void user_local_server_stop(void);
void data_send(void *arg,bool responseOK, char *psend);
void parse_app_data(void *arg,char *config, char *pusrdata, unsigned short length);
#endif /* GET_TOKEN_H_ */
