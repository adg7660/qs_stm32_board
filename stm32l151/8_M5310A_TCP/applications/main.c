/*
 * File      : main.c
 * This file is part of fun in evb_iot_m1 board
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-04-14     longmain     first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "qsdk_beep.h"
#include "qsdk_led.h"
#include "qsdk_key.h"
#include "qsdk_oled.h"
#include "sht20.h"
#include "qsdk.h"

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME              "[QSDK/CALLBACK]"

#define DBG_LEVEL                      DBG_LOG
//#define DBG_LEVEL                      DBG_INFO

#include <rtdbg.h>

//����EVENT �¼�
#define EVENT_SEND_ERROR		(11<<1)
#define EVENT_NET_CLOSE	(12<<1)
#define EVENT_REBOOT				(13<<1)

//������������¼����
rt_event_t net_error=RT_NULL;

//�����߳̿��ƾ��
rt_thread_t data_up_thread_id=RT_NULL;
static rt_thread_t led_thread_id=RT_NULL;
static rt_thread_t sht20_thread_id=RT_NULL;
static rt_thread_t key_thread_id=RT_NULL;
static rt_thread_t net_thread_id=RT_NULL;

//Ԥ�����߳����к���
void led_entry(void *parameter);
void sht20_entry(void *parameter);
void key_entry(void *parameter);
void net_entry(void *parameter);
void data_up_entry(void *parameter);

//����tcp client
qsdk_net_client_t tcp_client;

//����sht20 ���
sht20_device_t sht20_dev;

//����led ���ƿ�
struct led_state_type led0={0};
struct led_state_type led1={0};
struct led_state_type led2={0};
struct led_state_type led3={0};

//����key ���ƿ�
struct key_state_type key0={0};
struct key_state_type key1={0};

//������ʪ�Ȼ������
float humidity;
float temperature;

int main(void)
{
	LOG_I("��ӭʹ����ʿ���ܿƼ� STM32������\r\n");
	qsdk_oled_init("i2c1");
	qsdk_oled_clear_screen();
	qsdk_oled_dis_16x16_char(0, 16, Qi);
	qsdk_oled_dis_16x16_char(0, 32, Shi);
	qsdk_oled_dis_16x16_char(0, 48, Zhi);
	qsdk_oled_dis_16x16_char(0, 64, Neng);
	qsdk_oled_dis_16x16_char(0, 80, Ke);
	qsdk_oled_dis_16x16_char(0, 96, Ji);
	qsdk_oled_dis_8x16_string(2,24,"EVB_IOT_M1");
	
	qsdk_oled_dis_16x16_char(4, 0, wen);
	qsdk_oled_dis_16x16_char(4, 16, du);
	qsdk_oled_dis_8x16_string(4,32,":");

	qsdk_oled_dis_16x16_char(6, 0, shi);
	qsdk_oled_dis_16x16_char(6, 16, du);
	qsdk_oled_dis_8x16_string(6,32,":");
	
	//�����¼� 
	net_error=rt_event_create("net_error",RT_IPC_FLAG_FIFO);
	if(net_error==RT_NULL)
	{
		LOG_E("create net error event failure\r\n");
	}
	
	//����led �߳�
	led_thread_id=rt_thread_create("led_th",
																 led_entry,
																 RT_NULL,
																 500,
																 15,
																 20);
		if(led_thread_id!=RT_NULL)
			rt_thread_startup(led_thread_id);
		else
			return -1;
		
	//����sht20 �߳�
	sht20_thread_id=rt_thread_create("sht20_th",
																 sht20_entry,
																 RT_NULL,
																 1000,
																 19,
																 20);
		if(sht20_thread_id!=RT_NULL)
			rt_thread_startup(sht20_thread_id);
		else
			return -1;
	
	//����key �߳�
	key_thread_id=rt_thread_create("key_th",
																 key_entry,
																 RT_NULL,
																 500,
																 5,
																 20);
		if(key_thread_id!=RT_NULL)
			rt_thread_startup(key_thread_id);
		else
			return -1;
		
	//����net �߳�
	net_thread_id=rt_thread_create("net_th",
																 net_entry,
																 RT_NULL,
																 1500,
																 10,
																 20);
		if(net_thread_id!=RT_NULL)
			rt_thread_startup(net_thread_id);
		else
			return -1;

	//����data up �߳�
	data_up_thread_id=rt_thread_create("data_th",
																 data_up_entry,
																 RT_NULL,
																 1000,
																 12,
																 50);
		if(data_up_thread_id==RT_NULL)
			return -1;
}

void led_entry(void *parameter)
{
	while(1)
	{
	//����led0 ��ǰ״̬����
		if(led0.last_state!=led0.current_state)
		{
			if(led0.current_state==LED_ON)
			{
				qsdk_led_on(LED0);
			}
			else
			{
				qsdk_led_off(LED0);
			}
			led0.last_state=led0.current_state;
		}
		//����led1 ��ǰ״̬����
		if(led1.last_state!=led1.current_state)
		{
			if(led1.current_state==LED_ON)
			{
				qsdk_led_on(LED1);
			}
			else
			{
				qsdk_led_off(LED1);
			}
			led1.last_state=led1.current_state;
		}
		//����led2 ��ǰ״̬����
		if(led2.last_state!=led2.current_state)
		{
			if(led2.current_state==LED_ON)
			{
				qsdk_led_on(LED2);
			}
			else
			{
				qsdk_led_off(LED2);
			}
			led2.last_state=led2.current_state;
		}
		//����led3 ��ǰ״̬����
		if(led3.last_state!=led3.current_state)
		{
			if(led3.current_state==LED_ON)
			{
				qsdk_led_on(LED3);
			}
			else
			{
				qsdk_led_off(LED3);
			}
			led3.last_state=led3.current_state;
		}
		rt_thread_delay(100);
	}
}


void sht20_entry(void *parameter)
{
	sht20_dev=sht20_init("i2c1");
	if(sht20_dev==RT_NULL)
		LOG_E("no find sht20 device\r\n");
	while(1)
	{
		temperature=sht20_read_temperature(sht20_dev);
		humidity=sht20_read_humidity(sht20_dev);
		qsdk_oled_dis_8x16_string(4,40,"%0.1f",temperature);
		qsdk_oled_dis_8x16_string(6,40,"%0.1f",humidity);
		rt_thread_delay(500);
	}
}

void key_entry(void *parameter)
{
	while(1)
	{
		//��ʱ��ⰴ���Ƿ��а���
		qsdk_key_process();
		
		//����������
		if(key0.down_state==KEY_PRESS)
		{
			LOG_D("key0 dowm press \r\n");
			led0.current_state=LED_ON;
			key0.down_state=KEY_RELEASE;
		}
		else if(key0.double_state==KEY_PRESS)
		{
			LOG_D("key0 double press \r\n");
			led1.current_state=LED_ON;
			key0.double_state=KEY_RELEASE;
		}
		else if(key0.long_state==KEY_PRESS)
		{
			LOG_D("key0 long press \r\n");
			led2.current_state=LED_ON;
			key0.long_state=KEY_RELEASE;
		}
		
		else if(key1.down_state==KEY_PRESS)
		{
			LOG_D("key1 down press \r\n");
			led3.current_state=LED_ON;
			key1.down_state=KEY_RELEASE;
		}
		else if(key1.double_state==KEY_PRESS)
		{
			LOG_D("key1 double press \r\n");
			led0.current_state=LED_OFF;
			led1.current_state=LED_OFF;
			led2.current_state=LED_OFF;
			led3.current_state=LED_OFF;
			key1.double_state=KEY_RELEASE;
		}			
		else if(key1.long_state==KEY_PRESS)
		{
			LOG_D("key1 long press \r\n");
			key1.long_state=KEY_RELEASE;
		}	
		
		rt_thread_delay(20);	
	}
}


void net_entry(void *parameter)
{
	rt_uint16_t recon_count=0;
	rt_uint32_t event_status=0;
net_recon:	
	//nb-iotģ�����ٳ�ʼ������
	if(qsdk_nb_quick_connect()!=RT_EOK)
	{
		LOG_E("module init failure\r\n");	
		goto net_recon;
	}

tcp_recon:	
	//tcp client ��ʼ��
	tcp_client=qsdk_net_client_init(QSDK_NET_TYPE_TCP,1003,"114.116.144.151",2039);
	
	//tcp client ���� socket
	if(qsdk_net_create_socket(tcp_client)!=RT_EOK)
		LOG_E("create socket fail\r\n");
	
	//tcp ���ӵ�������
	if(qsdk_net_connect_to_server(tcp_client)!=RT_EOK)
		LOG_E("connect to server fail\r\n");
	
	//��������ɹ���beep��ʾһ��
	qsdk_beep_on();
	rt_thread_delay(500);
	qsdk_beep_off();
	if(recon_count==0)
		rt_thread_startup(data_up_thread_id);
	else
	{
		recon_count=0;
		led3.current_state=LED_OFF;
		rt_thread_resume(data_up_thread_id);
	}
	while (1)
	{
		//�ȴ���������¼�
		if(rt_event_recv(net_error,EVENT_SEND_ERROR|EVENT_NET_CLOSE|EVENT_REBOOT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER,&event_status)==RT_EOK)
		{
			//���ȹ��������ϱ��߳�
			rt_thread_suspend(data_up_thread_id);
			led3.current_state=LED_ON;
			
			//�жϵ�ǰ�Ƿ�Ϊ���ݷ��ʹ���
			if(event_status==EVENT_SEND_ERROR)
			{
				LOG_E("Now the tcp send error\r\n");
				if(qsdk_nb_wait_connect()!=RT_EOK)
				{
					LOG_E("Failure to communicate with module now\r\n");
					recon_count=1;
					qsdk_net_clear_environment();
					goto net_recon;
				}
				if(qsdk_nb_ping_ip("183.232.231.172")!=RT_EOK)
				{
					LOG_D("PING ERROR\r\n");
				}
				if(qsdk_nb_get_net_connect()!=RT_EOK)
				{
					LOG_E("Now Internet has failed\r\n");
					recon_count=1;
					qsdk_net_clear_environment();
					goto net_recon;
				}
				LOG_E("Now tcp will reconnection\r\n");
				qsdk_net_close_socket(tcp_client);
				rt_thread_delay(1500);
				recon_count=1;
				goto tcp_recon;
		
			}
			//�жϵ�ǰ�Ƿ�Ϊ�����쳣�ر�
			else if(event_status==EVENT_NET_CLOSE)
			{
				LOG_E("Now the tcp is disconnected\r\n");
				rt_thread_delay(1500);
				if(qsdk_net_create_socket(tcp_client)!=RT_EOK)
				{
					LOG_E("TCP create socket error\r\n");
				}
				if(qsdk_net_connect_to_server(tcp_client)!=RT_EOK)
				{
					LOG_E("Now tcp will reconnection\r\n");
					qsdk_net_close_socket(tcp_client);
					recon_count=1;
					goto tcp_recon;
				}
			}
			else if(event_status==EVENT_REBOOT)
			{
				if(qsdk_net_get_client_connect(tcp_client)!=RT_EOK)
				{
					LOG_E("Now nb-iot module is rebooted abnormally and needs to be init.\r\n");
					recon_count=1;
					goto net_recon;
				}
			}
			led3.current_state=LED_OFF;
			rt_thread_resume(data_up_thread_id);	
		}
		
	}
}

void data_up_entry(void *parameter)
{
	char *buf=rt_calloc(1,200);
	if(buf==RT_NULL)
	{
		LOG_E("net create buf error\r\n");
	}
	while(1)
	{
		//�жϵ�ǰ�����Ƿ�����ʱ�����ӳɹ������ӳɹ����ϱ���������
		if(qsdk_nb_get_net_connect_status()==RT_EOK)
		{
			sprintf(buf,"Now the board temperature is:%0.2f,humidity is:%0.2f,led0 state:%d,led1 state:%d,led2 state:%d,led3 state:%d",temperature,humidity,led0.current_state,led1.current_state,led2.current_state,led3.current_state);
			
			//�жϵ�ǰudp�Ƿ�������״̬  
			if(qsdk_net_get_client_connect(tcp_client)==RT_EOK)
			{
				LOG_D("data UP is open\r\n");	
data_repeat:
				if(qsdk_net_send_data(tcp_client,buf)==RT_EOK)
						LOG_D("tcp send data success\r\n");
				else	
				{
					rt_event_send(net_error,EVENT_SEND_ERROR);
					goto data_repeat;
				}
			}
			else 
			{
				rt_event_send(net_error,EVENT_NET_CLOSE);
			}
			rt_memset(buf,0,sizeof(buf));
		}
		rt_thread_delay(60000);
	}
}


