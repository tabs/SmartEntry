#ifndef __FLASHFUNCTION_H__
#define __FLASHFUNCTION_H__
#include "IAP15W4K61S4.h"
#include <intrins.H>
#include <string.h>

typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef ULONG FLADDR;

/****************************FLASH_W25Q64定义*********************************/
sbit 		WDI 			= P4^0;    //W25Q64数据输入
sbit 		WDO 			= P4^2;    //W25Q64数据输出
sbit 		WCK 			= P4^1;    //W25Q64时钟
sbit 		WCS 			= P4^3;    //W25Q64选择 


//定义端口操作
#define	W25Q64_WCS_Clr()		{WCS=0;}
#define	W25Q64_WCS_Set()		{WCS=1;}
#define	W25Q64_WDI_Clr()		{WDI=0;} 
#define	W25Q64_WDI_Set()		{WDI=1;}
#define	W25Q64_WDO_Clr()		{WDO=0;} 
#define	W25Q64_WDO_Set()		{WDO=1;}
#define	W25Q64_WCK_Clr()		{WCK=0;} 
#define	W25Q64_WCK_Set()		{WCK=1;}
//W25Q64串行FLASH的ID定义
#define 	sFLASH_W25Q64_ID		  0xEF16	//芯片ID
#define 	sFLASH_SPI_PAGESIZE       0x0100	//编程页大小256字节
//W25Q64串行FLASH的操作命令定义
#define 	sFLASH_CMD_WREN           0x06  	//写使能
#define 	sFLASH_CMD_WRDIS          0x04  	//写禁止
#define 	sFLASH_CMD_RDSR           0x05  	//读状态寄存器
#define 	sFLASH_CMD_WRSR           0x01  	//写状态寄存器
#define 	sFLASH_CMD_READ           0x03  	//读数据
#define 	sFLASH_CMD_HREAD          0x0B		//快速读数据
#define 	sFLASH_CMD_HREADDO        0x3B		//快速读，双路输出
#define 	sFLASH_CMD_PAGEPRO        0x02  	//页编程 
#define 	sFLASH_CMD_SE             0x20  	//扇区擦除
#define 	sFLASH_CMD_BE             0xD8  	//块擦除
#define 	sFLASH_CMD_CE             0xC7  	//芯片擦除
#define 	sFLASH_CMD_POWERD         0xB9  	//掉电模式
#define 	sFLASH_CMD_RDID           0x90  	//读生产厂家和器件ID号
#define 	sFLASH_CMD_JEDECID        0x9F  	//读JEDECID号 
#define 	sFLASH_DUMMY_BYTE         0x00
#define 	sFLASH_WIP_FLAG           0x01  	//FLASH忙标记 



#define FLASH_W25Q64_PAGESIZE 4096 /*FLASH_W25Q64一页为4k*/
#define FLASH_W25Q64_LAST 0x7FF000L            /*最后一页的起始地址，最后一页用来缓存待修改的页的数据*/
#define FLASH_W25Q64_TEMP 0x7FF000L
#define FLASH_W25Q64_SCRATCHSIZE 4096





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






//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void FLASH_W25Q64_ByteWrite (FLADDR addr, char byte);
unsigned char FLASH_W25Q64_ByteRead (FLADDR addr);
void FLASH_W25Q64_PageErase (FLADDR addr);
void FLASH_W25Q64_Write (FLADDR dest, char *src, FLADDR numbytes);
char * FLASH_W25Q64_Read (char *dest, FLADDR src, unsigned numbytes);
void FLASH_W25Q64_Clear (FLADDR addr, unsigned numbytes);
void FLASH_W25Q64_Update (FLADDR dest, char *src, unsigned numbytes);
void FLASH_W25Q64_Copy (FLADDR dest, FLADDR src, unsigned numbytes,bit IsFullBlockCopy);
void FLASH_W25Q64_Fill (FLADDR addr, ULONG length, UCHAR fill);
/****************************************************************************/
/*FLASH_W25Q64底层函数声明区*/
void Delay_1us(void);
unsigned int sFLASH_ReadID(void);
void sFLASH_EraseSector(FLADDR SectorAddr);
void sFLASH_64KBEraseSector(FLADDR SectorAddr);
void sFLASH_EraseFlash(void);
void sFLASH_ReadBuffer(unsigned char * pBuffer, FLADDR ReadAddr, unsigned int NumByteToRead);
void sFLASH_WritePage(unsigned char* pBuffer, FLADDR WriteAddr, unsigned int NumByteToWrite);
/****************************************************************************/





/*内部FLASH函数声明区*/
void IAP_Disable();/*关闭IAP 功能*/
void FLASH_ByteWrite (FLADDR addr, char byte);
unsigned char FLASH_ByteRead (FLADDR addr);
void FLASH_PageErase (FLADDR addr);
UCHAR sequential_write_flash_in_one_sector(UINT begin_addr, UINT counter,UCHAR array[]);

















#endif