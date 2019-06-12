//Ӳ������
#include "qsdk_oled.h"
#include "board.h"
//�ֿ�
#include "qsdk_oled_zk.h"

//C��
#include <stdarg.h>
#include <stdio.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME "OLED"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>


static struct rt_i2c_bus_device *i2c_bus;
static rt_mutex_t i2c_lock;

#define OLED_ADDRESS		0x3C
#define OLED_WRITE_CMD 	0x00
#define OLED_WRITE_DATE 0x40

/*
************************************************************
*	�������ƣ�	qsdk_oled_write_data
*
*	�������ܣ�	OLEDд��һ������
*
*	��ڲ�����	byte����Ҫд�������
*
*	���ز�����	д����
*
*	˵����		0-�ɹ�		1-ʧ��
************************************************************
*/
static _Bool qsdk_oled_write_data(unsigned char byte)
{
	rt_uint8_t buf[2];
	
	buf[0]=OLED_WRITE_DATE;
	buf[1]=byte;
	
	if(rt_i2c_master_send(i2c_bus,OLED_ADDRESS,0,buf,2)==2)
	{
		return RT_EOK;
	}
	else
	{
		LOG_E("oled write data error,plese check i2c gpio init\r\n");
		return RT_ERROR;
	}
}

/*
************************************************************
*	�������ƣ�	qsdk_oled_write_cmd
*
*	�������ܣ�	OLEDд��һ������
*
*	��ڲ�����	cmd����Ҫд�������
*
*	���ز�����	д����
*
*	˵����		0-�ɹ�		1-ʧ��
************************************************************
*/
static _Bool qsdk_oled_write_cmd(unsigned char cmd)
{
	rt_uint8_t buf[2];
	
	buf[0]=OLED_WRITE_CMD;
	buf[1]=cmd;
	
	if(rt_i2c_master_send(i2c_bus,OLED_ADDRESS,0,buf,2)==2)
	{
		return RT_EOK;
	}
	else
	{
		LOG_E("oled write cmd error,plese check i2c gpio init\r\n");
		return RT_ERROR;
	}
}

/*
************************************************************
*	�������ƣ�	qsdk_oled_init
*
*	�������ܣ�	OLED��ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void qsdk_oled_init(char *i2c_bus_name)
{
	i2c_bus=(struct rt_i2c_bus_device*)rt_device_find(i2c_bus_name);
	if(i2c_bus==RT_NULL)
	{
		LOG_E("no find i2c device:%s\r\n",i2c_bus_name);
	}
	i2c_lock=rt_mutex_create("i2c_lock",RT_IPC_FLAG_FIFO);
	if(i2c_lock==RT_NULL)
	{
		LOG_E("i2c create mutex fail\r\n");
	}
	rt_thread_delay(100);
#if 1
	qsdk_oled_write_cmd(0xAE); //�ر���ʾ
	qsdk_oled_write_cmd(0x20); //Set Memory Addressing Mode	
	qsdk_oled_write_cmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	qsdk_oled_write_cmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
	qsdk_oled_write_cmd(0xa1); //0xa0��X��������ʾ��0xa1��X�᾵����ʾ
	qsdk_oled_write_cmd(0xc8); //0xc0��Y��������ʾ��0xc8��Y�᾵����ʾ
	qsdk_oled_write_cmd(0x00); //�����е�ַ��4λ
	qsdk_oled_write_cmd(0x10); //�����е�ַ��4λ
	qsdk_oled_write_cmd(0x40); //������ʼ�ߵ�ַ
	qsdk_oled_write_cmd(0x81); //���öԱȶ�ֵ
	qsdk_oled_write_cmd(0x7f); //------
	qsdk_oled_write_cmd(0xa6); //0xa6,������ʾģʽ;0xa7��
	qsdk_oled_write_cmd(0xa8); //--set multiplex ratio(1 to 64)
	qsdk_oled_write_cmd(0x3F); //------
	qsdk_oled_write_cmd(0xa4); //0xa4,��ʾ����RAM�ĸı���ı�;0xa5,��ʾ���ݺ���RAM������
	qsdk_oled_write_cmd(0xd3); //������ʾƫ��
	qsdk_oled_write_cmd(0x00); //------
	qsdk_oled_write_cmd(0xd5); //�����ڲ���ʾʱ��Ƶ��
	qsdk_oled_write_cmd(0xf0); //------
	qsdk_oled_write_cmd(0xd9); //--set pre-charge period//
	qsdk_oled_write_cmd(0x22); //------
	qsdk_oled_write_cmd(0xda); //--set com pins hardware configuration//
	qsdk_oled_write_cmd(0x12); //------
	qsdk_oled_write_cmd(0xdb); //--set vcomh//
	qsdk_oled_write_cmd(0x20); //------
	qsdk_oled_write_cmd(0x8d); //--set DC-DC enable//
	qsdk_oled_write_cmd(0x14); //------
	qsdk_oled_write_cmd(0xaf); //����ʾ
#else
	qsdk_oled_write_cmd(0xAE);   //display off
	qsdk_oled_write_cmd(0x00);	//Set Memory Addressing Mode	
	qsdk_oled_write_cmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	qsdk_oled_write_cmd(0x40);	//Set Page Start Address for Page Addressing Mode,0-7
	qsdk_oled_write_cmd(0xb0);	//Set COM Output Scan Direction
	qsdk_oled_write_cmd(0x81);//---set low column address
	qsdk_oled_write_cmd(0xff);//---set high column address
	qsdk_oled_write_cmd(0xa1);//--set start line address
	qsdk_oled_write_cmd(0xa6);//--set contrast control register
	qsdk_oled_write_cmd(0xa8);
	qsdk_oled_write_cmd(0x3f);//--set segment re-map 0 to 127
	qsdk_oled_write_cmd(0xad);//--set normal display
	qsdk_oled_write_cmd(0x8b);//--set multiplex ratio(1 to 64)
	qsdk_oled_write_cmd(0x33);//
	qsdk_oled_write_cmd(0xc8);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	qsdk_oled_write_cmd(0xd3);//-set display offset
	qsdk_oled_write_cmd(0x00);//-not offset
	qsdk_oled_write_cmd(0xd5);//--set display clock divide ratio/oscillator frequency
	qsdk_oled_write_cmd(0x80);//--set divide ratio
	qsdk_oled_write_cmd(0xd9);//--set pre-charge period
	qsdk_oled_write_cmd(0x1f); //
	qsdk_oled_write_cmd(0xda);//--set com pins hardware configuration
	qsdk_oled_write_cmd(0x12);
	qsdk_oled_write_cmd(0xdb);//--set vcomh
	qsdk_oled_write_cmd(0x40);//0x20,0.77xVcc
//	qsdk_oled_write_cmd(0x8d);//--set DC-DC enable
//	qsdk_oled_write_cmd(0x14);//
	qsdk_oled_write_cmd(0xaf);//--turn on oled panel
#endif

}

/*
************************************************************
*	�������ƣ�	qsdk_oled_set_address
*
*	�������ܣ�	����OLED��ʾ��ַ
*
*	��ڲ�����	x���е�ַ
*				y���е�ַ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void qsdk_oled_set_address(unsigned char x, unsigned char y)
{

	qsdk_oled_write_cmd(0xb0 + x);					//�����е�ַ
	//HAL_Delay_us(5);
	qsdk_oled_write_cmd(((y & 0xf0) >> 4) | 0x10);	//�����е�ַ�ĸ�4λ
	//HAL_Delay_us(5);
	qsdk_oled_write_cmd(y & 0x0f);					//�����е�ַ�ĵ�4λ
	//HAL_Delay_us(5);
	
}

/*
************************************************************
*	�������ƣ�	qsdk_oled_clear_screen
*
*	�������ܣ�	OLEDȫ�����
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void qsdk_oled_clear_screen(void)
{
	
	unsigned char i = 0, j = 0;
	
	rt_enter_critical();
	for(; i < 8; i++)
	{
		qsdk_oled_write_cmd(0xb0 + i);
		qsdk_oled_write_cmd(0x10);
		qsdk_oled_write_cmd(0x00);
			
		for(j = 0; j < 128; j++)
		{
			qsdk_oled_write_data(0x00);
		}
	}
	rt_exit_critical();
	
}

/*
************************************************************
*	�������ƣ�	qsdk_oled_clear_line
*
*	�������ܣ�	OLED���ָ����
*
*	��ڲ�����	x����Ҫ�������
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void qsdk_oled_clear_line(unsigned char x)
{

	unsigned char i = 0;
		
	qsdk_oled_write_cmd(0xb0 + x);
	qsdk_oled_write_cmd(0x10);
	qsdk_oled_write_cmd(0x00);
			
	for(; i < 128; i++)
	{
		qsdk_oled_write_data(0x00);
	}

}

/*
************************************************************
*	�������ƣ�	qsdk_oled_dis_128x64_picture
*
*	�������ܣ�	��ʾһ��128*64��ͼƬ
*
*	��ڲ�����	dp��ͼƬ����ָ��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void qsdk_oled_dis_128x64_picture(const unsigned char *dp)
{
	
	unsigned char i = 0, j = 0;
	
		
	for(; j < 8; j++)
	{
		qsdk_oled_set_address(j, 0);
		
		for (i = 0; i < 128; i++)
		{	
			qsdk_oled_write_data(*dp++); //д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1
		}
	}
	
}

/*
************************************************************
*	�������ƣ�	qsdk_oled_dis_16x16_char
*
*	�������ܣ�	��ʾ16x16�ĵ�������
*
*	��ڲ�����	dp��ͼƬ����ָ��
*
*	���ز�����	��
*
*	˵����		��ʾ16x16����ͼ�񡢺��֡���Ƨ�ֻ�16x16���������ͼ��
************************************************************
*/
void qsdk_oled_dis_16x16_char(unsigned short x, unsigned short y, const unsigned char *dp)
{
	
	unsigned short i = 0, j = 0;
		
	for(j = 2; j > 0; j--)
	{
		qsdk_oled_set_address(x, y);
		
		for (i = 0; i < 16; i++)
		{
			qsdk_oled_write_data(*dp++);		//д���ݵ�OLED,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1
		}
		
		x++;
	}
	
}

/*
************************************************************
*	�������ƣ�	qsdk_oled_dis_6x8_string
*
*	�������ܣ�	��ʾ6x8�ĵ�������
*
*	��ڲ�����	x����ʾ��
*				y����ʾ��
*				fmt����������
*
*	���ز�����	��
*
*	˵����		����ʾ7��
************************************************************
*/
void qsdk_oled_dis_6x8_string(unsigned char x, unsigned char y, char *fmt, ...)
{

	unsigned char i = 0, ch = 0;
	unsigned char OledPrintfBuf[300];
	
	va_list ap;
	unsigned char *pStr = OledPrintfBuf;
	
	va_start(ap,fmt);
	vsprintf((char *)OledPrintfBuf, fmt, ap);
	va_end(ap);
	
	y += 2;
		
	while(*pStr != '\0')
	{
		ch = *pStr - 32;
		
		if(y > 126)
		{
			y = 2;
			x++;
		}
		
		qsdk_oled_set_address(x, y);
		for(i = 0; i < 6; i++)
			qsdk_oled_write_data(F6x8[ch][i]);
		
		y += 6;
		pStr++;
	}

}

/*
************************************************************
*	�������ƣ�	qsdk_oled_dis_8x16_string
*
*	�������ܣ�	��ʾ8x16�ĵ�������
*
*	��ڲ�����	x����ʾ��
*				y����ʾ��
*				fmt����������
*
*	���ز�����	��
*
*	˵����		����ʾ4��
************************************************************
*/
void qsdk_oled_dis_8x16_string(unsigned char x, unsigned char y, char *fmt, ...)
{

	unsigned char i = 0, ch = 0;
	unsigned char OledPrintfBuf[300];
	
	va_list ap;
	unsigned char *pStr = OledPrintfBuf;
	
	va_start(ap,fmt);
	vsprintf((char *)OledPrintfBuf, fmt, ap);
	va_end(ap);
	
	y += 2;
		
	while(*pStr != '\0')
	{
		ch = *pStr - 32;
			
		if(y > 128)
		{
			y = 2;
			x += 2;
		}
			
		qsdk_oled_set_address(x, y);
		for(i = 0; i < 8; i++)
			qsdk_oled_write_data(F8X16[(ch << 4) + i]);
		
		qsdk_oled_set_address(x + 1, y);
		for(i = 0; i < 8; i++)
			qsdk_oled_write_data(F8X16[(ch << 4) + i + 8]);
		
		y += 8;
		pStr++;
	}

}
