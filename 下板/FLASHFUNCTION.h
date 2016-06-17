#ifndef __FLASHFUNCTION_H__
#define __FLASHFUNCTION_H__
#include "IAP15W4K61S4.h"
#include <intrins.H>
#include <string.h>

typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef ULONG FLADDR;

/****************************FLASH_W25Q64����*********************************/
sbit 		WDI 			= P4^0;    //W25Q64��������
sbit 		WDO 			= P4^2;    //W25Q64�������
sbit 		WCK 			= P4^1;    //W25Q64ʱ��
sbit 		WCS 			= P4^3;    //W25Q64ѡ�� 


//����˿ڲ���
#define	W25Q64_WCS_Clr()		{WCS=0;}
#define	W25Q64_WCS_Set()		{WCS=1;}
#define	W25Q64_WDI_Clr()		{WDI=0;} 
#define	W25Q64_WDI_Set()		{WDI=1;}
#define	W25Q64_WDO_Clr()		{WDO=0;} 
#define	W25Q64_WDO_Set()		{WDO=1;}
#define	W25Q64_WCK_Clr()		{WCK=0;} 
#define	W25Q64_WCK_Set()		{WCK=1;}
//W25Q64����FLASH��ID����
#define 	sFLASH_W25Q64_ID		  0xEF16	//оƬID
#define 	sFLASH_SPI_PAGESIZE       0x0100	//���ҳ��С256�ֽ�
//W25Q64����FLASH�Ĳ��������
#define 	sFLASH_CMD_WREN           0x06  	//дʹ��
#define 	sFLASH_CMD_WRDIS          0x04  	//д��ֹ
#define 	sFLASH_CMD_RDSR           0x05  	//��״̬�Ĵ���
#define 	sFLASH_CMD_WRSR           0x01  	//д״̬�Ĵ���
#define 	sFLASH_CMD_READ           0x03  	//������
#define 	sFLASH_CMD_HREAD          0x0B		//���ٶ�����
#define 	sFLASH_CMD_HREADDO        0x3B		//���ٶ���˫·���
#define 	sFLASH_CMD_PAGEPRO        0x02  	//ҳ��� 
#define 	sFLASH_CMD_SE             0x20  	//��������
#define 	sFLASH_CMD_BE             0xD8  	//�����
#define 	sFLASH_CMD_CE             0xC7  	//оƬ����
#define 	sFLASH_CMD_POWERD         0xB9  	//����ģʽ
#define 	sFLASH_CMD_RDID           0x90  	//���������Һ�����ID��
#define 	sFLASH_CMD_JEDECID        0x9F  	//��JEDECID�� 
#define 	sFLASH_DUMMY_BYTE         0x00
#define 	sFLASH_WIP_FLAG           0x01  	//FLASHæ��� 



#define FLASH_W25Q64_PAGESIZE 4096 /*FLASH_W25Q64һҳΪ4k*/
#define FLASH_W25Q64_LAST 0x7FF000L            /*���һҳ����ʼ��ַ�����һҳ����������޸ĵ�ҳ������*/
#define FLASH_W25Q64_TEMP 0x7FF000L
#define FLASH_W25Q64_SCRATCHSIZE 4096





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
/*FLASH_W25Q64�ײ㺯��������*/
void Delay_1us(void);
unsigned int sFLASH_ReadID(void);
void sFLASH_EraseSector(FLADDR SectorAddr);
void sFLASH_64KBEraseSector(FLADDR SectorAddr);
void sFLASH_EraseFlash(void);
void sFLASH_ReadBuffer(unsigned char * pBuffer, FLADDR ReadAddr, unsigned int NumByteToRead);
void sFLASH_WritePage(unsigned char* pBuffer, FLADDR WriteAddr, unsigned int NumByteToWrite);
/****************************************************************************/





/*�ڲ�FLASH����������*/
void IAP_Disable();/*�ر�IAP ����*/
void FLASH_ByteWrite (FLADDR addr, char byte);
unsigned char FLASH_ByteRead (FLADDR addr);
void FLASH_PageErase (FLADDR addr);
UCHAR sequential_write_flash_in_one_sector(UINT begin_addr, UINT counter,UCHAR array[]);

















#endif