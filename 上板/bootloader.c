#include "IAP15W4K61S4.h"
#include <intrins.H>
#include <string.h>
//#define RESETSYSTEM()    ((void (code *)())0xF3F0)() //复位入口
#define USERAP()    		 ((void (code *)())0x1000)() //用户程序起始地址


#define USER_APP_ADDR 0x1000
	//定义一个函数指针，用于跳转到用户程序


typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef ULONG FLADDR;

#define FOSC 22118400L          //时钟频率
#define BAUD 115200             //波特率

#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7



/*定义Flash 操作等待时间及允许IAP/ISP/EEPROM 操作的常数*/
/*#define ENABLE_ISP 0x80*/ //系统工作时钟<30MHz 时，对IAP_CONTR 寄存器设置此值
#define ENABLE_ISP 0x81 //系统工作时钟<24MHz 时，对IAP_CONTR 寄存器设置此值
/*#define ENABLE_ISP 0x82*/ //系统工作时钟<20MHz 时，对IAP_CONTR 寄存器设置此值
/*#define ENABLE_ISP 0x83*/ //系统工作时钟<12MHz 时，对IAP_CONTR 寄存器设置此值
/*#define ENABLE_ISP 0x84*/ //系统工作时钟<6MHz 时，对IAP_CONTR 寄存器设置此值
/*#define ENABLE_ISP 0x85*/ //系统工作时钟<3MHz 时，对IAP_CONTR 寄存器设置此值
/*#define ENABLE_ISP 0x86*/ //系统工作时钟<2MHz 时，对IAP_CONTR 寄存器设置此值
/*#define ENABLE_ISP 0x87*/ //系统工作时钟<1MHz 时，对IAP_CONTR 寄存器设置此值
/****************************************************************************/

//const unsigned char code welcome[]="welcome to SmartEntery System BootLoader!";

#define SysCordAppUpdataFlag 0x7E00
#define UpdataFlag0 0x5A/*是否升级判断标志 0x5a 0xa5*/
#define UpdataFlag1 0xA5
#define SysCordStartAddr 0x1000 /*程序应用区的起始地址*/
#define SysCordEndAddr 0x7200 /*程序应用区的结束地址*/
#define SysCordAppSaveStartTempAddr 0x8000 /*程序应用区暂存的起始地址*/



/*内部FLASH函数声明区*/
void IAP_Disable();/*关闭IAP 功能*/
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

	//bootloader里不能开启中断
	EA = 0;
	Btlder_init_Uart1();//串口1初始化
//	 SBUF = 0x09;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();

//	UART_Send(welcome,sizeof(welcome));
	
//	for(i=0x1000;i<0x1200;i++){
//	 SBUF = FLASH_ByteRead(i);while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//	}
	btlder();
	USERAP();//从用户区启动
	while(1);
}


void Btlder_init_Uart1(void)
{
ACC = P_SW1;
ACC &= ~(S1_S0 | S1_S1);    //S1_S0=0 S1_S1=0
P_SW1 = ACC;                //(P3.0/RxD, P3.1/TxD)
SCON = 0x50;								//1010000
T2L = (65536 - (FOSC/4/BAUD));   //设置波特率重装值
T2H = (65536 - (FOSC/4/BAUD))>>8;
AUXR = 0x14;								//T2为1T模式, 并启动定时器2 10100
AUXR |= 0x01;               //选择定时器2为串口1的波特率发生器
}



void btlder(void)
 {
unsigned int Addr_j,UpdatePacNum ;
unsigned char UART_DATA_buf = 0,UART_DATA_buf0 = 0,IAP_UpdatePacNum=0;

//	 SBUF = 0xf0;while(!(SCON&0x02));SCON &= ~ 0x02;
 /********************************************************************/
 /*上电读取程序是否修改，修改则更新程序，不修改，则继续执行          */
 UART_DATA_buf=FLASH_ByteRead (SysCordAppUpdataFlag);
 UART_DATA_buf0=FLASH_ByteRead ((SysCordAppUpdataFlag+1));
 IAP_UpdatePacNum = FLASH_ByteRead (SysCordAppUpdataFlag+2);
//	 SBUF = UART_DATA_buf;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//	 SBUF = UART_DATA_buf0;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//	 SBUF = IAP_UpdatePacNum;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
 if((UART_DATA_buf==UpdataFlag0)&&(UART_DATA_buf0==UpdataFlag1))/*如果相等则更新源程序*/
 {/*擦除程序区*/
//SBUF = 0xf1;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();

   for(Addr_j=SysCordStartAddr;Addr_j<SysCordEndAddr;Addr_j+=0x200)		//根据包的数量更新用户程序区
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
			FLASH_PageErase (SysCordAppSaveStartTempAddr+UpdatePacNum*0x200);								//擦除临时储存区					
//		  SBUF = 0xf2;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();_nop_();
	 }


	 
//	 SBUF = 0xf3;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
    FLASH_PageErase(SysCordAppUpdataFlag); 
	 
//	 for(Addr_j = 0;Addr_j<8;Addr_j++){
//	 SBUF = 0x88;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//	 }

	 
		IAP_CONTR = 0x20;//重启
   }
 
//	 SBUF = 0xf4;while(!(SCON&0x02));SCON &= ~ 0x02;_nop_();
//   UART_DATA_buf=0;
//   Addr_j=0;
	
	 USERAP();//从用户区启动
	
 }
 
 
 
void FLASH_ByteWrite (FLADDR addr, char byte) 
{
    IAP_CONTR = ENABLE_ISP;         //打开 IAP 功能, 设置Flash 操作等待时间
    IAP_CMD = 0x02;                 //IAP/ISP/EEPROM 字节编程命令
    IAP_ADDRH = (addr&0xff00)>>8;    //设置目标单元地址的高8 位地址
    IAP_ADDRL = (addr&0xff);    //设置目标单元地址的低8 位地址

    IAP_DATA = byte;                  //要编程的数据先送进IAP_DATA 寄存器
//		EA=0;
    IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
    IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动
			_nop_();
//		EA=1;	
    IAP_Disable();  //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
                    //一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
}
/****************************************************************************/
/* FLASH_ByteRead                                                           */ 
/*This routine reads a <byte> from the linear FLASH address <addr>.         */ 
/*                                                                          */ 
/****************************************************************************/
unsigned char FLASH_ByteRead (FLADDR addr)
{
    IAP_DATA = 0x00;
    IAP_CONTR = ENABLE_ISP;         //打开IAP 功能, 设置Flash 操作等待时间
    IAP_CMD = 0x01;                 //IAP/ISP/EEPROM 字节读命令

    IAP_ADDRH = (addr&0xff00)>>8;    //设置目标单元地址的高8 位地址
    IAP_ADDRL = (addr&0xff);    //设置目标单元地址的低8 位地址

//		EA=0;
    IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
    IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动
		_nop_(); 
//		EA=1;
    IAP_Disable();  //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
                    //一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
    return (IAP_DATA);
}



/****************************************************************************/
/* FLASH_PageErase                                                          */ 
/*This routine erases the FLASH page containing the linear FLASH addres     */ 
/* <addr>.                                                                  */ 
/****************************************************************************/
void FLASH_PageErase (FLADDR addr)
{
	
    IAP_CONTR = ENABLE_ISP;         //打开IAP 功能, 设置Flash 操作等待时间
    IAP_CMD = 0x03;                 //IAP/ISP/EEPROM 扇区擦除命令

    IAP_ADDRH = (addr&0xff00)>>8;    //设置目标单元地址的高8 位地址
    IAP_ADDRL = (addr&0xff);    //设置目标单元地址的低8 位地址

//		EA=0;
    IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
    IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动
    _nop_();

//		EA=1;
    IAP_Disable();  //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
	//一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
}
void IAP_Disable()
{
    //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
    //一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
    IAP_CONTR = 0;      //关闭IAP 功能
    IAP_CMD   = 0;      //清命令寄存器,使命令寄存器无命令,此句可不用
    IAP_TRIG  = 0;      //清命令触发寄存器,使命令触发寄存器无触发,此句可不用
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