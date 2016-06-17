#include "IAP15W4K61S4.h"
#include <intrins.H>
#include <string.h>
//#define RESETSYSTEM()    ((void (code *)())0xF3F0)() //��λ���
#define USERAP()    		 ((void (code *)())0x1000)() //�û�������ʼ��ַ


#define USER_APP_ADDR 0x1000
	//����һ������ָ�룬������ת���û�����


typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef ULONG FLADDR;

#define FOSC 22118400L          //ʱ��Ƶ��
#define BAUD 115200             //������

#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7



/*����Flash �����ȴ�ʱ�估����IAP/ISP/EEPROM �����ĳ���*/
/*#define ENABLE_ISP 0x80*/ //ϵͳ����ʱ��<30MHz ʱ����IAP_CONTR �Ĵ������ô�ֵ
#define ENABLE_ISP 0x81 //ϵͳ����ʱ��<24MHz ʱ����IAP_CONTR �Ĵ������ô�ֵ
/*#define ENABLE_ISP 0x82*/ //ϵͳ����ʱ��<20MHz ʱ����IAP_CONTR �Ĵ������ô�ֵ
/*#define ENABLE_ISP 0x83*/ //ϵͳ����ʱ��<12MHz ʱ����IAP_CONTR �Ĵ������ô�ֵ
/*#define ENABLE_ISP 0x84*/ //ϵͳ����ʱ��<6MHz ʱ����IAP_CONTR �Ĵ������ô�ֵ
/*#define ENABLE_ISP 0x85*/ //ϵͳ����ʱ��<3MHz ʱ����IAP_CONTR �Ĵ������ô�ֵ
/*#define ENABLE_ISP 0x86*/ //ϵͳ����ʱ��<2MHz ʱ����IAP_CONTR �Ĵ������ô�ֵ
/*#define ENABLE_ISP 0x87*/ //ϵͳ����ʱ��<1MHz ʱ����IAP_CONTR �Ĵ������ô�ֵ
/****************************************************************************/

//const unsigned char code welcome[]="welcome to SmartEntery System BootLoader!";

#define SysCordAppUpdataFlag 0x7E00
#define UpdataFlag0 0x5A/*�Ƿ������жϱ�־ 0x5a 0xa5*/
#define UpdataFlag1 0xA5
#define SysCordStartAddr 0x1000 /*����Ӧ��������ʼ��ַ*/
#define SysCordEndAddr 0x7200 /*����Ӧ�����Ľ�����ַ*/
#define SysCordAppSaveStartTempAddr 0x8000 /*����Ӧ�����ݴ����ʼ��ַ*/



/*�ڲ�FLASH����������*/
void IAP_Disable();/*�ر�IAP ����*/
void FLASH_ByteWrite (FLADDR addr, char byte);
unsigned char FLASH_ByteRead (FLADDR addr);
void FLASH_PageErase (FLADDR addr);
//UCHAR sequential_write_flash_in_one_sector(UINT begin_addr, UINT counter,UCHAR array[]);
void Btlder_init_Uart1(void);
//void UART_Send(unsigned char *dat,unsigned int len);
void btlder(void);
void Delay_ms(UINT ms);
//void (*boot)() = USER_APP_ADDR;

//?PR?*?UPDATEMOUDLE(0x0F000)

void main(void){
//ULONG i;

	//bootloader�ﲻ�ܿ����ж�
	EA = 0;
	Btlder_init_Uart1();//����1��ʼ��
//	 SBUF = 0x09;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();

//	UART_Send(welcome,sizeof(welcome));
	
//	for(i=0x1000;i<0x1200;i++){
//	 SBUF = FLASH_ByteRead(i);while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//	}
	btlder();
	USERAP();//���û�������
	while(1);
}


void Btlder_init_Uart1(void)
{
ACC = P_SW1;
ACC &= ~(S1_S0 | S1_S1);    //S1_S0=0 S1_S1=0
P_SW1 = ACC;                //(P3.0/RxD, P3.1/TxD)
SCON = 0x50;								//1010000
T2L = (65536 - (FOSC/4/BAUD));   //���ò�������װֵ
T2H = (65536 - (FOSC/4/BAUD))>>8;
AUXR = 0x14;								//T2Ϊ1Tģʽ, ��������ʱ��2 10100
AUXR |= 0x01;               //ѡ��ʱ��2Ϊ����1�Ĳ����ʷ�����
}



void btlder(void)
 {
unsigned int Addr_j,UpdatePacNum ;
unsigned char UART_DATA_buf = 0,UART_DATA_buf0 = 0,IAP_UpdatePacNum=0;

//	 SBUF = 0xf0;while(!(SCON&0x02));SCON &= ~ 0x02;
 /********************************************************************/
 /*�ϵ��ȡ�����Ƿ��޸ģ��޸�����³��򣬲��޸ģ������ִ��          */
 UART_DATA_buf=FLASH_ByteRead (SysCordAppUpdataFlag);
 UART_DATA_buf0=FLASH_ByteRead ((SysCordAppUpdataFlag+1));
 IAP_UpdatePacNum = FLASH_ByteRead (SysCordAppUpdataFlag+2);
//	 SBUF = UART_DATA_buf;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//	 SBUF = UART_DATA_buf0;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//	 SBUF = IAP_UpdatePacNum;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
 if((UART_DATA_buf==UpdataFlag0)&&(UART_DATA_buf0==UpdataFlag1))/*�����������Դ����*/
 {/*����������*/
//SBUF = 0xf1;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();

   for(Addr_j=SysCordStartAddr;Addr_j<SysCordEndAddr;Addr_j+=0x200)		//���ݰ������������û�������
   { 
		 FLASH_PageErase (Addr_j);	
//		 SBUF = Addr_j>>8;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//		 SBUF = Addr_j;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
	 }
		for(UpdatePacNum = 0;UpdatePacNum<IAP_UpdatePacNum;UpdatePacNum++)
	 {
		 //SBUF = UpdatePacNum;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
				for(Addr_j=0;Addr_j<0x200;Addr_j++)
		 {
			UART_DATA_buf=FLASH_ByteRead(SysCordAppSaveStartTempAddr+Addr_j+UpdatePacNum*0x200);
			FLASH_ByteWrite(SysCordStartAddr+Addr_j+UpdatePacNum*0x200, UART_DATA_buf);
//			SBUF = UART_DATA_buf;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
		 }
//		 	 for(Addr_j = 0;Addr_j<8;Addr_j++){
//	 SBUF = 0x00;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//	 }
			Delay_ms(50);
			FLASH_PageErase (SysCordAppSaveStartTempAddr+UpdatePacNum*0x200);								//������ʱ������					
//		  SBUF = 0xf2;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();_nop_();
	 }


	 
//	 SBUF = 0xf3;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
    FLASH_PageErase(SysCordAppUpdataFlag); 
	 
//	 for(Addr_j = 0;Addr_j<8;Addr_j++){
//	 SBUF = 0x88;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//	 }

	 
		IAP_CONTR = 0x20;//����
   }
 
//	 SBUF = 0xf4;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//   UART_DATA_buf=0;
//   Addr_j=0;
	
	 USERAP();//���û�������
	
 }
 
 
 
void FLASH_ByteWrite (FLADDR addr, char byte) 
{
    IAP_CONTR = ENABLE_ISP;         //�� IAP ����, ����Flash �����ȴ�ʱ��
    IAP_CMD = 0x02;                 //IAP/ISP/EEPROM �ֽڱ������
    IAP_ADDRH = (addr&0xff00)>>8;    //����Ŀ�굥Ԫ��ַ�ĸ�8 λ��ַ
    IAP_ADDRL = (addr&0xff);    //����Ŀ�굥Ԫ��ַ�ĵ�8 λ��ַ

    IAP_DATA = byte;                  //Ҫ��̵��������ͽ�IAP_DATA �Ĵ���
//		EA=0;
    IAP_TRIG = 0x5A;   //���� 5Ah,����A5h ��ISP/IAP �����Ĵ���,ÿ�ζ������
    IAP_TRIG = 0xA5;   //����A5h ��ISP/IAP ����������������
			_nop_();
//		EA=1;	
    IAP_Disable();  //�ر�IAP ����, ����ص����⹦�ܼĴ���,ʹCPU ���ڰ�ȫ״̬,
                    //һ��������IAP �������֮����ر�IAP ����,����Ҫÿ�ζ���
}
/****************************************************************************/
/* FLASH_ByteRead                                                           */ 
/*This routine reads a <byte> from the linear FLASH address <addr>.         */ 
/*                                                                          */ 
/****************************************************************************/
unsigned char FLASH_ByteRead (FLADDR addr)
{
    IAP_DATA = 0x00;
    IAP_CONTR = ENABLE_ISP;         //��IAP ����, ����Flash �����ȴ�ʱ��
    IAP_CMD = 0x01;                 //IAP/ISP/EEPROM �ֽڶ�����

    IAP_ADDRH = (addr&0xff00)>>8;    //����Ŀ�굥Ԫ��ַ�ĸ�8 λ��ַ
    IAP_ADDRL = (addr&0xff);    //����Ŀ�굥Ԫ��ַ�ĵ�8 λ��ַ

//		EA=0;
    IAP_TRIG = 0x5A;   //���� 5Ah,����A5h ��ISP/IAP �����Ĵ���,ÿ�ζ������
    IAP_TRIG = 0xA5;   //����A5h ��ISP/IAP ����������������
		_nop_(); 
//		EA=1;
    IAP_Disable();  //�ر�IAP ����, ����ص����⹦�ܼĴ���,ʹCPU ���ڰ�ȫ״̬,
                    //һ��������IAP �������֮����ر�IAP ����,����Ҫÿ�ζ���
    return (IAP_DATA);
}



/****************************************************************************/
/* FLASH_PageErase                                                          */ 
/*This routine erases the FLASH page containing the linear FLASH addres     */ 
/* <addr>.                                                                  */ 
/****************************************************************************/
void FLASH_PageErase (FLADDR addr)
{
	
    IAP_CONTR = ENABLE_ISP;         //��IAP ����, ����Flash �����ȴ�ʱ��
    IAP_CMD = 0x03;                 //IAP/ISP/EEPROM ������������

    IAP_ADDRH = (addr&0xff00)>>8;    //����Ŀ�굥Ԫ��ַ�ĸ�8 λ��ַ
    IAP_ADDRL = (addr&0xff);    //����Ŀ�굥Ԫ��ַ�ĵ�8 λ��ַ

//		EA=0;
    IAP_TRIG = 0x5A;   //���� 5Ah,����A5h ��ISP/IAP �����Ĵ���,ÿ�ζ������
    IAP_TRIG = 0xA5;   //����A5h ��ISP/IAP ����������������
    _nop_();

//		EA=1;
    IAP_Disable();  //�ر�IAP ����, ����ص����⹦�ܼĴ���,ʹCPU ���ڰ�ȫ״̬,
	//һ��������IAP �������֮����ر�IAP ����,����Ҫÿ�ζ���
}
void IAP_Disable()
{
    //�ر�IAP ����, ����ص����⹦�ܼĴ���,ʹCPU ���ڰ�ȫ״̬,
    //һ��������IAP �������֮����ر�IAP ����,����Ҫÿ�ζ���
    IAP_CONTR = 0;      //�ر�IAP ����
    IAP_CMD   = 0;      //������Ĵ���,ʹ����Ĵ���������,�˾�ɲ���
    IAP_TRIG  = 0;      //��������Ĵ���,ʹ������Ĵ����޴���,�˾�ɲ���
    IAP_ADDRH = 0x80;
    IAP_ADDRL = 0;
}

void Delay_ms(UINT ms){
UINT k;
unsigned char i, j;
for(k=0;k<ms;k++){
	_nop_();
	_nop_();
	i = 22;
	j = 128;
	do
	{
		while (--j);
	} while (--i);
}

}