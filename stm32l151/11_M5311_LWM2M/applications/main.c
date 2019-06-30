/*
 * File      : main.c
 * This file is part of fun in evb_iot_m1 board
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-04-14     longmain     first version
 * 2019-06-19     longmain     add onenet lwm2m demo
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

//����ulog ����
#define LOG_TAG              "[main]"
//�����ڼ俪��
#define LOG_LVL              LOG_LVL_DBG

//��ʽ�ڼ俪������ر��Ϸ��ĵ���ѡ��
//#define LOG_LVL              LOG_LVL_INFO
#include <ulog.h>

//����EVENT �¼�
#define EVENT_SEND_ERROR		(11<<1)
#define EVENT_UPDATE_ERROR	(12<<1)
#define EVENT_ONENET_CLOSE	(13<<1)
#define EVENT_NB_REBOOT			(14<<1)

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

//����onenet stream
qsdk_onenet_stream_t temp_object;
qsdk_onenet_stream_t hump_object;
qsdk_onenet_stream_t light0_object;
qsdk_onenet_stream_t light1_object;
qsdk_onenet_stream_t light2_object;

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

//����ƽ̨�����ݱ�־
rt_uint16_t update_status;

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
																 1000,
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
		//���ƽ̨�鿴���ݣ���Ҫ�ϱ�
		if(update_status!=0)
		{
			update_status=0;
			if(qsdk_onenet_notify(temp_object,0,(qsdk_onenet_value_t)&temperature,0)!=RT_EOK)
			{
				LOG_E("onenet notify error\r\n");
				rt_event_send(net_error,EVENT_SEND_ERROR);
			}
			rt_thread_delay(100);
			if(qsdk_onenet_notify(hump_object,0,(qsdk_onenet_value_t)&humidity,0)!=RT_EOK)
			{
				LOG_E("onenet notify error\r\n");
				rt_event_send(net_error,EVENT_SEND_ERROR);
			}
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
	rt_uint16_t recon_count=0,recon_onenet_count=0;
	rt_uint32_t event_status=0;
net_recon:	
	//nb-iotģ�����ٳ�ʼ������
	if(qsdk_nb_quick_connect()!=RT_EOK)
	{
		LOG_E("module init failure\r\n");	
		goto net_recon;
	}
	//�ر��Զ�����ģʽ
	qsdk_nb_close_auto_psm();
	
onenet_recon:	
	//onenet stream ��ʼ��
	temp_object=qsdk_onenet_object_init(3303,0,5700,1,"1",1,0,qsdk_onenet_value_float);
	if(temp_object==RT_NULL)
	{
		LOG_E("temp object create failure\r\n");
	}
	hump_object=qsdk_onenet_object_init(3304,0,5700,1,"1",1,0,qsdk_onenet_value_float);
	if(hump_object==RT_NULL)
	{
		LOG_E("hump object create failure\r\n");
	}
	light0_object=qsdk_onenet_object_init(3311,0,5850,3,"111",3,0,qsdk_onenet_value_bool);
	if(light0_object==RT_NULL)
	{
		LOG_E("light object create failure\r\n");
	}
		light1_object=qsdk_onenet_object_init(3311,1,5850,3,"111",3,0,qsdk_onenet_value_bool);
	if(light1_object==RT_NULL)
	{
		LOG_E("light object create failure\r\n");
	}
		light2_object=qsdk_onenet_object_init(3311,2,5850,3,"111",3,0,qsdk_onenet_value_bool);
	if(light2_object==RT_NULL)
	{
		LOG_E("light object create failure\r\n");
	}
	//�������ӵ�onenet
	if(qsdk_onenet_quick_start()!=RT_EOK)
	{
		recon_onenet_count++;
		LOG_E("onenet quick start failure\r\n");
		rt_thread_delay(30000);
		if(qsdk_onenet_open()!=RT_EOK)
		{
			qsdk_onenet_delete_instance();
			goto onenet_recon;
		}
	}
	
	//��������ɹ���beep��ʾһ��
	qsdk_beep_on();
	rt_thread_delay(500);
	qsdk_beep_off();
	if(recon_count==0)
		rt_thread_startup(data_up_thread_id);
	else
	{
		rt_thread_resume(data_up_thread_id);
		recon_count=0;
	}
	while (1)
	{
		//�ȴ���������¼�
		if(rt_event_recv(net_error,EVENT_SEND_ERROR|EVENT_UPDATE_ERROR|EVENT_ONENET_CLOSE,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER,&event_status)==RT_EOK)
		{
			//���ȹ��������ϱ��߳�
			rt_thread_suspend(data_up_thread_id);
			led3.current_state=LED_ON;
			
			//�жϵ�ǰ�Ƿ�Ϊ���ݷ��ʹ�����߸�������ʱ�����
			if(event_status==EVENT_SEND_ERROR||event_status==EVENT_UPDATE_ERROR)
			{
				if(qsdk_nb_wait_connect()!=RT_EOK)
				{
					LOG_E("Failure to communicate with module now\r\n");
					recon_count=1;
					goto net_recon;
				}
				if(qsdk_nb_get_net_connect()!=RT_EOK)
				{
					LOG_E("Now Internet has failed\r\n");
					recon_count=1;
					goto net_recon;
				}
				if(qsdk_onenet_get_connect()!=RT_EOK)
				{
				  LOG_E("Now the onenet link is disconnected\r\n");
					if(qsdk_onenet_open()!=RT_EOK)
					{
						LOG_E("Now onenet reconnection\r\n");
						qsdk_onenet_delete_instance();
						recon_count=1;
						goto onenet_recon;
					}
				}			
			}
			//�жϵ�ǰ�Ƿ�Ϊonenet�쳣�ر�
			else if(event_status==EVENT_ONENET_CLOSE)
			{
				LOG_E("Now the onenet link is disconnected\r\n");
				if(qsdk_onenet_open()!=RT_EOK)
				{
					LOG_E("Now onenet reconnection\r\n");
					qsdk_onenet_delete_instance();
					recon_count=1;
					goto onenet_recon;
				}
			}
			//�жϵ�ǰ�Ƿ�Ϊģ���쳣����
			else if(event_status==EVENT_NB_REBOOT)
			{
				if(qsdk_nb_get_net_connect_status()!=RT_EOK)
				{
					LOG_E("Now the nb-iot is reboot,will goto init\r\n");
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
	int lifetime=0;
	char *buf=rt_calloc(1,200);
	if(buf==RT_NULL)
	{
		LOG_E("net create buf error\r\n");
	}
	while(1)
	{
		//�ϱ�����ǰ�����жϵ�ǰlwm2m Э���Ƿ���������
		if(qsdk_onenet_get_connect()==RT_EOK)
		{
			LOG_D("data UP is open\r\n");
retry:			
			if(qsdk_onenet_notify(temp_object,0,(qsdk_onenet_value_t)&temperature,0)!=RT_EOK)
			{
				LOG_E("onenet notify error\r\n");
				rt_event_send(net_error,EVENT_SEND_ERROR);
				goto retry;
			}
			rt_thread_delay(100);
			if(qsdk_onenet_notify(hump_object,0,(qsdk_onenet_value_t)&humidity,0)!=RT_EOK)
			{
				LOG_E("onenet notify error\r\n");
				rt_event_send(net_error,EVENT_SEND_ERROR);
				goto retry;
			}
			
			//ÿ40���Ӷ�ʱ����һ��(�豸����ʱ��) lwm2m lifetime
			if(lifetime>40)
			{
retry_update:
				if(qsdk_onenet_update_time(0)!=RT_EOK)
				{
					LOG_E("onenet lifetime update error");
					rt_thread_delay(5000);
					if(qsdk_onenet_update_time(0)!=RT_EOK)
					{
						LOG_E("Two Update Errors in OneNet Lifetime");
						rt_event_send(net_error,EVENT_UPDATE_ERROR);
						goto retry_update;
					}
				}
				lifetime=0;
			}
		}
		else LOG_E("Now lwm2m is no connect\r\n");
		rt_memset(buf,0,sizeof(buf));
		lifetime++;
		rt_thread_delay(60000);
	}
}


