#include "flashfunction.h"
#include "LD9900MT.h"
#include "DS1302.h"
#include "SC50X0B.h"
#include "init.h"
#define BuzzerTimeNormal	0x01						//触发一次蜂鸣器响的时间（正常100ms）
#define BuzzerTimeLong 0x02								//触发一次蜂鸣器响的时间（拉长3000ms）
#define BuzzerTimeShort 0x03							//急促蜂鸣器（20ms）
#define FE_MESSTYPEADDR 11 /*消息类型字节在帧中的起始位置*/
#define FE_DATAADDR 13 /*数据字节在帧中的起始位置*/
#define FE_CODENUM 10 //帧序号字节在帧中的起始位置
#define FE_LENGTH 2 //定义数据长度在帧中的位置
#define SYSTEM_ID 4 //定义本机编号在帧中的位置
#define FindCard_TimerCounterMAX 50 /*每隔0.5s检测一次身份证是否存在*/
#define ShieldCMD_OpenOrCloseTime 300	/*本地开闸后屏蔽服务器开闸时间3S*/
#define ResetU980Time 100*300			/*二维码定时重启300S*/
#define SYSTEM_ID 4 //定义本机编号在帧中的位置
#define SysCordAppUpdataAddrFlag 0x7E00
#define SysCordAppSaveStartTempAddr 0x8000 /*程序应用区暂存的起始地址*/
#define STATE_NOCARD 0
#define STATE_SELECTCARD 1
#define STATE_READCARD 2
bit DoorTrigger;
bit ResetU980Trigger;
bit ShieldOpenOrClose;
bit OpenOrClose;
bit Flag_ErWeiMaReceiveStart;
unsigned int DoorOpenTime = 10;
unsigned int u8RevcLen = 0;
unsigned char EWMBuffer1[23] ={0x00};
unsigned char EWMBuffer[12] ={0x00};
unsigned char Com2Buf=0;

unsigned int EWMCounter = 0;
unsigned char BluetoothRecBuffer[30] = {0x00};//蓝牙接收数据缓存
unsigned int BTCounter = 0;//蓝牙接收数组计数器
unsigned char FingerDataBuffer[600] = {0x00}  ;
unsigned int DoorCounter = 0;
unsigned int ResetU980Counter = 0;
unsigned int ShieldCMD_OpenOrCloseCounter = 0;
unsigned char System_ID[6]={0x03,0x71,0x00,0x01,0x00,0x01};
const unsigned char code AppVersionNum[4] = {0x20,0x16,0x06,0x14};/*闸机程序版本编号V2016.5.26*/
const unsigned char code FE_START[2]={0x55,0xaa};
const unsigned char code FE_START_ACK[2]={0xaa,0x55};//回复上位机的帧头
const unsigned char code CMD_Heartbeat[2]={0x00,0x01};/*心跳包命令*/
//const unsigned char code CMD_ServerAck[2]={0x01,0x01};/*和服务器通信超时重启WIFI*/
const unsigned char code CMD_ReStart[2]={0x00,0x02};/*重启命令*/
const unsigned char code CMD_UpdateSystem[2]={0x00,0x06};/*上板升级命令*/
const unsigned char code CMD_OpenOrClose[2]={0x00,0x03};/*强制开关闸*/
const unsigned char code CMD_SearchUerID[2]={0x80,0x01};/*搜索游客身份证信息*/
const unsigned char code CMD_SearchErWerMa[2]={0x70,0x01};/*搜索游客二维码信息*/
const unsigned char code CMD_DownLoadUerID[2]={0x30,0x01};/*下载身份证信息*/
const unsigned char code CMD_UploadUerID[2]={0x00,0x19};/*流水功能上传身份证*/
const unsigned char code CMD_DownLoadErWerMa[2]={0x40,0x01};/*下载二维码信息*/
const unsigned char code CMD_DownLoadFingerData[2]={0x50,0x01};/*下载指纹信息*/
const unsigned char code CMD_UploadErWeiMa[2]={0x00,0x29};/*流水功能上传二维码*/
const unsigned char code CMD_FingerUploadStreamInfo[2]={0x00,0x39};/*流水功能上传指纹对应的ID*/
const unsigned char code CMD_DeleteFingerID[2]={0x50,0x02};/*删除单条指纹信息*/
const unsigned char code CMD_DeleteErWeiMa[2]={0x40,0x02};/*删除单条二维码信息*/
const unsigned char code CMD_DeleteID[2]={0x30,0x02};/*删除单条身份证信息*/
const unsigned char code CMD_DeleteAllErWeiMa[2]={0x40,0x03};/*删除所有二维码信息*/
const unsigned char code CMD_DeleteAllID[2]={0x30,0x03};/*删除所有身份证信息*/
const unsigned char code CMD_DeleteAllUserID[2]={0x88,0x88};/*删除所有用户ID信息*/
const unsigned char code CMD_FirstRecordFinger[2]={0x31, 0x03};/*第一次录取指纹*/
const unsigned char code CMD_SecondRecordFinger[2]={0x32, 0x03};/*第二次录取指纹*/
const unsigned char code CMD_EmptyFingerRecord[2]={0x50, 0x03};/*清空指纹库*/
const unsigned char code CMD_AskAppVersionNum_Top[2]={0x60,0x01};/*查询闸机上板软件版本号*/
const unsigned char code CMD_AskAppVersionNum_Bottom[2]={0x60,0x02};/*查询闸机下板软件版本号*/
const unsigned char code CMD_WifiSerialConfigMode[2]={0xf8,0x01};/*进入WIFI串口配置模式命令*/
const unsigned char code CMD_Buzzer[2]={0xff,0xff};/*蜂鸣器命令*/
const unsigned char code CMD_DoorTrigger[2]={0xfe,0xfe};/*内部开闸命令*/
const unsigned char code CMD_SetSystemId[2]={0xff,0x01};/*闸机ID设置命令*/
const unsigned char code CMD_nReloadWifi[2]={0xff,0x02};/*闸机重置WIFI命令*/
const unsigned char code CMD_SetDoorOpenTime[2]={0xff,0x03};/*闸机开门时间设置命令*/
const unsigned char code CMD_DeleteSystem_SetPara[2]={0xff,0x04};/*删除系统参数设置*/
const unsigned char code FIND_CARD[10]={0xaa, 0xaa, 0xaa, 0x96, 0x69, 0x00, 0x03, 0x20, 0x01, 0x22};
const unsigned char code SELECT_CARD[10]={0xaa, 0xaa, 0xaa, 0x96, 0x69, 0x00, 0x03, 0x20, 0x02, 0x21};
const unsigned char code READ_CARD[10]={0xaa, 0xaa, 0xaa, 0x96, 0x69, 0x00, 0x03, 0x30, 0x01, 0x32};

//0x1000-0x7E00  ?C_C51STARTUP(1000H)

//身份证储存划分储存地址表,每条信息占用20字节
//身份证储存根据身份证号最后一位校验码划分为11个区域，每个区域128KB 每条身份证信息加上时间20个字节
//每个区域共可以储存6553-3条信息，这里前3*20=60字节用来储存附加的信息，比如当前区域的储存数量。每个区域128KB共占用1408KB空间
const unsigned long int code Id_StoreAddrTable[11][2] = {{0x000000,0x1ffff},{0x20000,0x3ffff},{0x40000,0x5ffff},{0x60000,0x7ffff},
{0x80000,0x9ffff},{0xa0000,0xbffff},{0xc0000,0xdffff},{0xe0000,0xfffff},{0x100000,0x11ffff},{0x120000,0x13ffff},{0x140000,0x15ffff}};
//二维码储存划分储存地址表，每条信息占用10字节
//二维码储存根据手机号累加得到一个int型数据取低4位作为分类依据0x00-0x0f(16个分类)
//每个区域共可以储存6553-3条信息，这里前3*10=30字节用来储存附加的信息，比如当前区域的储存数量。每个区域64KB共占用1024KB空间
const unsigned long int code ErWeiMa_AddrTable[16][2] = {{0x160000,0x16ffff},{0x170000,0x17ffff},{0x180000,0x18ffff},{0x190000,0x19ffff},
{0x1a0000,0x1affff},{0x1b0000,0x1bffff},{0x1c0000,0x1cffff},{0x1d0000,0x1dffff},{0x1e0000,0x1effff},{0x1f0000,0x1fffff},{0x200000,0x20ffff},
{0x210000,0x21ffff},{0x220000,0x22ffff},{0x230000,0x23ffff},{0x240000,0x24ffff},{0x250000,0x25ffff}};
//用户id-指纹编号的储存和二维码的相同
const unsigned long int code FingerId_AddrTable[16][2] = {{0x260000,0x26ffff},{0x270000,0x27ffff},{0x280000,0x28ffff},{0x290000,0x29ffff},
{0x2a0000,0x2affff},{0x2b0000,0x2bffff},{0x2c0000,0x2cffff},{0x2d0000,0x2dffff},{0x2e0000,0x2effff},{0x2f0000,0x2fffff},{0x300000,0x30ffff},
{0x310000,0x31ffff},{0x320000,0x32ffff},{0x330000,0x33ffff},{0x340000,0x34ffff},{0x350000,0x35ffff}};
const unsigned char code EWMBuffer_Mima[11]={0xDB,0xCA,0xFE,0x63,0x78,0x12,0x43,0x67,0x35,0x23,0x98};
unsigned char UART_Fe_Seq  = 0;						//帧序号
unsigned char CommonDataBuffer[600]={0};//数据缓存数组
unsigned char FingerPrcBuffer[600]={0};//指纹数据暂存处理数组
unsigned int Exint0Counter = 0;						//外部中断0接受数据计数器
unsigned char Exint0DataBuffer[600] = {0};
unsigned char Uart4FindCartACK[200] = {0};//身份证信息缓存数组
unsigned int Uart4ACKCounter = 0;//身份证数据接收计数器
unsigned char UserCardReadState=0; 
unsigned char Flag_FindCard_500msOver=0;
unsigned int FindCard_TimerCounter=0;


unsigned char SetTime[7]={30,52,22,27,03,04,16};//秒分时日月周年10-08-15 23:59:50   7代表周日
unsigned char TimeReadBuffer[7] = {0x00};
void Exint0_DATA_deal(void);
void Uart2_DATA_deal(void);
void Uart3_DATA_deal(void);
void Uart4_DATA_deal(void);
void Poll_Finger_Module(void);
extern void USART1_Send(unsigned char *dat,unsigned int len);
extern void USART4_Send(unsigned char *dat,unsigned int len);
void Arraycat(unsigned char *Data_Dest,unsigned char *Data_Src,unsigned int DestDataNum,unsigned int SrcDataNum);
unsigned int Fe_data_Organize(unsigned char CMD[],unsigned char Data[],unsigned int DataNum,bit OrgType);
char Muti_Read_Byte(void);
void Muti_Send_Byte(unsigned char *Data,unsigned int len);
void Reply_State(unsigned char CMD[],unsigned char State);
void Buzzer_Beep(unsigned char TimeFlag);
UCHAR FLASH_ByteRead(FLADDR addr);
void FLASH_ByteWrite(FLADDR addr, UCHAR dat);
void FLASH_PageErase(FLADDR addr);
UCHAR sequential_write_flash_in_one_sector(UINT begin_addr, UINT counter, UCHAR *array);
UCHAR Delete_IdOrErWeiMa_Info(UCHAR *IdData,UCHAR SecletFlag);
unsigned long int Search_IdOrErWeiMa_Info(UCHAR *Data,UCHAR SecletFlag);
unsigned char Store_IdOrErWeiMa_Data(unsigned char *Data,UCHAR SecletFlag);
unsigned long int  SearchFingerStoreNumber(unsigned char FingerStoreNumber[]);
unsigned char Bytencmp(unsigned char *src,unsigned char *dest,unsigned int len);
void btlder(void);
extern void Delay_ms(UINT ms);
extern void Delay_us(UINT ms);
void main(void)
  { UINT i = 0;
//   for(i=0;i<10;i++)
//    { Com2Buf= 0x65;
//	 if((Flag_ErWeiMaReceiveStart==0)&&(Com2Buf==0x65))
//	  {EWMCounter=0;Flag_ErWeiMaReceiveStart=1;}
//	 if(Flag_ErWeiMaReceiveStart==1)
//	  {if(EWMCounter<22)  EWMBuffer1[EWMCounter++] = Com2Buf;
//	   if(EWMCounter>22) {EWMCounter=0;Flag_ErWeiMaReceiveStart=0; }
//	  }
//	  }	 
	IO_Init();
	init_Uart();
	Timer0Init();
	INT_Init();
	System_SetParaInit();
	EA=1;
	Delay_ms(500);
	Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_SetSystemId,System_ID,6,1));	
	while(1)
	{if(Exint0Counter != 0)
	  {Exint0_DATA_deal();}
	 if(EWMCounter >= 21)
	  {Uart2_DATA_deal();}
	 if(BTCounter >= 1)
	  {Uart3_DATA_deal();}
	 if(ResetU980Trigger)
	  {Reset_U980 = 0;
	   Delay_ms(1000);
	   Reset_U980 = 1;
	   ResetU980Trigger=0;
		}
	  Uart4_DATA_deal();
	  Poll_Finger_Module();
	 }	
}

/****************************************************/
/* 定时器0中断入口函数					            */
/* 程序功能：定时器0中断处理函数，必须写成这样的形式*/
/* 入口参数:无										*/
/* 出口参数:无										*/
/****************************************************/
void Timer0_intrupt_function(void) interrupt 1 using 1
{ 
	/*开门动作*/
	if(DoorTrigger==1)
	{Door1_Ctl_0=0;
	 Door1_Ctl_1=0;
	 DoorCounter++;
	 if(DoorCounter>DoorOpenTime)
	  {DoorTrigger=0;DoorCounter=0;Door1_Ctl_0=1;Door1_Ctl_1=1;
	while(1 == SC50x0B_Busy());
	SC50x0B_Out(DOOROPEN);
		}
	 }
	//身份证
	if((Flag_FindCard_500msOver==0)&&(UserCardReadState==STATE_NOCARD))
	 {FindCard_TimerCounter++;
	  if(FindCard_TimerCounter>FindCard_TimerCounterMAX)
	   {Flag_FindCard_500msOver=1;FindCard_TimerCounter=0;}}
 //重置二维码模块
 if(ResetU980Trigger == 0)
 {
	 ResetU980Counter++;
	 if(ResetU980Counter>ResetU980Time)
	 {
		 ResetU980Trigger =1;
		 ResetU980Counter = 0;
	 }
 }
//屏蔽指纹验证后的服务端开闸
 if(ShieldOpenOrClose == 1)
	 {
		 ShieldCMD_OpenOrCloseCounter++;
		 if(ShieldCMD_OpenOrCloseCounter>ShieldCMD_OpenOrCloseTime)
		 {
			 ShieldCMD_OpenOrCloseCounter=0;
			 ShieldOpenOrClose=0;
		 }
	 }
 
}

void com1_isr(void) interrupt 4 using 1
{ 
	unsigned char dat;

if(RI)
{	
	dat = SBUF;
	//SBUF=dat;while(!(SCON&0X02));SCON &=~0X02;
		if(u8RevcLen<600)
			{
			FingerDataBuffer[u8RevcLen]=dat;
			u8RevcLen++;
			}
		else
			{
			u8RevcLen=0;
			}
		RI = 0;
	}
}

void com2_isr(void) interrupt 8 using 1
{ if(S2CON&0x01)  /*接收*/
   { S2CON &=~0x01;
     Com2Buf= S2BUF;
	 if((Flag_ErWeiMaReceiveStart==0)&&(Com2Buf==0x65))
	  {EWMCounter=0;Flag_ErWeiMaReceiveStart=1;}
	 if(Flag_ErWeiMaReceiveStart==1)
	  {if(EWMCounter<22)  EWMBuffer1[EWMCounter++] = Com2Buf;
	   if(EWMCounter>22) {EWMCounter=0;Flag_ErWeiMaReceiveStart=0; }
	  }	 
    }	
}

void com3_isr(void) interrupt 17 using 1
{
		unsigned char dat = 0;
	if(S3CON&0x01)  /*接收*/
{
	dat = S3BUF;
	S3CON &=~0x01;
if(BTCounter<30)
	BluetoothRecBuffer[BTCounter++] = dat;
if(BTCounter>30) {BTCounter=0;}
}								   

}

void com4_isr(void) interrupt 18 using 1
{
		unsigned char dat;
if(S4CON & 0x01)  /*接收*/
{
	dat  = S4BUF;
//	Uart4ACKCounter++;
	S4CON &=~0x01;
	//(UserCardReadState == STATE_NOCARD)||(UserCardReadState == STATE_SELECTCARD)&& 
	if(Uart4ACKCounter<200)
	Uart4FindCartACK[Uart4ACKCounter++] = dat;
	if(Uart4ACKCounter>200)  {Uart4ACKCounter=0;}

}
}

void Exint0() interrupt 0       //INT0中断入口
{
	Exint0DataBuffer[Exint0Counter++] = Muti_Read_Byte();
}

void Exint0_DATA_deal(void){
unsigned int i = 0,j = 0,DataLength = 0,FingerStoreNumber = 0;
unsigned int LastUpdatePacFlag = 0,FE_CS = 0;
unsigned long int DeleAddr = 0;
// unsigned int StoreNum = 0;
static bit FirstState = 0,SecondState = 0;
unsigned char flag = 0, CS = 0,TimeOverCounter = 0,StoreNumBuffer[2] = {0};
unsigned char FingerStoreNumberBuffer[2]={0};
	Delay_ms(100);
//		for(i=0;i<Exint0Counter;i++){
//SBUF = Exint0DataBuffer[i];while(!(SCON&0x02));SCON &=~0x02;
//}
//	SBUF = 0xf7;while(!(SCON&0x02));SCON &=~0x02;
	if((Exint0DataBuffer[0] == FE_START[0])&&(Exint0DataBuffer[1] == FE_START[1])){
		DataLength = Exint0DataBuffer[FE_LENGTH]*0x100+Exint0DataBuffer[FE_LENGTH+1];
		FE_CS=DataLength+FE_DATAADDR;/*指向校验码的位置*/
		for(i=0;i<FE_DATAADDR+DataLength;i++)//计算校验码
		CS+=Exint0DataBuffer[i];
		
			for(i=0,j=0;i<6,j<6;i++,j++){
			if(System_ID[j] == Exint0DataBuffer[SYSTEM_ID+i])															//判断是否是本机编号
				flag++;
	}
//		for(i=0;i<Exint0Counter;i++){
//SBUF = Exint0DataBuffer[i];while(!(SCON&0x02));SCON &=~0x02;
//}
	if(CS == Exint0DataBuffer[FE_CS] && flag == 0x06){//校验码正确
		UART_Fe_Seq = Exint0DataBuffer[FE_CODENUM];								//纪录帧序号
//		for(i=0;i<Exint0Counter;i++){
//SBUF = Exint0DataBuffer[i];while(!(SCON&0x02));SCON &=~0x02;
//}
		//强制开关闸
			if((CMD_OpenOrClose[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_OpenOrClose[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]) && !ShieldOpenOrClose)
			{	
				DoorTrigger = 1;
				Reply_State(CMD_OpenOrClose,1);
				Buzzer_Beep(BuzzerTimeNormal);
			}//进入WIFI串口配置模式
			if((CMD_WifiSerialConfigMode[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_WifiSerialConfigMode[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
			}//心跳包回复
			if((CMD_Heartbeat[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_Heartbeat[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
			}			
			//重启
			if((CMD_ReStart[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_ReStart[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_ReStart,&flag,1,1));
				Delay_ms(1000);
				Buzzer_Beep(BuzzerTimeNormal);
				IAP_CONTR = 0x20;
			}
			//内部开闸
			if((CMD_DoorTrigger[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DoorTrigger[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				DoorTrigger = 1;
			}//查询上板软件版本号
			if((CMD_AskAppVersionNum_Top[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_AskAppVersionNum_Top[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_AskAppVersionNum_Top,AppVersionNum,4,0));
			}//查询下板软件版本号
			if((CMD_AskAppVersionNum_Bottom[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_AskAppVersionNum_Bottom[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
				Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
			}			
			//设置闸机ID号
			if((CMD_SetSystemId[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SetSystemId[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				FLASH_W25Q64_Update (System_SetParaAddr, &Exint0DataBuffer[FE_DATAADDR], sizeof(System_ID));//更新系统ID信息
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_SetSystemId,&Exint0DataBuffer[FE_DATAADDR],6,1));
				Delay_ms(1000);
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_ReStart,&flag,1,1));//重启
				IAP_CONTR = 0x20;
			}//软件升级
			if((CMD_UpdateSystem[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_UpdateSystem[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
					if(Exint0DataBuffer[FE_DATAADDR] == 0x00){
						if(Exint0DataBuffer[FE_DATAADDR+4]==Exint0DataBuffer[FE_DATAADDR+6]){//最后一个升级包
							if(sequential_write_flash_in_one_sector(SysCordAppSaveStartTempAddr+(Exint0DataBuffer[FE_DATAADDR+6]-1)*0x200, 512, &Exint0DataBuffer[FE_DATAADDR+9])){
							FLASH_PageErase(SysCordAppUpdataAddrFlag);
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag, 0x5A);//写升级标志
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag+1, 0xA5);
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag+2, Exint0DataBuffer[FE_DATAADDR+4]);//纪录升级数据包个数
							Reply_State(CMD_UpdateSystem,1);
							Buzzer_Beep(BuzzerTimeNormal);
							Delay_ms(1000);
							IAP_CONTR = 0x20;//重启进入升级
						}
						}else{			
								if(sequential_write_flash_in_one_sector(SysCordAppSaveStartTempAddr+(Exint0DataBuffer[FE_DATAADDR+6]-1)*0x200, 512, &Exint0DataBuffer[FE_DATAADDR+9])){
								Reply_State(CMD_UpdateSystem,1);
								Buzzer_Beep(BuzzerTimeShort);
							}	
					}
				}else if( Exint0DataBuffer[FE_DATAADDR] == 0x01)
					{
						Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);//将升级数据转发给U2（下板）	
					}
		}//删除所有身份证
			if((CMD_DeleteAllID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
					for(DeleAddr=Id_StoreAddrTable[0][0];DeleAddr<Id_StoreAddrTable[10][1];DeleAddr+=0x10000){
					//SBUF = 0x76;while(!(SCON&0x02));SCON &=~0x02;
					sFLASH_64KBEraseSector(DeleAddr);
					//SBUF = 0x77;while(!(SCON&0x02));SCON &=~0x02;
					}
					Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DeleteAllID,&CS,1,1));
			}//删除所有二维码
			if((CMD_DeleteAllErWeiMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllErWeiMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
					for(DeleAddr=ErWeiMa_AddrTable[0][0];DeleAddr<ErWeiMa_AddrTable[15][1];DeleAddr+=0x10000){
					sFLASH_64KBEraseSector(DeleAddr);
					}
					Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DeleteAllErWeiMa,&CS,1,1));
			}
			//删除所有指纹ID
			if((CMD_DeleteAllUserID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllUserID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
					for(DeleAddr=FingerId_AddrTable[0][0];DeleAddr<FingerId_AddrTable[15][1];DeleAddr+=0x10000){
					sFLASH_64KBEraseSector(DeleAddr);
					}
					Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DeleteAllUserID,&CS,1,1));
			}
			//下载身份证
			if((CMD_DownLoadUerID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DownLoadUerID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(Exint0DataBuffer[FE_DATAADDR] == 0x00){
				if(Store_IdOrErWeiMa_Data(&Exint0DataBuffer[FE_DATAADDR+1],1) == 0x01){
				Reply_State(CMD_DownLoadUerID,1);
				}else{
				Reply_State(CMD_DownLoadUerID,0);
				}
			}else if(Exint0DataBuffer[FE_DATAADDR] == 0x01)
				{
					//发给下板子接收储存
					Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
				}
			}
			//删除身份证
			if((CMD_DeleteID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(Exint0DataBuffer[FE_DATAADDR] == 0x00){
				if(Delete_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR+1],1) == 0x01){
				Reply_State(CMD_DeleteID,1);
				}else{
				Reply_State(CMD_DeleteID,0);
				}
			}else if(Exint0DataBuffer[FE_DATAADDR] == 0x01)
				{
			//发给下板子接收删除
					Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
				}
			}
			//下载二维码
			if((CMD_DownLoadErWerMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DownLoadErWerMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(Exint0DataBuffer[FE_DATAADDR] == 0x00){
					if(Store_IdOrErWeiMa_Data(&Exint0DataBuffer[FE_DATAADDR+1],0) == 0x01){
					Reply_State(CMD_DownLoadErWerMa,1);
					}else{
					Reply_State(CMD_DownLoadErWerMa,0);
					}
				}else if(Exint0DataBuffer[FE_DATAADDR] == 0x01)
					{
			//发给下板子接收储存
						Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
					}
			}
			//删除二维码
			if((CMD_DeleteErWeiMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteErWeiMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(Exint0DataBuffer[FE_DATAADDR] == 0x00){
					if(Delete_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR+1],0) == 0x01){
						Reply_State(CMD_DeleteErWeiMa,1);
						}else{
						Reply_State(CMD_DeleteErWeiMa,0);
						}
					}else if(Exint0DataBuffer[FE_DATAADDR] == 0x01)
					{
			//发给下板子接收删除
						Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
					}
			}//指纹同步下载
			if((CMD_DownLoadFingerData[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DownLoadFingerData[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(LD9900MT_DownChar_ReDataProc(&Exint0DataBuffer[FE_DATAADDR+5],&Exint0DataBuffer[FE_DATAADDR+1]))
				{
					Reply_State(CMD_DownLoadFingerData,1);
				}else
				{
					Reply_State(CMD_DownLoadFingerData,0);
				}
			}//指纹单条删除
				if((CMD_DeleteFingerID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteFingerID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				//先找到指纹ID对应的指纹编号，对指纹模块进行单条删除操作，然后清空FLASH中对应的指纹ID和指纹编号对应表。
				FLASH_W25Q64_Read(FingerStoreNumberBuffer,Search_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR] ,0x03)+6,2);//指纹编号读入缓冲区
//				S4BUF = FingerStoreNumberBuffer[0];while(!(S4CON&0x02));S4CON &= ~0x02;
//				S4BUF = FingerStoreNumberBuffer[1];while(!(S4CON&0x02));S4CON &= ~0x02;
				FingerStoreNumber = FingerStoreNumberBuffer[0]*256+ FingerStoreNumberBuffer[1];
				if(LD9900MT_DeleteChar(FingerStoreNumber ,1) == 0x00)//删除指纹模块中的数据
				{
//				S4BUF = 0x88;while(!(S4CON&0x02));S4CON &= ~0x02;
					Delete_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR],0x03);//清空FLASH中对应的指纹ID和指纹编号对应表
					Reply_State(CMD_DeleteFingerID,1);
				}else
				{
					Reply_State(CMD_DeleteFingerID,1);
				}
			}			
			//清空指纹库
				if((CMD_EmptyFingerRecord[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_EmptyFingerRecord[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
							if(LD9900MT_Empty()==0x00){
								for(DeleAddr=FingerId_AddrTable[0][0];DeleAddr<FingerId_AddrTable[15][1];DeleAddr+=0x10000){
									sFLASH_64KBEraseSector(DeleAddr);
									}
								Reply_State(CMD_EmptyFingerRecord,1);
							}else{
								Reply_State(CMD_EmptyFingerRecord,0);
							}
				}
				//第一次录取指纹		
				if((CMD_FirstRecordFinger[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_FirstRecordFinger[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
					{
						//SBUF=0xab;while(!(SCON&0X02));SCON &=~0X02;Delay_ms(100);
						Delay_ms(100);		
						TimeOverCounter = 0;
					while(1 == SC50x0B_Busy());
					SC50x0B_Out(PUTFINGER);
					while(LD9900MT_GetImage(600) != 0x00 & TimeOverCounter < 10)
					{						//一直等待获取指纹图像成功  最多尝试10次
						FirstState = 0;																					//置状态位为0
						TimeOverCounter++;
						Buzzer_Beep(BuzzerTimeNormal);
					}
					if(TimeOverCounter < 10)
					{					
							if(LD9900MT_GenChar(0x01,600) == 0x00){											//生成特征储存在 CharBuffer 1							
							FirstState = 1;
							Reply_State(CMD_FirstRecordFinger,1);
							Buzzer_Beep(BuzzerTimeLong);
							while(1 == SC50x0B_Busy());
							SC50x0B_Out(FINGERPRINTSUCCESS);
						}else
						{
							FirstState = 0;						
							Reply_State(CMD_FirstRecordFinger,0);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							while(1 == SC50x0B_Busy());
							SC50x0B_Out(FINGERWRONG);
						}
					}else
						{
							FirstState = 0;																			
							Reply_State(CMD_FirstRecordFinger,0);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);	
							while(1 == SC50x0B_Busy());
							SC50x0B_Out(INVALIDFINGER);
						}

				}
					/*第二次录取指纹*/
				if((CMD_SecondRecordFinger[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SecondRecordFinger[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))/*第一次录取指纹*/
				{
						Delay_ms(100);
						TimeOverCounter = 0;//计数器清零
					while(1 == SC50x0B_Busy());
					SC50x0B_Out(PUTFINGERAGAIN);
						while(LD9900MT_GetImage(600) != 0x00 & TimeOverCounter < 10){						//一直等待获取指纹图像成功	 最多尝试10次
						SecondState = 0;																				//置状态位为0
						TimeOverCounter++;	
						Buzzer_Beep(BuzzerTimeNormal);
						}
						if(TimeOverCounter < 10){
						Buzzer_Beep(BuzzerTimeLong);
						SecondState = 1;
						if(LD9900MT_GenChar(0x02,600) == 0x00){									//生成特征储存在 CharBuffer 2	
						if(FirstState == 1 & SecondState == 1 & LD9900MT_Search(0x02,0x0000,1000,FingerStoreNumberBuffer) != 0x00){
						FirstState = 0;	
						SecondState = 0;
								if(LD9900MT_CreateTemplate() == 0x00){									//生成模板
										if(LD9900MT_Store(0x02,LD9900MT_SearchEmpty()) == 0x00){						//生成模板后CharBuffer1和CharBuffer2中的内容是一样的，储存模板到库序号1
											//FLASH_W25Q64_Write (Search_IdOrErWeiMa_Info(UserId,0x03)+6, FingerStoreNumber, 2);
//											ItemCounter = FLASH_W25Q64_ByteRead(FingerId_AddrTable[0][0])*256+FLASH_W25Q64_ByteRead(FingerId_AddrTable[0][0]+1);//读取该区域储存的信息总量
//											FLASH_W25Q64_Update (FingerId_AddrTable[0][0]+(ItemCounter-1)*10+36, FingerStoreNumber, 2);//更新数量信息
//									FLASH_W25Q64_Read(CommonDataBuffer,FingerId_AddrTable[0][0],600);//读入缓冲区
//			 //S4BUF = 0xa2;while(!(S4CON&0x02));S4CON &= ~0x02;
//			for(i=0;i<600;i++){
//		 S4BUF = CommonDataBuffer[i];while(!(S4CON&0x02));S4CON &= ~0x02;_nop_();_nop_();
//		}
											
											LD9900MT_UpChar(0x01);				
											LD9900MT_UpCharDataProc(CMD_SecondRecordFinger);							
											Delay_ms(1000);
											while(1 == SC50x0B_Busy());
											SC50x0B_Out(FINGERPRINTSUCCESS);
										}
								}
							else{								
							Reply_State(CMD_SecondRecordFinger,0);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							while(1 == SC50x0B_Busy());
							SC50x0B_Out(INVALIDFINGER);
							}
						}
						else{						
							Reply_State(CMD_SecondRecordFinger,0);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);	
							while(1 == SC50x0B_Busy());
							SC50x0B_Out(INVALIDFINGER);
						}
						}
						else{
								
							Reply_State(CMD_SecondRecordFinger,0);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							while(1 == SC50x0B_Busy());
							SC50x0B_Out(FINGERWRONG);
						}
						}
						else{
									SecondState = 0;								
									Reply_State(CMD_SecondRecordFinger,0);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							Buzzer_Beep(BuzzerTimeShort);
							while(1 == SC50x0B_Busy());
							SC50x0B_Out(INVALIDFINGER);
						}
						}
	}
}

	memset(Exint0DataBuffer,0x00,600);
	Exint0Counter = 0;

}

//回复上位机
void Reply_State(unsigned char CMD[],unsigned char  State){
unsigned int Reply_Num = 0;
unsigned char const ReplySucc = 0x31;		
unsigned char const ReplyFail = 0x30;
//				SBUF = 0xaf;while(!(SCON&0x02));SCON &= ~0x02;
	if(State == 1){
		Reply_Num =  Fe_data_Organize(CMD,&ReplySucc,1,0);
		Muti_Send_Byte(CommonDataBuffer,Reply_Num);
}else if(State == 0){
		Reply_Num =  Fe_data_Organize(CMD,&ReplyFail,1,0);
		Muti_Send_Byte(CommonDataBuffer,Reply_Num);
}
	memset(CommonDataBuffer,0x00,600);
}

void Buzzer_Beep(unsigned char TimeFlag){
	unsigned int Reply_Num = 0;
	Delay_ms(100);
			Reply_Num =  Fe_data_Organize(CMD_Buzzer,&TimeFlag,1,1);
			Muti_Send_Byte(CommonDataBuffer,Reply_Num);
	memset(CommonDataBuffer,0x00,600);
}




//储存信息
unsigned char Store_IdOrErWeiMa_Data(unsigned char *Data,UCHAR SecletFlag){
UINT 	i=0;
UINT 	ItemCounter=0;//该区域的用户信息数量
UCHAR	ItemCounterBuffer[2] = {0},Cursor = 0;
UCHAR TempIdBuffer[20]={0};
UCHAR StoreErWeiMaBuffer[10]={0};
UCHAR StoreFingerIdBuffer[10]={0};
UCHAR AttachData[6]={0xff,0xff,0xff,0xff,0xff,0xff};
//身份证
if(SecletFlag == 0x01){
for(i=0;i<16;i++)
	{
		TempIdBuffer[i] = *Data++;	//暂存第一条信息
	}
	Arraycat(TempIdBuffer,AttachData,16,4);	//添加附加信息
	Cursor = TempIdBuffer[8];
	if(Search_IdOrErWeiMa_Info(TempIdBuffer,1) == 0){//信息不存在则添加
		ItemCounter = FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0]+1);//读取该区域储存的信息总量
		 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
	//根据身份证校验码10种分类储存不同的区域
		if(ItemCounter == 0xffff)//初始化工作
		ItemCounter = 0;

		FLASH_W25Q64_Write ( Id_StoreAddrTable[Cursor&= 0x0f][0]+ItemCounter*20+60,TempIdBuffer, 20);//在尾部追加信息 每个区域前60字节预留

		ItemCounter ++;//数量增加1
	//	SBUF = 0xa6;while(!(SCON&0x02));SCON &= ~0x02;
		ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
		ItemCounterBuffer[1] = ItemCounter & 0x00ff;
		FLASH_W25Q64_Update (Id_StoreAddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//更新数量信息
//				SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
//				FLASH_W25Q64_Read(CommonDataBuffer,Id_StoreAddrTable[Cursor&= 0x0f][0],600);//读入缓冲区
//			for(i=0;i<600;i++){
//			 SBUF = CommonDataBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();_nop_();
//			}
//	 SBUF = 0xa7;while(!(SCON&0x02));SCON &= ~0x02;
	}else{
		return 1;
	}
	return 1;
}else if(SecletFlag == 0x00)
//二维码
{
for(i=0;i<6;i++)
	{
		StoreErWeiMaBuffer[i] = *Data++;	//暂存第一条信息
	}
		Arraycat(StoreErWeiMaBuffer,AttachData,6,4);	//添加附加信息
		if(Search_IdOrErWeiMa_Info(StoreErWeiMaBuffer,0x00) == 0){//信息不存在则添加
		Cursor = StoreErWeiMaBuffer[5] ;
//		for(i=0;i<6;i++){
//		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}

	ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0]+1);//读取该区域储存的信息总量
//	 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
//根据校验码16种分类储存不同的区域
	if(ItemCounter == 0xffff)//初始化工作
	ItemCounter = 0;

					for(i=0;i<10;i++){
		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
			}
					
	FLASH_W25Q64_Write (ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30, StoreErWeiMaBuffer, 10);//在尾部追加信息 每个区域前30字节预留
	ItemCounter ++;//数量增加1
//	SBUF = 0xa6;while(!(SCON&0x02));SCON &= ~0x02;
	ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
	ItemCounterBuffer[1] = ItemCounter & 0x00ff;
	FLASH_W25Q64_Update (ErWeiMa_AddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//更新数量信息
//	 SBUF = 0xa7;while(!(SCON&0x02));SCON &= ~0x02;
	//	FLASH_W25Q64_Write ( ErWeiMa_AddrTable[1][0]+5*10+30,erweima, 10);
//	memset(CommonDataBuffer,0x00,600);
		FLASH_W25Q64_Read(CommonDataBuffer,ErWeiMa_AddrTable[Cursor&= 0x0f][0],600);//读入缓冲区
			 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
		for(i=0;i<600;i++){
		 SBUF = CommonDataBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();_nop_();
		}
		return 1;
	}else{
		return 1;
	}
	return 1;
}//用户ID
else if(SecletFlag == 0x03)
{
for(i=0;i<4;i++)
	{
		StoreFingerIdBuffer[i] = *Data++;	//暂存第一条信息
	}
		if(Search_IdOrErWeiMa_Info(StoreFingerIdBuffer,0x03) == 0){//信息不存在则添加
		Arraycat(StoreFingerIdBuffer,AttachData,4,6);	//添加附加信息
		Cursor = StoreFingerIdBuffer[0]+StoreFingerIdBuffer[1]+StoreFingerIdBuffer[2]+StoreFingerIdBuffer[3] ;
//		for(i=0;i<6;i++){
//		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}

	ItemCounter = FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor&= 0x0f][0]+1);//读取该区域储存的信息总量
//	 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
//根据校验码16种分类储存不同的区域
	if(ItemCounter == 0xffff)//初始化工作
	ItemCounter = 0;

//					for(i=0;i<10;i++){
//		S4BUF = StoreFingerIdBuffer[i];while(!(S4CON&0x02));S4CON &= ~0x02;_nop_();
//			}
					
	FLASH_W25Q64_Write (FingerId_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30, StoreFingerIdBuffer, 10);//在尾部追加信息 每个区域前30字节预留
	ItemCounter ++;//数量增加1
//	SBUF = 0xa6;while(!(SCON&0x02));SCON &= ~0x02;
	ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
	ItemCounterBuffer[1] = ItemCounter & 0x00ff;
	FLASH_W25Q64_Update (FingerId_AddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//更新数量信息
//	 SBUF = 0xa7;while(!(SCON&0x02));SCON &= ~0x02;
//	memset(CommonDataBuffer,0x00,600);
//		FLASH_W25Q64_Read(CommonDataBuffer,FingerId_AddrTable[Cursor&= 0x0f][0],600);//读入缓冲区
			 //S4BUF = 0xa2;while(!(S4CON&0x02));S4CON &= ~0x02;
//		for(i=0;i<600;i++){
//		 S4BUF = CommonDataBuffer[i];while(!(S4CON&0x02));S4CON &= ~0x02;_nop_();_nop_();
//		}
		return 1;
	}else{
		return 1;
	}
	return 1;
}

return 1;
}


//搜索身份证信息 返回所在地址或者0（没有搜索到信息）
unsigned long int Search_IdOrErWeiMa_Info(UCHAR *Data,UCHAR SecletFlag){

UCHAR TempIdData[9],*Result_Addr,Cursor = 0;
UCHAR TempErWeiMaBuffer[6]={0};
UCHAR TempFingerIdBuffer[4]={0};
ULONG CurrentAddr = 0;
UINT i=0,ItemCounter=0,Comparei = 0;
	if( SecletFlag == 0x01){
		for(i=0;i<9;i++){
		TempIdData[i] = *Data++;
		}
		Cursor = TempIdData[8];
//			for(i=0;i<9;i++){
//		SBUF = TempIdData[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}
		ItemCounter = FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor &= 0x0f][0])*256+FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor &= 0x0f][0]+1);//读取该区域储存的信息总量
			if(ItemCounter == 0xffff)
				ItemCounter = 0;
		for(CurrentAddr = Id_StoreAddrTable[Cursor&= 0x0f][0];CurrentAddr<Id_StoreAddrTable[Cursor&= 0x0f][0]+ItemCounter*20+60;CurrentAddr+=600){
				 FLASH_W25Q64_Read(CommonDataBuffer,CurrentAddr,600);//读入缓冲区
				 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
				 Result_Addr = CommonDataBuffer;
					for(i=0;i<600;i+=20){
						if(Bytencmp(Result_Addr,TempIdData,9) != 0){
						Result_Addr+=20;
						Comparei++;
						}
					else
					{
						memset(CommonDataBuffer,0x00,600);
						return Id_StoreAddrTable[Cursor&= 0x0f][0]+Comparei*20;
					}
				}
			}
	}else if(SecletFlag == 0x00){
	for(i=0;i<6;i++){
		TempErWeiMaBuffer[i] = *Data++;
	}
		for(i=0;i<6;i++){
		SBUF = TempErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
			}
		Cursor = TempErWeiMaBuffer[5];
		ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor &= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor &= 0x0f][0]+1);//读取该区域储存的信息总量
			if(ItemCounter == 0xffff)
				ItemCounter = 0;
			for(CurrentAddr = ErWeiMa_AddrTable[Cursor&= 0x0f][0];CurrentAddr<ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30;CurrentAddr+=600){
				 FLASH_W25Q64_Read(CommonDataBuffer,CurrentAddr,600);//读入缓冲区
				 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
				 Result_Addr = CommonDataBuffer;
					for(i=0;i<600;i+=10){
						if(Bytencmp(Result_Addr,TempErWeiMaBuffer,6) != 0){
						Result_Addr+=10;
						Comparei++;//纪录次数
						}
					else
					{
						memset(CommonDataBuffer,0x00,600);
						return ErWeiMa_AddrTable[Cursor&= 0x0f][0]+Comparei*10;
					}
				}
			}
		}else if(SecletFlag == 0x03){
			for(i=0;i<4;i++){
		TempFingerIdBuffer[i] = *Data++;
	}
//		for(i=0;i<4;i++){
//		S4BUF = TempFingerIdBuffer[i];while(!(S4CON&0x02));S4CON &= ~0x02;_nop_();
//			}
				Cursor = TempFingerIdBuffer[0]+TempFingerIdBuffer[1]+TempFingerIdBuffer[2]+TempFingerIdBuffer[3] ;
		ItemCounter = FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor &= 0x0f][0])*256+FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor &= 0x0f][0]+1);//读取该区域储存的信息总量
			if(ItemCounter == 0xffff)
				ItemCounter = 0;
			for(CurrentAddr = FingerId_AddrTable[Cursor&= 0x0f][0];CurrentAddr<FingerId_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30;CurrentAddr+=600){
				memset(CommonDataBuffer,0x00,600);
				FLASH_W25Q64_Read(CommonDataBuffer,CurrentAddr,600);//读入缓冲区
//						for(i=0;i<600;i++){
//		 S4BUF = CommonDataBuffer[i];while(!(S4CON&0x02));S4CON &= ~0x02;_nop_();_nop_();
//		}
				 S4BUF = 0xa2;while(!(S4CON&0x02));S4CON &= ~0x02;
				 Result_Addr = CommonDataBuffer;
					for(i=0;i<600;i+=10){
//						S4BUF = i>>8;while(!(S4CON&0x02));S4CON &= ~0x02;
//						S4BUF = i;while(!(S4CON&0x02));S4CON &= ~0x02;
//						S4BUF = strncmp(Result_Addr,TempFingerIdBuffer,4)>>8;while(!(S4CON&0x02));S4CON &= ~0x02;
//						S4BUF = strncmp(Result_Addr,TempFingerIdBuffer,4);while(!(S4CON&0x02));S4CON &= ~0x02;
//						S4BUF = *(	Result_Addr+=30);while(!(S4CON&0x02));S4CON &= ~0x02;
//						S4BUF = *(	Result_Addr++);while(!(S4CON&0x02));S4CON &= ~0x02;
//						S4BUF = *(	Result_Addr++);while(!(S4CON&0x02));S4CON &= ~0x02;
//						S4BUF = *(	Result_Addr++);while(!(S4CON&0x02));S4CON &= ~0x02;
						if(Bytencmp(Result_Addr,TempFingerIdBuffer,4) != 0){
						Result_Addr+=10;
						Comparei++;//纪录次数
//						S4BUF = Comparei>>8;while(!(S4CON&0x02));S4CON &= ~0x02;
//						S4BUF = Comparei;while(!(S4CON&0x02));S4CON &= ~0x02;
//							S4BUF = Result_Addr>>16;while(!(S4CON&0x02));S4CON &= ~0x02;
//							S4BUF = Result_Addr>>8;while(!(S4CON&0x02));S4CON &= ~0x02;
//							S4BUF = Result_Addr;while(!(S4CON&0x02));S4CON &= ~0x02;
//							S4BUF = Comparei>>8;while(!(S4CON&0x02));S4CON &= ~0x02;
//								S4BUF = Comparei;while(!(S4CON&0x02));S4CON &= ~0x02;
						}
					else
					{
						memset(CommonDataBuffer,0x00,600);
						return FingerId_AddrTable[Cursor&= 0x0f][0]+Comparei*10;
					}
				}
			}
		
		}
memset(CommonDataBuffer,0x00,600);
return 0;
}

/***********************************************
函数名称：Bytencmp
函数功能：比对
入口函数：*src  *dest  len
出后口数：无
***********************************************/
unsigned char Bytencmp(unsigned char *src,unsigned char *dest,unsigned int len)
{
	unsigned int i=0;
	for(i=0;i<len;i++)
	{
//		S4BUF = *(src+i);while(!(S4CON&0x02));S4CON &= ~0x02;
//		S4BUF = 0xaa;while(!(S4CON&0x02));S4CON &= ~0x02;
//		S4BUF = *(dest+i);while(!(S4CON&0x02));S4CON &= ~0x02;
		if(*(src+i) != *(dest+i)){
		return 1;
		}else
		{
		;
		}
	}
		//S4BUF = i;while(!(S4CON&0x02));S4CON &= ~0x02;
	if(i==len)
{	
//	S4BUF = 0xaa;while(!(S4CON&0x02));S4CON &= ~0x02;
	return 0;
}
		return 1;
}

//删除身份证信息  返回0（删除失败）或者1（删除成功）
UCHAR Delete_IdOrErWeiMa_Info(UCHAR *IdData,UCHAR SecletFlag){
	UINT 	ItemCounter=0;//该区域的用户信息数量
	UCHAR	ItemCounterBuffer[2] = {0},Cursor = 0;
	UCHAR DelErWeiMaBuffer[6]={0};
	UCHAR DelFingerIdBuffer[4]={0};
	ULONG TempAddr = 0;
	UCHAR TempIdData[9] ={0};
	UCHAR TempFlashBuffer[20]= {0};
	UINT i;
		if(SecletFlag == 0x01){
		for(i=0;i<9;i++)
		TempIdData[i] = *IdData++;
			Cursor = TempIdData[8];
		ItemCounter = FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0]+1);//读取该区域储存的信息总量
		TempAddr = Search_IdOrErWeiMa_Info(TempIdData,1);
		if(TempAddr == 0){
		return 1;
		}else{
			FLASH_W25Q64_Read(TempFlashBuffer,Id_StoreAddrTable[Cursor&= 0x0f][0]+ItemCounter*20+60,20);
			FLASH_W25Q64_Update (TempAddr, TempFlashBuffer, 20);//将尾部的一条信息追加到删除的位置
			memset(TempFlashBuffer,0xff,20);
			FLASH_W25Q64_Update (Id_StoreAddrTable[Cursor &= 0x0f][0]+ItemCounter*20+60,TempFlashBuffer , 20);//删除末尾的一条信息
			ItemCounter--;//数量减1
			ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
			ItemCounterBuffer[1] = ItemCounter & 0x00ff;
			FLASH_W25Q64_Update (Id_StoreAddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//更新数量信息
			return 1;
		}
	}else if(SecletFlag == 0x00){
		for(i=0;i<6;i++)
		DelErWeiMaBuffer[i] = *IdData++;
		Cursor  = DelErWeiMaBuffer[5];
		ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0]+1);//读取该区域储存的信息总量
		TempAddr = Search_IdOrErWeiMa_Info(DelErWeiMaBuffer,0);
		if(TempAddr == 0){
		return 1;
		}else{
			FLASH_W25Q64_Read(TempFlashBuffer,ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30,10);
			FLASH_W25Q64_Update (TempAddr, TempFlashBuffer, 10);//将尾部的一条信息追加到删除的位置
			memset(TempFlashBuffer,0xff,10);
			FLASH_W25Q64_Update ((ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30),TempFlashBuffer , 10);//删除末尾的一条信息
			ItemCounter--;//数量减1
			ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
			ItemCounterBuffer[1] = ItemCounter & 0x00ff;
			FLASH_W25Q64_Update (ErWeiMa_AddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//更新数量信息
			return 1;
		}
	}else if(SecletFlag == 0x03){
		for(i=0;i<4;i++)
		DelFingerIdBuffer[i] = *IdData++;
		Cursor = DelFingerIdBuffer[0]+DelFingerIdBuffer[1]+DelFingerIdBuffer[2]+DelFingerIdBuffer[3] ;
		ItemCounter = FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor&= 0x0f][0]+1);//读取该区域储存的信息总量
		TempAddr = Search_IdOrErWeiMa_Info(DelFingerIdBuffer,3);
		if(TempAddr == 0){
		return 1;
		}else{
			FLASH_W25Q64_Read(TempFlashBuffer,FingerId_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30,10);
			FLASH_W25Q64_Update (TempAddr, TempFlashBuffer, 10);//将尾部的一条信息追加到删除的位置
			memset(TempFlashBuffer,0xff,10);
			FLASH_W25Q64_Update ((FingerId_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30),TempFlashBuffer , 10);//删除末尾的一条信息
			ItemCounter--;//数量减1
			ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
			ItemCounterBuffer[1] = ItemCounter & 0x00ff;
			FLASH_W25Q64_Update (FingerId_AddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//更新数量信息
			return 1;
		}
	}
	return 0;
}



/*****************************************************************************/
/*身份证收发处理程序*/
/*程序组当收到有效帧后进行下一步处理程序                                     */
/*入口参数：无												 */
/*   返回值： 整型，为当前DataSortBuffer数组有效数据的个数                    */
/*帧结构为 帧头 数据长度 目标地址 序列号 命令 数据 校验和 帧尾               */
/*****************************************************************************/
void Uart4_DATA_deal(void)
{ 
//	unsigned char FindACKBuffer[20] = {0};
//	const unsigned char code FIND_CARD_FAILURE_ACK[12]={0xaa, 0xaa, 0xaa, 0x96, 0x69,0x00,0x04, 0x00,0x00,0x80,0x84,0x00};
//	const unsigned char code FIND_CARD_SUCCEES_ACK[16]={0xaa, 0xaa, 0xaa, 0x96, 0x69,0x00,0x08, 0x00,0x00,0x9F,0x00,0x00,0x00,0x00,0x97,0x00};
	unsigned int i = 0,j = 0;
	unsigned char IdNumberAscii[18] = {0};
	unsigned char Id_BCD_Buffer[9] = 0;
		if((Flag_FindCard_500msOver == 1)&& (	UserCardReadState == STATE_NOCARD)){
			Flag_FindCard_500msOver=0;
		USART4_Send(FIND_CARD,sizeof(FIND_CARD));
		Delay_ms(100);
				if( Uart4FindCartACK[6] == 0x04){//寻卡失败
	//		SBUF=Uart4ACKCounter>>8;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
	//		SBUF=Uart4ACKCounter;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
//			for(i=0;i<Uart4ACKCounter;i++){
//				SBUF=Uart4FindCartACK[i];
//				while(!(SCON&0x02));
//				SCON &=~0x02;
//				_nop_();
//			}
		}else if(Uart4FindCartACK[6] == 0x08){//寻卡成功
			UserCardReadState = STATE_SELECTCARD;
//				SBUF=0x35;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
//				for(i=0;i<Uart4ACKCounter;i++){
//				SBUF=Uart4FindCartACK[i];
//				while(!(SCON&0x02));
//				SCON &=~0x02;
//				_nop_();
//			}
//				SBUF=0x36;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
				Uart4ACKCounter = 0;
//				_nop_();	_nop_();
				USART4_Send(SELECT_CARD,sizeof(SELECT_CARD));//选卡
//							SBUF=0x37;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
					Delay_ms(100);
//							SBUF=0x38;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
		}
	}
			if(UserCardReadState == STATE_SELECTCARD){
//				for(i=0;i<Uart4ACKCounter;i++){
//				SBUF=Uart4FindCartACK[i];while(!(SCON&0x02));SCON &=~0x02;
//			}
//				SBUF=0x88;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
					if( Uart4FindCartACK[6] == 0x04){//选卡失败

//						SBUF=0x23;while(!(SCON&0x02));SCON &=~0x02;
						UserCardReadState = STATE_NOCARD;
						Flag_FindCard_500msOver=0;
//						SBUF=0x59;while(!(SCON&0x02));SCON &=~0x02;
		//		SBUF=Uart4ACKCounter>>8;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
		//		SBUF=Uart4ACKCounter;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
//				for(i=0;i<Uart4ACKCounter;i++){
//					SBUF=Uart4FindCartACK[i];
//					while(!(SCON&0x02));
//					SCON &=~0x02;
//					_nop_();
//				}
			}else if(Uart4FindCartACK[6] == 0x0c){//选卡成功
			UserCardReadState = STATE_READCARD;
//				SBUF=0x05;while(!(SCON&0x02));SCON &=~0x02;
//				for(i=0;i<Uart4ACKCounter;i++){
//				SBUF=Uart4FindCartACK[i];while(!(SCON&0x02));SCON &=~0x02;

//			}
//					SBUF=0xff;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
				Uart4ACKCounter = 0;
				USART4_Send(READ_CARD,sizeof(READ_CARD));//读卡
					
		}
	}
			
		if(UserCardReadState == STATE_READCARD){
//				for(i=0;i<Uart4ACKCounter;i++){
//				SBUF=Uart4FindCartACK[i];while(!(SCON&0x02));SCON &=~0x02;
//			}
//				SBUF=0x19;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
			Delay_ms(500);
					if( Uart4FindCartACK[6] == 0x04){//读卡失败
						UserCardReadState = STATE_NOCARD;
						Flag_FindCard_500msOver=0;
				
//						SBUF=0x20;while(!(SCON&0x02));SCON &=~0x02;	
//						SBUF=0x59;while(!(SCON&0x02));SCON &=~0x02;
		//		SBUF=Uart4ACKCounter>>8;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
		//		SBUF=Uart4ACKCounter;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
//				for(i=0;i<Uart4ACKCounter;i++){
//					SBUF=Uart4FindCartACK[i];
//					while(!(SCON&0x02));
//					SCON &=~0x02;
//					_nop_();
//				}
			}else if(Uart4FindCartACK[6] == 0x08){//读卡成功
				
//				SBUF=0x05;while(!(SCON&0x02));SCON &=~0x02;
				for(j=0,i=136;j<18,i<172;j++,i+=2){
				IdNumberAscii[j]=Uart4FindCartACK[i];
			}
					for(i=0;i<8;i++){
					 Id_BCD_Buffer[i] = (IdNumberAscii[i*2]-0x30)<<4;
					 Id_BCD_Buffer[i] |= (IdNumberAscii[i*2+1]-0x30);
					}
				if(IdNumberAscii[17] == 0x58){//末尾为X单独处理转化为BCD 1010即十进制10  十六进制为0x0a
				Id_BCD_Buffer[8]  = (IdNumberAscii[16] - 0x30)<<4;
				Id_BCD_Buffer[8] |= 0x0a;
			}else{
				Id_BCD_Buffer[8]  = (IdNumberAscii[16] - 0x30)<<4;
				Id_BCD_Buffer[8] |= (IdNumberAscii[17] - 0x30);
			}
			
//			for(j=0;j<9;j++){
//			SBUF=Id_BCD_Buffer[j];while(!(SCON&0x02));SCON &=~0x02;
//			}
			Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_UploadUerID,Id_BCD_Buffer,9,0));//流水功能上传身份证
			if(Search_IdOrErWeiMa_Info(Id_BCD_Buffer,1)!=0){
				DoorTrigger = 1;
				Buzzer_Beep(BuzzerTimeNormal);
			}else {		
			Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_SearchUerID,Id_BCD_Buffer,9,1));//发给下板搜索游客信息
			}

						UserCardReadState = STATE_NOCARD;
						Flag_FindCard_500msOver=0;
		}
		
		}
//	SBUF=0x39;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
	Uart4ACKCounter = 0;
}











/***********************************************
函数名称：Op_Finger_Module
函数功能：轮询指纹模块实现脱机认证
入口函数：无
出后口数：无
***********************************************/
void Poll_Finger_Module(void){
unsigned char cnt = 0;
bit State = 0;
unsigned char FingerStoreNumber[2]={0};
unsigned char TempFingerId[4]={0};
//	P41 = 1;
	if(!LD9900MT_TOUCH_O)
{
	
if( LD9900MT_GetImage(550) != 0x00 & cnt < 1 )
{											//一直等待获取指纹图像成功
	State = 0;																			//置状态位为0
	cnt++;			
}
if(cnt < 1)
	{
//		SBUF = 0x08;while(!(SCON&0x02));SCON &= ~0x02;
	if(LD9900MT_GenChar(0x01,550) == 0x00)											//生成特征储存在 CharBuffer 1
		State = 1;
	else
		State = 0;
}
else
	{
	State = 0;
}
cnt = 0;
if(State == 1)
{
//	SBUF = 0x09;while(!(SCON&0x02));SCON &= ~0x02;
if(LD9900MT_Search(0x01,0x0000,1000,FingerStoreNumber) == 0x00){
	ShieldOpenOrClose=1;/*屏蔽强制开闸，避免二次开闸*/
	//Search_IdOrErWeiMa_Info(FingerStoreNumber,0x03)+6
	//FLASH_W25Q64_Read(TempFingerId,SearchFingerStoreNumber(FingerStoreNumber),4);
	//Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_FingerUploadStreamInfo,&TempFingerId,4,0));//流水功能指纹 此句必须放在蜂鸣器前面，否则出错
	DoorTrigger = 1;
	LD9900MT_UpChar(0x01);				
	LD9900MT_UpCharDataProc(CMD_FingerUploadStreamInfo);
	Delay_ms(200);
	Buzzer_Beep(BuzzerTimeNormal);
	
}
}
else	;
//SBUF = 0x10;while(!(SCON&0x02));SCON &= ~0x02;
//P41 = 0;
}
}

unsigned long int  SearchFingerStoreNumber(unsigned char FingerStoreNumber[]){
unsigned long int i = 0;
unsigned char j = 0;
unsigned char TempBuffer[2]={0};
for(j=0;j<16;j++){
		for(i=FingerId_AddrTable[j][0]+30;i<FingerId_AddrTable[j][1];i+=10){
			FLASH_W25Q64_Read(TempBuffer,i+6,2);//读入缓冲区
			if( TempBuffer[0]!=0xff && TempBuffer[1]!=0xff)
				{
				if(FingerStoreNumber[0]==TempBuffer[0] && FingerStoreNumber[1]==TempBuffer[1] )
					{
					return i;
						}else{					
						break;					
						}
			}else
				{
					break;
				}
		}
}
return 0;
}


void Uart2_DATA_deal(void)
  {
	unsigned char CS = 0;
	unsigned int i;
	unsigned char ErWeiMaBCDBuffer[6] = {0};
	/*处理数据接收到的加密二维码数据，例加密数据为：eaf2c850402470500c1aa8*/
	//EWMBuffer1[0]='e';EWMBuffer1[1]='a';EWMBuffer1[2]='f';EWMBuffer1[3]='2';EWMBuffer1[4]='c';EWMBuffer1[5]='8';
	//EWMBuffer1[6]='5';EWMBuffer1[7]='0';EWMBuffer1[8]='4';EWMBuffer1[9]='0';EWMBuffer1[10]='2';EWMBuffer1[11]='4';
	//EWMBuffer1[12]='7';EWMBuffer1[13]='0';EWMBuffer1[14]='5';EWMBuffer1[15]='0';EWMBuffer1[16]='0';EWMBuffer1[17]='c';
	//EWMBuffer1[18]='1';EWMBuffer1[19]='a';EWMBuffer1[20]='a';EWMBuffer1[21]='8';
	for(i=0;i<22;i++)/*将接收到的数据转换为十六进制数据*/
	  {SBUF=EWMBuffer1[i];while(!(SCON&0X02));SCON &=~0X02;
	   if(EWMBuffer1[i]>0x60) {EWMBuffer1[i]= EWMBuffer1[i]-0x57;}
	   else {EWMBuffer1[i]= EWMBuffer1[i]-0x30;}
	    }
	for(i=0;i<11;i++)/*合成数据并解密*/
	  {EWMBuffer[i]= EWMBuffer1[2*i]*16+EWMBuffer1[2*i+1];/*将处理过的数据合成十六进制数据*/
	   EWMBuffer[i]=EWMBuffer[i]^EWMBuffer_Mima[i];}/*解密数据*/
	for(i=0;i<11;i++) CS+=EWMBuffer[i];

//函数功能:对数组进行拼接
//入口函数: Data_Dest			原数组
//				 Data_Src				要添加的数组
//				 DestDataNum  	原数组的数量
//				 SrcDataNum 		要添加的数量
//出口函数:无
    CS &=0x0F;
    Arraycat(EWMBuffer,&CS,11,1);
	for(i=0;i<11;i++)
	{SBUF = EWMBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;}
	for(i=0;i<5;i++)
	{ErWeiMaBCDBuffer[i] = (EWMBuffer[i*2]-0x30)<<4;
     ErWeiMaBCDBuffer[i] |= (EWMBuffer[i*2+1]-0x30);}
	ErWeiMaBCDBuffer[5] = CS;
	ErWeiMaBCDBuffer[5] |= (EWMBuffer[10]-0x30)<<4;


//			for(i=0;i<6;i++){
//	 SBUF = ErWeiMaBCDBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;
//	}
	Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_UploadErWeiMa,ErWeiMaBCDBuffer,6,0));//流水功能上传二维码
	if(Search_IdOrErWeiMa_Info(ErWeiMaBCDBuffer,0) != 0){
					DoorTrigger = 1;
					Buzzer_Beep(BuzzerTimeNormal);
	}else {		
	Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_SearchErWerMa,ErWeiMaBCDBuffer,6,1));//发给下板搜索游客信息
	}
						EWMCounter = 0;Flag_ErWeiMaReceiveStart=0;
}


/***********************************************
函数名称：Uart3_DATA_deal
函数功能：蓝牙设置处理函数
入口函数：无
出后口数：无
***********************************************/

void Uart3_DATA_deal(void){
unsigned char DatBuffer = 0x00;
	Delay_ms(10);
	
 if((CMD_nReloadWifi[0] ==  BluetoothRecBuffer[0]) && (CMD_nReloadWifi[1] ==  BluetoothRecBuffer[1])){
	Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_nReloadWifi,&DatBuffer,1,1));
 }
else if((CMD_SetSystemId[0] ==  BluetoothRecBuffer[0]) && (CMD_SetSystemId[1] ==  BluetoothRecBuffer[1]))
		{	
			FLASH_W25Q64_Update (System_SetParaAddr, &BluetoothRecBuffer[2], sizeof(System_ID));//更新系统ID信息
			Delay_ms(500);
			Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_SetSystemId, &BluetoothRecBuffer[2],6,1));
			Delay_ms(2000);
			Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_ReStart,&DatBuffer,1,1));
			IAP_CONTR = 0x20;
		}
else if((CMD_SetDoorOpenTime[0] ==  BluetoothRecBuffer[0]) && (CMD_SetDoorOpenTime[1] ==  BluetoothRecBuffer[1]))
		{	
			FLASH_W25Q64_Update (System_SetParaAddr+10, &BluetoothRecBuffer[2], 2);//更新系统开门时间
			Buzzer_Beep(BuzzerTimeNormal);
			Reply_State(CMD_SetDoorOpenTime,1);
			Delay_ms(1000);
			IAP_CONTR = 0x20;
		}
else if((CMD_DeleteSystem_SetPara[0] ==  BluetoothRecBuffer[0]) && (CMD_DeleteSystem_SetPara[1] ==  BluetoothRecBuffer[1]))
		{	
			FLASH_W25Q64_PageErase(System_SetParaAddr);
			Buzzer_Beep(BuzzerTimeNormal);
			Reply_State(CMD_DeleteSystem_SetPara,1);
			Delay_ms(1000);
			IAP_CONTR = 0x20;
		}
BTCounter  = 0;
memset(BluetoothRecBuffer,0x00,30);
}



/*****************************************************************************/
/*串口数据发送程序   														 */
/*入口参数：data 待发送的数据缓存                                            */
/*          len 待发送数据的个数                                             */
/*          UATR_Num   使用的串口号                                          */
/*   返回值： 无                                                             */
/*****************************************************************************/
void USART1_Send(unsigned char *dat,unsigned int len)
{  unsigned int i;

for(i=0;i<len;i++)	
{ 
	{SBUF=*dat++;while(!(SCON&0x02));SCON &=~0x02;_nop_();_nop_();}
}

}

void USART4_Send(unsigned char *dat,unsigned int len){
unsigned int i;
for(i=0;i<len;i++)	
{ 
 {S4BUF=*dat++;while(!(S4CON&0X02));S4CON &=~0X02;_nop_();_nop_();}	  
}

}


/*******************************************************
函数名称：Muti_Send_Byte
函数功能：接收1个字节的数据）
入口函数：
出口函数：收到的字符
********************************************************/
char Muti_Read_Byte(void)
{  
unsigned char dat;
	
dat = P2;

P1M1 &= ~(1<<2);									//CHACK初始化为输出	
P1M0 |= (1<<2);
CHACK = 1;
_nop_();
CHACK = 0;
return dat;
} 


/*******************************************************
函数名称：Muti_Send_Byte
函数功能：本机主动开始一次会话（发送数据）
入口函数：要转送的数据Data  数据长度len
出口函数：无
********************************************************/
void Muti_Send_Byte(unsigned char *Data,unsigned int len)
{ 
unsigned int i=0; 
//SBUF=0xbb;while(!(SCON&0X02));SCON &=~0X02;    

if(!CHKREQ)
{
REQ = 1;															//置REQ为有效
if(!CHKREQ)
{
	P2M1 = 0x00;												//P2为输出
	P2M0 = 0xff;
	P1M1 |= (1<<2);
	P1M0 &= ~(1<<2);										//CHACK初始化为输入
	while(i<len)
		{
			Delay_us(100);			
			P2 = *(Data+i);									//将数据送到P2端口
			Delay_us(10);
			STB = ~STB;
			while(CHACK);
			i++;
		}
REQ = 0;		
}else REQ = 0;
}
REQ = 0;
P2M1 = 0xff;
P2M0 = 0x00;		//初始化P2为输入
}

/********************************************************
函数名称:Arraycat
函数功能:对数组进行拼接
入口函数: Data_Dest			原数组
				 Data_Src				要添加的数组
				 DestDataNum  	原数组的数量
				 SrcDataNum 		要添加的数量
出口函数:无
备注:无
********************************************************/
void Arraycat(unsigned char *Data_Dest,unsigned char *Data_Src,unsigned int DestDataNum,unsigned int SrcDataNum)
 {unsigned int i;
  for(i=0;i<SrcDataNum;i++)
   {*(Data_Dest+DestDataNum+i)=*(Data_Src+i);} 
  }
/*****************************************************************************
串口数据帧组织程序 
函数名称:Fe_data_Organize
入口参数：	CMD 当前操作的命令                                                      
					Data 当前的操作数据                                       
          DataNum   当前的操作数据的个数                                    					 
返回值： 整型，为当前UART1_RE_DATA数组有效数据的个数                  
备注:程序组织成通用程序，只用传递相关的参数，自己合成返回帧，定义结构体变量。 
	帧结构为 帧头 数据长度 目标地址 序列号 命令 数据 校验和 帧尾            
*****************************************************************************/
unsigned int Fe_data_Organize(unsigned char CMD[],unsigned char Data[],unsigned int DataNum,bit OrgType)
 { 
	unsigned char Organize_DataBuffer[2]={0};
	unsigned int i=0;
	unsigned int StartPointer0=0,StartPointer1=0; 
	/************************************************************************/
	/*组织帧*/
	/*组织帧头*/
		if(OrgType){
	StartPointer1=2;
	Arraycat(&CommonDataBuffer[0],&FE_START[0],StartPointer0,StartPointer1);/*组织帧头*/
	}else if(!OrgType){
	StartPointer1=2;
	Arraycat(&CommonDataBuffer[0],&FE_START_ACK[0],StartPointer0,StartPointer1);/*组织帧头*/
	}
	StartPointer0=StartPointer0+StartPointer1;
	Organize_DataBuffer[0]=(DataNum&0xff00)>>8;/*追加数据长度*/
	Organize_DataBuffer[1]=DataNum&0xff;
	StartPointer1=2;
	Arraycat(&CommonDataBuffer[0],&Organize_DataBuffer[0],StartPointer0,StartPointer1);
	StartPointer0=StartPointer0+StartPointer1;
	StartPointer1=6;
	Arraycat(&CommonDataBuffer[0],&System_ID[0],StartPointer0,StartPointer1);/*追加主机ID*/
	StartPointer0=StartPointer0+StartPointer1;
	CommonDataBuffer[StartPointer0]=UART_Fe_Seq;
	StartPointer0++;
	StartPointer1=2;
	Arraycat(&CommonDataBuffer[0],&CMD[0],StartPointer0,StartPointer1);/*追加命令号*/
	StartPointer0=StartPointer0+StartPointer1;
	if(DataNum>0) 
	 {StartPointer1=DataNum;
	  Arraycat(&CommonDataBuffer[0],&Data[0],StartPointer0,StartPointer1);/*追加数据*/
	  StartPointer0=StartPointer0+StartPointer1; }
	 Organize_DataBuffer[0]=0;
	for(i=0;i<StartPointer0;i++)/*计算校验和*/
		Organize_DataBuffer[0]=Organize_DataBuffer[0]+CommonDataBuffer[i];
//		S4BUF =StartPointer0>>8;while(!(S4CON&0x02));S4CON &=~0x02;_nop_();
//		S4BUF =StartPointer0;while(!(S4CON&0x02));S4CON &=~0x02;_nop_();
//		S4BUF =Organize_DataBuffer[0];while(!(S4CON&0x02));S4CON &=~0x02;_nop_();
		Organize_DataBuffer[1]=0x16;
		Arraycat(&CommonDataBuffer[0],&Organize_DataBuffer[0],StartPointer0,2);/*追加校验和和帧尾*/
		StartPointer0=StartPointer0+2;
		return(StartPointer0);
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
void Delay_us(UINT us){
UINT k;
unsigned char i;
for(k=0;k<us;k++){
	i = 3;
	while (--i);
}
}


