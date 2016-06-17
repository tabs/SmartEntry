#ifndef __INIT_H__
#define __INIT_H__

#include "IAP15W4K61S4.h"

#define FOSC 22118400L          //ʱ��Ƶ��
#define BAUD 115200             //������
#define S3FOSC 22118400L          //ϵͳƵ��
#define S3BAUD 9600             //���ڲ�����

#define System_SetParaAddr 0x7FE000 /*8MB FLASH�����ڶ�������*/

#define  STB  	 P33 							//P33�˿ڽ�U2��P32  INT1��P32����INT0��
#define  INT0		 P32 							//P32�˿ڽ�U2��P44
#define	 CHACK	 P12							//CHK/ACK
#define	 CHKREQ	 P13							//CHKREQ
#define	 REQ		 P14							//REQ
#define  BUSYSTE P15							//�ڶԷ�û�д�����һ�λỰǰ��λʼ��Ϊ1 �����Է������κ�����


#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7

/****************************���忪��բ�ܽŶ���******************************/
sbit Door1_Ctl_0 			= P4^4;
sbit Door1_Ctl_1 			= P4^3;
sbit Reset_U980       = P3^7;
// //�趨��ʼʱ��				 //��0 ��30  ʱ8 ������ ��01 ��03 ��15 
// unsigned char code Init[7] = {0x00,0x30,0x08,0x07,0x01,0x03,0x15};


void IO_Init(void);
void INT_Init(void);
void Timer0Init(void);
void init_Uart(void);
void System_SetParaInit(void);
void Delay_ms(unsigned int ms);











#endif