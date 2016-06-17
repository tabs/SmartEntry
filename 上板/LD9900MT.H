/*****************************************************
File name ：LD9900MT.h
File Version: V1.0
File Function: head file of LD9900MT finger print module driver
File Date : 2014-05-28
File Editor : yilin
Company :Zhi ang Tec.
@Content:The finger print module communication with the MCU by USART
*****************************************************/
#ifndef __LD9900MT_H__
#define __LD9900MT_H__


 #define ZA_8U unsigned char
 #define ZA_16U  unsigned int
 #define ZA_8CONST const unsigned char code
 #define true 1	 
 #define LD9900MT_TOUCH_O P40

typedef enum LD9900MT_RES_ENUM
{
	LD_OK=0x00,
	LD_DPERROR=0x01,  //data package error
	LD_NOFINGER=0x02, //not finger on the sensor
	LD_RFPFAIL=0x03,  //Record fingerprint fail
	LD_IBFAIL=0x06,   //image of fingerprint is bad
	LD_CLFAIL=0x07,   //characteristic too less can not create
	LD_NOTMATCH=0x08, //fingerprint not match
	LD_NOTSECRCH=0x09,//not searched the fingerprint
	LD_CCFAIL=0x0A,   //characteristic combine fail
	LD_OUTADDRESS=0x0B,//the address number over the fingerprint library
	LD_READFAIL=0x0C,  //read the fingerprint template fail or not valid
	LD_ULFAIL=0x0D,   //upload the fingerprint characteristic fail
	LD_RRDATA=0x0E,   //the module refuse receive the next data
	LD_IULFAIL=0x0F,  //upload image fail
	LD_DFTFAIL=0x10,  //delete the fingerprint template fail
	LD_CPLFAIL=0x11, //clear the fingerprint library fail
	LD_PWERROR=0x13, //the password error
	LD_NVDFAIL=0x15, //buffer not have vaild data to create image
	LD_WRFERROR=0x18, //write or read flash error
	LD_UNKNOW=0x19,   //unknow error
	LD_NVRN=0x1A,     //not valid register number
	LD_SRERROR=0x1B,  //set the register data error
	LD_NPERROR=0x1C,  //note page error
	LD_POFAIL=0x1D,   //port operate fail
	LD_NODEVICE=0xFE, //no device
}ZALD_Res;

//下面这些函数需要外部提供，在LD9900MT.c文件中给了51单片机的实现
extern void LD9900MT_PowerOn(void);
extern void LD9900MT_PowrDown(void);
extern ZA_8U LD9900MT_Receive(void);
//extern ZA_8U LD9900MT_ReceiveBuff[32];
//extern void LD9900MT_DelayMs(ZA_16U u16Count);
extern void LD9900MT_Send(ZA_8U *u8Data,ZA_8U len);
//需要外部提供的函数到此结束

//下面的函数供用户使用
//under function provide to user
//ZALD_Res LD9900MT_Init(void);
//ZALD_Res LD9900MT_DeInit(void);
ZALD_Res LD9900MT_GetImage(ZA_16U Time);
ZALD_Res LD9900MT_GenChar(ZA_8U u8BufferID,ZA_16U Time);
ZALD_Res LD9900MT_CreateTemplate(void);
ZALD_Res LD9900MT_UpChar(ZA_8U u8BufferID);
ZALD_Res LD9900MT_DwonChar(ZA_8U u8BufferID);
ZALD_Res LD9900MT_Store(ZA_8U u8BufferID ,ZA_16U u16PageID);
ZALD_Res LD9900MT_LoadChar(ZA_8U u8BufferID ,ZA_16U u16PageID);
ZALD_Res LD9900MT_DeleteChar(ZA_16U u16PageID ,ZA_16U u16DeleteNum);
ZALD_Res LD9900MT_Empty(void);
ZALD_Res LD9900MT_Match(ZA_16U *u16MatchScore);
ZALD_Res LD9900MT_Search(ZA_8U u8BufferID ,ZA_16U u16StartPage ,ZA_16U u16PageNum,ZA_8U *u8IdNumber);
ZALD_Res LD9900MT_TemplateNum(ZA_16U *u16TeNum);
ZALD_Res LD9900MT_SetSysParm(ZA_8U u8ParmNum, ZA_8U u8Data);
ZALD_Res LD9900MT_GetSysParm(ZA_8U *u8SysParm);
ZALD_Res LD9900MT_ReadFpflash(ZA_8U u8PageID,ZA_8U *u8FPcontent);
bit LD9900MT_DownChar_ReDataProc(unsigned char * FingerData,unsigned char *FingerId);
void LD9900MT_UpCharDataProc(unsigned char *T);
ZA_16U LD9900MT_SearchEmpty(void);
//void LD9900MT_UpChar_ReDataProc(void);
//bit LD9900MT_DownChar_ReDataProc(void);
//void SmartEntry_SendProcess(unsigned char *MT, unsigned char *Data, unsigned int len);

#endif