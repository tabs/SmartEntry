#include "LD9900MT.h"
#include "IAP15W4K61S4.h"
#include "flashfunction.h"
#include <string.h>
extern  unsigned int u8RevcLen;
extern  unsigned char FingerDataBuffer[600];
extern  unsigned char FingerPrcBuffer[600];
extern  unsigned char CommonDataBuffer[600];
extern const unsigned long int code FingerId_AddrTable[16][2];
extern 	void Delay_ms(unsigned int ms);
void USART1_Send(unsigned char *dat,unsigned int len);
extern void USART4_Send(unsigned char *dat,unsigned int len);
extern void Muti_Send_Byte(unsigned char *Data,unsigned int len);
extern unsigned int Fe_data_Organize(unsigned char CMD[],unsigned char Data[],unsigned int DataNum,bit OrgType);
extern const unsigned char code CMD_SecondRecordFinger[2];/*第二次录取指纹*/
extern unsigned char Delete_IdOrErWeiMa_Info(unsigned char *IdData,unsigned char SecletFlag);
extern unsigned long int Search_IdOrErWeiMa_Info(unsigned char *Data,unsigned char SecletFlag);
extern unsigned char Store_IdOrErWeiMa_Data(unsigned char *Data,unsigned char SecletFlag);
ZA_8CONST GETIMAGE_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x01,0x00,0x05};
ZA_8CONST GENCHAR_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x04,0x02};
ZA_8CONST CREATET_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x05,0x00,0x09};
ZA_8CONST UPCHAR_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x04,0x08};
ZA_8CONST DOWNCHAR_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x04,0x09};
ZA_8CONST DOWNCHAR_PROCESS_START_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x02,0x00,0x82};
ZA_8CONST DOWNCHAR_PROCESS_END_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x08,0x00,0x82};
ZA_8CONST STORE_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x06,0x06};
ZA_8CONST LOADCHAR_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x06,0x07};
ZA_8CONST DELETECHAR_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x07,0x0C};
ZA_8CONST EMPTY_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x0D,0x00,0x11};
ZA_8CONST MATCH_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x03,0x00,0x07};
ZA_8CONST SEARCH_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x08,0x04};
ZA_8CONST READTN_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x1D,0x00,0x21};
ZA_8CONST SETSYSP_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x05,0x0E};
ZA_8CONST GETSYSP_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x0F,0x00,0x13};
ZA_8CONST READFPFLASH_CMD[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x04, 0x1F};

//下面函数由外部提供,下面是51单片机的实现，与硬件和外部有关函数的实现

//ZA_8U LD9900MT_ReceiveBuff[32]={0};
//ZA_16U u8RevcLen=0;

//void LD9900MT_DelayMs(ZA_16U u16Count)
//{
//Delay_ms(u16Count);
//}
/*
void LD9900MT_PowerOn(void)
{
LD9900MT_POWER_PIN=1;
}
void LD9900MT_PowrDown(void)
{
LD9900MT_POWER_PIN=0;
}
*/


ZA_8U LD9900MT_Receive(void)
{
ZA_8U len=0;
//ZA_16U iTimeOut=0;
//while(true)
//{
//	iTimeOut++;
//	if(50000==iTimeOut)
//	{
//		#ifdef __C51__
//		WatchDog();
//		#endif
//		break;
//	}
//}	
len=u8RevcLen;
u8RevcLen=0;
//ET0=1;
return len;
}


void LD9900MT_Send(ZA_8U *u8Data,ZA_8U len)
{
USART1_Send(u8Data,len);
//ET0=0;//close extern interrupt 0
}


//与硬件有关外部提供的函数到此结束


static const ZA_16U CMD_AddSum(ZA_8U *u8CmdData,ZA_8U len)
{
ZA_8U i=0;
ZA_16U res=0;
for(i=6;i<len;i++)
{
res +=u8CmdData[i];
}
return res;
}

//static const ZA_16U CMD_AttachData_AddSum(ZA_8U *u8CmdData,ZA_8U len)
//{
//ZA_8U i=0;
//ZA_16U res=0;
//for(i=0;i<len;i++)
//{
//res +=u8CmdData[i];
//}
//return res;
//}


static ZALD_Res LD9900MT_CMDPorcess(ZA_8U *u8CMD,ZA_16U u16CMDLen,ZA_8U *u8AttachData,ZA_16U u16ADLen,ZA_16U AckWaitTime)
{

	memset(FingerDataBuffer,0x00,600);
  LD9900MT_Send(u8CMD,u16CMDLen);

  if(0!=u16ADLen)
  {LD9900MT_Send(u8AttachData,u16ADLen);}
  LD9900MT_Receive();
  Delay_ms(AckWaitTime);										//等待接收数据

	//USART_Send(FingerDataBuffer,15,4);
  if((0xEF==FingerDataBuffer[0]) && (0x01==FingerDataBuffer[1]))
  {return FingerDataBuffer[9]; }
   
  {return LD_NODEVICE;}
}


//ZALD_Res LD9900MT_Init(void)
//{
////LD9900MT_PowerOn();
//Delay_ms(600); //at least 500ms
////send the GETIMAGE_CM test the LD9900MT module is work normal
//return LD9900MT_CMDPorcess(GETIMAGE_CMD,sizeof(GETIMAGE_CMD),(ZA_8U*)0,0);
//}

//ZALD_Res LD9900MT_DeInit(void)
//{
////LD9900MT_PowrDown();
//return LD_OK;
//}

ZALD_Res LD9900MT_GetImage(ZA_16U Time)
{
return LD9900MT_CMDPorcess(GETIMAGE_CMD,sizeof(GETIMAGE_CMD),(ZA_8U*)0,0,Time);
}

ZALD_Res LD9900MT_GenChar(ZA_8U u8BufferID,ZA_16U Time)
{
  ZA_8U u8AttachData[3]={0};
  ZA_16U u16Sum=0;
  u8AttachData[0]=u8BufferID;
  u16Sum=CMD_AddSum(GENCHAR_CMD,sizeof(GENCHAR_CMD))+u8AttachData[0];
  u8AttachData[1]=(ZA_8U)(u16Sum>>8);
  u8AttachData[2]=(ZA_8U)u16Sum;
  return LD9900MT_CMDPorcess(GENCHAR_CMD,sizeof(GENCHAR_CMD),u8AttachData,sizeof(u8AttachData),Time);
}

ZALD_Res LD9900MT_CreateTemplate(void)
{
return LD9900MT_CMDPorcess(CREATET_CMD,sizeof(CREATET_CMD),(ZA_8U*)0,0,600);
}



ZALD_Res LD9900MT_UpChar(ZA_8U u8BufferID)
{
ZA_8U u8AttachData[3]={0};
ZA_16U u16Sum=0;
u8AttachData[0]=u8BufferID;
u16Sum=CMD_AddSum(UPCHAR_CMD,sizeof(UPCHAR_CMD))+u8AttachData[0];
u8AttachData[1]=(ZA_8U)(u16Sum>>8);
u8AttachData[2]=(ZA_8U)u16Sum;
return LD9900MT_CMDPorcess(UPCHAR_CMD,sizeof(UPCHAR_CMD),u8AttachData,sizeof(u8AttachData),600);
}

ZALD_Res LD9900MT_DwonChar(ZA_8U u8BufferID)
{
ZA_8U u8AttachData[3]={0};
ZA_16U u16Sum=0;
u8AttachData[0]=u8BufferID;
u16Sum=CMD_AddSum(DOWNCHAR_CMD,sizeof(DOWNCHAR_CMD))+u8AttachData[0];
u8AttachData[1]=(ZA_8U)(u16Sum>>8);
u8AttachData[2]=(ZA_8U)u16Sum;
return LD9900MT_CMDPorcess(DOWNCHAR_CMD,sizeof(DOWNCHAR_CMD),u8AttachData,sizeof(u8AttachData),600);
}

ZALD_Res LD9900MT_Store(ZA_8U u8BufferID ,ZA_16U u16PageID)
{
ZA_8U u8AttachData[5]={0};
ZA_16U u16Sum=0;
u8AttachData[0]=u8BufferID;
u8AttachData[1]=(ZA_8U)(u16PageID>>8);
u8AttachData[2]=(ZA_8U)u16PageID;
u16Sum=CMD_AddSum(STORE_CMD,sizeof(STORE_CMD))+u8AttachData[0]+u8AttachData[1]+u8AttachData[2];
u8AttachData[3]=(ZA_8U)(u16Sum>>8);
u8AttachData[4]=(ZA_8U)u16Sum;
return LD9900MT_CMDPorcess(STORE_CMD,sizeof(STORE_CMD),u8AttachData,sizeof(u8AttachData),600);
}

ZALD_Res LD9900MT_LoadChar(ZA_8U u8BufferID ,ZA_16U u16PageID){
ZA_8U u8AttachData[5]={0};
ZA_16U u16Sum=0;
u8AttachData[0]=u8BufferID;
u8AttachData[1]=(ZA_8U)(u16PageID>>8);
u8AttachData[2]=(ZA_8U)u16PageID;
u16Sum=CMD_AddSum(LOADCHAR_CMD,sizeof(LOADCHAR_CMD))+u8AttachData[0]+u8AttachData[1]+u8AttachData[2];
u8AttachData[3]=(ZA_8U)(u16Sum>>8);
u8AttachData[4]=(ZA_8U)u16Sum;
return LD9900MT_CMDPorcess(LOADCHAR_CMD,sizeof(LOADCHAR_CMD),u8AttachData,sizeof(u8AttachData),100);
}

ZALD_Res LD9900MT_DeleteChar(ZA_16U u16PageID ,ZA_16U u16DeleteNum)
{
ZA_8U u8AttachData[6]={0};
ZA_16U u16Sum=0;
u8AttachData[0]=(ZA_8U)(u16PageID>>8);
u8AttachData[1]=(ZA_8U)u16PageID;
u8AttachData[2]=(ZA_8U)(u16DeleteNum>>8);
u8AttachData[3]=(ZA_8U)u16DeleteNum;
u16Sum=CMD_AddSum(DELETECHAR_CMD,sizeof(DELETECHAR_CMD))+u8AttachData[0]+u8AttachData[1]+u8AttachData[2]+u8AttachData[3];
u8AttachData[4]=(ZA_8U)(u16Sum>>8);
u8AttachData[5]=(ZA_8U)u16Sum;
return LD9900MT_CMDPorcess(DELETECHAR_CMD,sizeof(DELETECHAR_CMD),u8AttachData,sizeof(u8AttachData),100);
}

ZALD_Res LD9900MT_Empty(void)
{
return LD9900MT_CMDPorcess(EMPTY_CMD,sizeof(EMPTY_CMD),(ZA_8U *)0,0,600);
}

ZALD_Res LD9900MT_Match(ZA_16U *u16MatchScore)
{
ZALD_Res res=LD_OK;
res=LD9900MT_CMDPorcess(MATCH_CMD,sizeof(MATCH_CMD),(ZA_8U *)0,0,600);
if(LD_OK==res)
{
*u16MatchScore=(ZA_16U)(FingerDataBuffer[10]<<8)+FingerDataBuffer[11];
}
return res;
}

ZALD_Res LD9900MT_Search(ZA_8U u8BufferID ,ZA_16U u16StartPage ,ZA_16U u16PageNum,ZA_8U *u8IdNumber)
{
ZALD_Res res=LD_OK;
ZA_8U u8AttachData[7]={0},i=0;
ZA_16U u16Sum=0;
u8AttachData[0]=u8BufferID;
u8AttachData[1]=(ZA_8U)(u16StartPage>>8);
u8AttachData[2]=(ZA_8U)u16StartPage;
u8AttachData[3]=(ZA_8U)(u16PageNum>>8);
u8AttachData[4]=(ZA_8U)u16PageNum;
u16Sum=CMD_AddSum(SEARCH_CMD,sizeof(SEARCH_CMD))+u8AttachData[0]+u8AttachData[1]+u8AttachData[2]
																											+u8AttachData[3]+u8AttachData[4];
u8AttachData[5]=(ZA_8U)(u16Sum>>8);
u8AttachData[6]=(ZA_8U)u16Sum;

res= LD9900MT_CMDPorcess(SEARCH_CMD,sizeof(SEARCH_CMD),u8AttachData,sizeof(u8AttachData),200);
for(i=0;i<2;i++)
{
	u8IdNumber[i]=FingerDataBuffer[10+i];
}
return res;

}

ZALD_Res LD9900MT_TemplateNum(ZA_16U *u16TeNum)
{
ZALD_Res res=LD_OK;
res=LD9900MT_CMDPorcess(READTN_CMD,sizeof(READTN_CMD),(ZA_8U *)0,0,600);
if(LD_OK==res)
{
*u16TeNum=(ZA_16U)(FingerDataBuffer[10]<<8)+FingerDataBuffer[11];
}
return res;
}

ZALD_Res LD9900MT_SetSysParm(ZA_8U u8ParmNum, ZA_8U u8Data)
{
ZA_8U u8AttachData[4]={0};
ZA_16U u16Sum=0;
u8AttachData[0]=u8ParmNum;
u8AttachData[1]=u8Data;
u16Sum=CMD_AddSum(SETSYSP_CMD,sizeof(SETSYSP_CMD))+u8AttachData[0]+u8AttachData[1];
u8AttachData[3]=(ZA_8U)(u16Sum>>8);
u8AttachData[4]=(ZA_8U)u16Sum;
return LD9900MT_CMDPorcess(SETSYSP_CMD,sizeof(SETSYSP_CMD),u8AttachData,sizeof(u8AttachData),600);
}


ZALD_Res LD9900MT_GetSysParm(ZA_8U *u8SysParm)
{
ZALD_Res res=LD_OK;
ZA_8U i=0;
res=LD9900MT_CMDPorcess(GETSYSP_CMD,sizeof(GETSYSP_CMD),(ZA_8U *)0,0,600);
if(LD_OK==res)
{
for(i=0;i<16;i++)
{
	u8SysParm[i]=FingerDataBuffer[10+i];
}
}
return res;
}


ZALD_Res LD9900MT_ReadFpflash(ZA_8U u8PageID,ZA_8U *u8FPcontent)
{
ZALD_Res res=LD_OK;
ZA_8U i=0;
ZA_8U u8AttachData[3]={0};
ZA_16U u16Sum=0;
u8AttachData[0]=(ZA_8U)u8PageID;
u16Sum=CMD_AddSum(READFPFLASH_CMD,sizeof(READFPFLASH_CMD))+u8AttachData[0];
u8AttachData[1]=(ZA_8U)(u16Sum>>8);
u8AttachData[2]=(ZA_8U)u16Sum;
res=LD9900MT_CMDPorcess(READFPFLASH_CMD,sizeof(READFPFLASH_CMD),u8AttachData,sizeof(u8AttachData),600);
if(LD_OK==res)
{
for(i=0;i<32;i++)
{
	u8FPcontent[i]=FingerDataBuffer[10+i];
}
}
return res;
}

//此函数返回指纹模块中空闲的位置的指纹编号
ZA_16U LD9900MT_SearchEmpty(void){
	ZA_8U FP_content[32] = 0x00,i=0,j=0,k=0;
//					for(k=0;k<32;k++){
//		 SBUF = FP_content[k];while(!(SCON&0x02));SCON &= ~0x02;
//		}
	for(i=0;i<3;i++){
	if(LD9900MT_ReadFpflash(i,FP_content) == 0x00)
		for(j=0;j<32;j++){
			for(k=0;k<8;k++){
				if((FP_content[j]>>k & 0x01) != 0x01)
					return i*32*8+j*8+k;
				else
					;
			}
		}
	}
return 0;
}


/***********************************************************
函数名称：LD9900MT_UpChar_ReDataProc()
函数功能：将从指纹模块接收到的512字节指纹数据分析
入口函数：无
出口函数：无
***********************************************************/
void LD9900MT_UpCharDataProc(unsigned char *T){

ZA_16U i,j;
	for(j=0;j<556;j++)
	{
		if(j==21){
			for(i=0;i<128;i++,j++)
		FingerPrcBuffer[i] = FingerDataBuffer[j];
		}
		if(j==160){
			for(i=128;i<256;i++,j++)
		FingerPrcBuffer[i] = FingerDataBuffer[j];
		}
		if(j==299){
			for(i=256;i<384;i++,j++)
		FingerPrcBuffer[i] = FingerDataBuffer[j];
		}
		if(j==438){
			for(i=384;i<512;i++,j++)
		FingerPrcBuffer[i] = FingerDataBuffer[j];
		}
	}
Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(T,FingerPrcBuffer,512,0));
memset(FingerPrcBuffer,0x00,600);
memset(CommonDataBuffer,0x00,600);
}


static const ZA_16U CMD_AttachData_AddSum(ZA_8U *u8CmdData,ZA_8U len)
{
ZA_8U i=0;
ZA_16U res=0;
for(i=0;i<len;i++)
{
res +=u8CmdData[i];
}
return res;
}



/***********************************************************
函数名称：LD9900MT_DwonChar_ReDataProc()
函数功能：将从上位机接收到的512字节指纹数据分析并提取出来下载到
指纹模块
入口函数：无
出口函数：无
***********************************************************/
bit LD9900MT_DownChar_ReDataProc(unsigned char * FingerData,unsigned char *FingerId){

ZA_16U i,u16Sum = 0,StoreAddr = 0x0000,*MatchScore;
ZA_8U FingerStoreNumber[2]={0x00};
ZA_8U UserId[4] = {0x00},Cursor=0;
unsigned long int UseridStoreAddr=0;

LD9900MT_DwonChar(0x02);																										//下载到CharBuffer2 随后指纹模块接收指纹数据
	Delay_ms(10);
for(i=0;i<4;i++)
{
	UserId[i] = *FingerId++;
}
	
for(i=0;i<128;i++)
	FingerPrcBuffer[i] = *FingerData++;

u16Sum=CMD_AddSum(DOWNCHAR_PROCESS_START_CMD,sizeof(DOWNCHAR_PROCESS_START_CMD)) + CMD_AttachData_AddSum(FingerPrcBuffer,128);
FingerPrcBuffer[128]=(ZA_8U)(u16Sum>>8);
FingerPrcBuffer[129]=(ZA_8U)u16Sum;
LD9900MT_CMDPorcess(DOWNCHAR_PROCESS_START_CMD,sizeof(DOWNCHAR_PROCESS_START_CMD),FingerPrcBuffer,130,10);


for(i=0;i<128;i++)
	FingerPrcBuffer[i] =*FingerData++;

u16Sum=CMD_AddSum(DOWNCHAR_PROCESS_START_CMD,sizeof(DOWNCHAR_PROCESS_START_CMD)) + CMD_AttachData_AddSum(FingerPrcBuffer,128);
FingerPrcBuffer[128]=(ZA_8U)(u16Sum>>8);
FingerPrcBuffer[129]=(ZA_8U)u16Sum;
LD9900MT_CMDPorcess(DOWNCHAR_PROCESS_START_CMD,sizeof(DOWNCHAR_PROCESS_START_CMD),FingerPrcBuffer,130,10);


for(i=0;i<128;i++)
	FingerPrcBuffer[i] = *FingerData++;

u16Sum=CMD_AddSum(DOWNCHAR_PROCESS_START_CMD,sizeof(DOWNCHAR_PROCESS_START_CMD)) + CMD_AttachData_AddSum(FingerPrcBuffer,128);
FingerPrcBuffer[128]=(ZA_8U)(u16Sum>>8);
FingerPrcBuffer[129]=(ZA_8U)u16Sum;
LD9900MT_CMDPorcess(DOWNCHAR_PROCESS_START_CMD,sizeof(DOWNCHAR_PROCESS_START_CMD),FingerPrcBuffer,130,10);


for(i=0;i<128;i++)
	FingerPrcBuffer[i] = *FingerData++;

u16Sum=CMD_AddSum(DOWNCHAR_PROCESS_END_CMD,sizeof(DOWNCHAR_PROCESS_END_CMD)) + CMD_AttachData_AddSum(FingerPrcBuffer,128);
FingerPrcBuffer[128]=(ZA_8U)(u16Sum>>8);
FingerPrcBuffer[129]=(ZA_8U)u16Sum;
LD9900MT_CMDPorcess(DOWNCHAR_PROCESS_END_CMD,sizeof(DOWNCHAR_PROCESS_END_CMD),FingerPrcBuffer,130,10);

memset(FingerPrcBuffer,0x00,600);


 FLASH_W25Q64_Read(FingerStoreNumber,Search_IdOrErWeiMa_Info(UserId,0x03)+6,2);//搜索对应指纹ID对应的指纹编号
 if(FingerStoreNumber[0]==0xff && FingerStoreNumber[1]==0xff)
{
	StoreAddr = 0;
}else
{
	 StoreAddr=FingerStoreNumber[0]*256+FingerStoreNumber[1];
}

 if(LD9900MT_LoadChar(0x01,StoreAddr)==0x00 | LD9900MT_LoadChar(0x01,StoreAddr)==0x0c)//0ch表示读出有错或模板无效
 {
//	 	S4BUF = 0xb8;while(!(S4CON&0x02));S4CON &= ~0x02;
	if(LD9900MT_Match(MatchScore) == 0x00){
	//储存指纹ID号并更新FLASH中指纹模块中的指纹编号
//	S4BUF = 0xa8;while(!(S4CON&0x02));S4CON &= ~0x02;
	Store_IdOrErWeiMa_Data(&UserId,0x03);
	FLASH_W25Q64_Update (Search_IdOrErWeiMa_Info(UserId,0x03)+6, FingerStoreNumber, 2);
//	Cursor = UserId[0]+UserId[1]+UserId[2]+UserId[3];
//	FLASH_W25Q64_Read(CommonDataBuffer,FingerId_AddrTable[Cursor&= 0x0f][0],600);//读入缓冲区
//		//S4BUF = 0xa2;while(!(S4CON&0x02));S4CON &= ~0x02;
//		for(i=0;i<600;i++){
//		 S4BUF = CommonDataBuffer[i];while(!(S4CON&0x02));S4CON &= ~0x02;_nop_();_nop_();
//		}
		return 1;
	}
		else{
//			S4BUF = 0xa9;while(!(S4CON&0x02));S4CON &= ~0x02;
			StoreAddr = LD9900MT_SearchEmpty();
			LD9900MT_Store(0x02 ,StoreAddr);																							//储存到指纹库
	  	FingerStoreNumber[0]=StoreAddr>>8;
		  FingerStoreNumber[1]=StoreAddr;
			UseridStoreAddr = Search_IdOrErWeiMa_Info(UserId,0x03);
			if(UseridStoreAddr!=0)
			{
			FLASH_W25Q64_Update (UseridStoreAddr+6, FingerStoreNumber, 2);
				return 1;
			}else if(UseridStoreAddr==0)
				{
						Store_IdOrErWeiMa_Data(UserId,0x03);
						FLASH_W25Q64_Write(Search_IdOrErWeiMa_Info(UserId,0x03)+6, FingerStoreNumber, 2);
//						Cursor = UserId[0]+UserId[1]+UserId[2]+UserId[3];
//						FLASH_W25Q64_Read(CommonDataBuffer,FingerId_AddrTable[Cursor&= 0x0f][0],600);//读入缓冲区
//						//S4BUF = 0xa2;while(!(S4CON&0x02));S4CON &= ~0x02;
//							for(i=0;i<600;i++){
//								S4BUF = CommonDataBuffer[i];while(!(S4CON&0x02));S4CON &= ~0x02;_nop_();_nop_();}
						return 1;
				}
			return 1;
		}
	}
// S4BUF = 0xb9;while(!(S4CON&0x02));S4CON &= ~0x02;
 return 1;
}











