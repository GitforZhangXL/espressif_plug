/*
 * ds18b20.c
 *
 *  Created on: 2017��3��15��
 *      Author: Administrator
 */
#include "driver/18b20.h"
#include "driver/gpio.h"
#include "esp8266/eagle_soc.h"
#include "c_types.h"
#include "lwip/mem.h"
static uint8 IO_18B20=4;
//GPIO_OUTPUT_SET
//GPIO_INPUT_GET
//��λDS18B20
void DS18B20_Rst(void)
{
	GPIO_OUTPUT_SET(IO_18B20,0);
    os_delay_us(750);    //����750us
    GPIO_OUTPUT_SET(IO_18B20,1);; //DQ=1
	os_delay_us(15);     //15US
}
//�ȴ�DS18B20�Ļ�Ӧ
//����1:δ��⵽DS18B20�Ĵ���
//����0:����
u8 DS18B20_Check(void)
{
	u8 retry=0;
    while (GPIO_INPUT_GET(IO_18B20)&&retry<200)
	{
		retry++;
		os_delay_us(1);
	};
	if(retry>=200)return 1;
	else retry=0;
    while (!GPIO_INPUT_GET(IO_18B20)&&retry<240)
	{
		retry++;
		os_delay_us(1);
	};
	if(retry>=240)return 1;
	return 0;
}


//��DS18B20��ȡһ��λ
//����ֵ��1/0
u8 DS18B20_Read_Bit(void)
{
    u8 data;
	GPIO_OUTPUT_SET(IO_18B20,0);
	os_delay_us(2);
	GPIO_OUTPUT_SET(IO_18B20,1);
	os_delay_us(12);
	if(GPIO_INPUT_GET(IO_18B20))data=1;
    else data=0;
    os_delay_us(50);
    return data;
}
//��DS18B20��ȡһ���ֽ�
//����ֵ������������
u8 DS18B20_Read_Byte(void)
{
    u8 i,j,dat;
    dat=0;
	for (i=1;i<=8;i++)
	{
        j=DS18B20_Read_Bit();
        dat=(j<<7)|(dat>>1);
    }
    return dat;
}
//GPIO_OUTPUT_SET
//GPIO_INPUT_GET
//дһ���ֽڵ�DS18B20
//dat��Ҫд����ֽ�
void DS18B20_Write_Byte(u8 dat)
 {
    u8 j;
    u8 testb;
	//DS18B20_IO_OUT();//SET PG11 OUTPUT;
    for (j=1;j<=8;j++)
	{
        testb=dat&0x01;
        dat=dat>>1;
        if (testb)
        {
            //DS18B20_DQ_OUT=0;// Write 1
            GPIO_OUTPUT_SET(IO_18B20,0);
            os_delay_us(2);
            GPIO_OUTPUT_SET(IO_18B20,1);
            //DS18B20_DQ_OUT=1;
            os_delay_us(60);
        }
        else
        {
        	GPIO_OUTPUT_SET(IO_18B20,0);
            //DS18B20_DQ_OUT=0;// Write 0
            os_delay_us(60);
            GPIO_OUTPUT_SET(IO_18B20,1);
            //DS18B20_DQ_OUT=1;
            os_delay_us(2);
        }
    }
}
//��ʼ�¶�ת��
void DS18B20_Start(void)
{
    DS18B20_Rst();
	DS18B20_Check();
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
}
//��ʼ��DS18B20��IO�� DQ ͬʱ���DS�Ĵ���
//����1:������
//����0:����
u8 DS18B20_Init(void)
{
 	DS18B20_Rst();
	return DS18B20_Check();
}
//��ds18b20�õ��¶�ֵ
//���ȣ�0.1C
//����ֵ���¶�ֵ ��-550~1250��
short DS18B20_Get_Temp(void)
{
    u8 temp;
    u8 TL,TH;
	short tem;
    DS18B20_Start();// ds1820 start convert
    DS18B20_Rst();
    DS18B20_Check();
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// convert
    TL=DS18B20_Read_Byte(); // LSB
    TH=DS18B20_Read_Byte(); // MSB
    if(TH>7)
    {
        TH=~TH;
        TL=~TL;
        temp=0;		//�¶�Ϊ��
    }else temp=1;	//�¶�Ϊ��
    tem=TH; 		//��ø߰�λ
    tem<<=8;
    tem+=TL;		//��õװ�λ
    tem=(double)tem*0.625;	//ת��
	if(temp)return tem;	 	//�����¶�ֵ
	else return -tem;
}



uint8 ICACHE_FLASH_ATTR temp_data(uint8 gpio_num,short *temp_18b20){

	if(DS18B20_Init()==0){
		IO_18B20=gpio_num;
		*temp_18b20=DS18B20_Get_Temp();
		//printf("wendu :------%d\n",DS18B20_Get_Temp());
	return 0;
	}
	return 1;
}



