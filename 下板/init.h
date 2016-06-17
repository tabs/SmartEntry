#ifndef __INIT_H__
#define __INIT_H__




#include "IAP15W4K61S4.h"

#define FOSC 22118400L          //ʱ��Ƶ��
#define BAUD 115200             //������


#define  STB  	P44 							//P44�˿ڽ�U1��P32(INT0��
#define  INT0		P32 							//P32�˿ڽ�U2��P44
#define	 CHACK	P12								//CHK/ACK
#define	 CHKREQ	P13								//CHKREQ
#define	 REQ		P14								//REQ
#define  BUSYSTE P15							//�ڶԷ�û�д�����һ�λỰǰ��λʼ��Ϊ1 �����Է������κ�����

#define S4_S0 0x04              //P_SW2.2
#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7


#define System_SetParaAddr 0x7FE000 /*8MB FLASH�����ڶ�������*/


/****************************���忪��բ�ܽŶ���******************************/
sbit Door1_Ctl_0 			= P4^4;
sbit Door1_Ctl_1 			= P4^3;



void IO_Init(void);
void INT_Init(void);
void Timer0Init(void);
void init_Uart(void);
void System_SetParaInit(void);











#endif