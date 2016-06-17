#ifndef __INIT_H__
#define __INIT_H__




#include "IAP15W4K61S4.h"

#define FOSC 22118400L          //时钟频率
#define BAUD 115200             //波特率


#define  STB  	P44 							//P44端口接U1的P32(INT0）
#define  INT0		P32 							//P32端口接U2的P44
#define	 CHACK	P12								//CHK/ACK
#define	 CHKREQ	P13								//CHKREQ
#define	 REQ		P14								//REQ
#define  BUSYSTE P15							//在对方没有处理完一次会话前此位始终为1 不给对方发送任何数据

#define S4_S0 0x04              //P_SW2.2
#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7


#define System_SetParaAddr 0x7FE000 /*8MB FLASH倒数第二个扇区*/


/****************************定义开关闸管脚定义******************************/
sbit Door1_Ctl_0 			= P4^4;
sbit Door1_Ctl_1 			= P4^3;



void IO_Init(void);
void INT_Init(void);
void Timer0Init(void);
void init_Uart(void);
void System_SetParaInit(void);











#endif