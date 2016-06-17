#ifndef __INIT_H__
#define __INIT_H__

#include "IAP15W4K61S4.h"

#define FOSC 22118400L          //时钟频率
#define BAUD 115200             //波特率
#define S3FOSC 22118400L          //系统频率
#define S3BAUD 9600             //串口波特率

#define System_SetParaAddr 0x7FE000 /*8MB FLASH倒数第二个扇区*/

#define  STB  	 P33 							//P33端口接U2的P32  INT1（P32——INT0）
#define  INT0		 P32 							//P32端口接U2的P44
#define	 CHACK	 P12							//CHK/ACK
#define	 CHKREQ	 P13							//CHKREQ
#define	 REQ		 P14							//REQ
#define  BUSYSTE P15							//在对方没有处理完一次会话前此位始终为1 不给对方发送任何数据


#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7

/****************************定义开关闸管脚定义******************************/
sbit Door1_Ctl_0 			= P4^4;
sbit Door1_Ctl_1 			= P4^3;
sbit Reset_U980       = P3^7;
// //设定初始时间				 //秒0 分30  时8 星期日 日01 月03 年15 
// unsigned char code Init[7] = {0x00,0x30,0x08,0x07,0x01,0x03,0x15};


void IO_Init(void);
void INT_Init(void);
void Timer0Init(void);
void init_Uart(void);
void System_SetParaInit(void);
void Delay_ms(unsigned int ms);











#endif