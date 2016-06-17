#ifndef __DS1302_H__
#define __DS1302_H__

#include "IAP15W4K61S4.h"

typedef unsigned char uint8;
typedef unsigned int  uint16;  
sbit SDA=P3^5; 	//数据	
sbit SCK=P3^4; 	//时钟
sbit RST=P5^5;	//DS1302复位(片选)


#define DS1302_W_ADDR 0x80
#define DS1302_R_ADDR 0x81

void ds1302_read_time(uint8 *timedata);
void ds1302_set_time(uint8 *timedata);
uint8 ds1302_read(uint8 addr);
void ds1302_write(uint8 addr,uint8 dat);
void ds1302_set_WP();
void ds1302_clear_WP();
uint8 ds1302_read_byte();
void ds1302_write_byte(uint8 dat);
void ds1302_reset();







#endif