#include "flashfunction.h"
#include "init.h"
#include <string.h>
#define FE_MESSTYPEADDR 11 /*消息类型字节在帧中的起始位置*/
#define FE_DATAADDR 13 /*数据字节在帧中的起始位置*/
#define FE_CODENUM 10 //帧序号字节在帧中的起始位置
#define FE_LENGTH 2 //定义数据长度在帧中的位置
#define SYSTEM_ID 4 //定义本机编号在帧中的位置
#define HeartbeatTime 1000*4					  //心跳包时间40s
#define BuzzerTimeNormal	10						//触发一次蜂鸣器响的时间（正常100ms）
#define BuzzerTimeLong 100							//触发一次蜂鸣器响的时间（拉长1000ms）
#define BuzzerTimeShort 2								//急促蜂鸣器（20ms）
#define ResetWifiTime 6000*8						//和服务器通信超时WIFI重启时间60*5s
#define Buzzer P36											//蜂鸣器引脚定义
#define nReloadWifi P37									//WIFI恢复出厂设置引脚（需要拉低电平大于1s）
#define nResetWifi P07									//WIFI重启引脚（需要拉低电平至少200MS）
#define SysCordAppUpdataAddrFlag 0x7E00
#define SysCordAppSaveStartTempAddr 0x8000 /*程序应用区暂存的起始地址*/
unsigned char const ReplySucc = 0x31;		
unsigned char const ReplyFail = 0x30;
unsigned int HeartbeatPacketCounter = 0;
unsigned int BuzzerCounter = 0;
unsigned int ResetWifiCounter = 0;
int  BuzzerTime = BuzzerTimeNormal;							  //初始化触发一次蜂鸣器响的时间（正常100ms）
bit HeartbeatTrigger;
bit BuzzerTriggrt;
bit ResetWifiTrigger;
bit ResetWifiOnOff;
bit ENTM;						//WIFI模块透传和串口命令模式切换位
bit Com1HaveDate;
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

unsigned int Exint0Counter = 0;						//外部中断0接受数据计数器
unsigned char Exint0DataBuffer[600] = {0};
unsigned char Com1DataBuffer[600] = {0};
unsigned int Com1DataCounter = 0;
unsigned char System_ID[6]={0x03,0x71,0x00,0x01,0x00,0x01};
const unsigned char code WifiInterSerialConfigModeCMD[4] = {'+','+','+','a'};/*+++ a*/
const unsigned char code WifiInterTmModeCMD[8] = {'A','T','+','E','N','T','M',0x0d};/*AT+ENTM<CR>*/
const unsigned char code AppVersionNum[4] = {0x20,0x16,0x06,0x14};/*闸机程序版本编号V2016.6.01*/
const unsigned char code FE_START[2]={0x55,0xaa};//帧头
const unsigned char code FE_START_ACK[2]={0xaa,0x55};//回复上位机的帧头
const unsigned char code CMD_Heartbeat[2]={0x00,0x01};/*心跳包命令*/
//const unsigned char code CMD_ServerAck[2]={0x01,0x01};/*和服务器通信超时重启WIFI*/
const unsigned char code CMD_ReStart[2]={0x00,0x02};/*重启命令*/
const unsigned char code CMD_UpdateSystem[2]={0x00,0x06};/*升级命令*/
const unsigned char code CMD_DownLoadUerID[2]={0x30,0x01};/*下载身份证信息*/
const unsigned char code CMD_DownLoadErWerMa[2]={0x40,0x01};/*下载二维码信息*/
const unsigned char code CMD_DeleteErWeiMa[2]={0x40,0x02};/*删除单条二维码信息*/
const unsigned char code CMD_DeleteID[2]={0x30,0x02};/*删除单条身份证信息*/
const unsigned char code CMD_DeleteAllErWeiMa[2]={0x40,0x03};/*删除所有二维码信息*/
const unsigned char code CMD_DeleteAllID[2]={0x30,0x03};/*删除所有身份证信息*/
const unsigned char code CMD_SearchUerID[2]={0x80,0x01};/*搜索游客身份证信息*/
const unsigned char code CMD_SearchErWerMa[2]={0x70,0x01};/*搜索游客二维码信息*/
const unsigned char code CMD_AskAppVersionNum_Top[2]={0x60,0x01};/*查询闸机上板软件版本号*/
const unsigned char code CMD_AskAppVersionNum_Bottom[2]={0x60,0x02};/*查询闸机下板软件版本号*/
const unsigned char code CMD_WifiSerialConfigMode[2]={0xf8,0x01};/*进入WIFI串口配置模式命令*/
const unsigned char code CMD_Buzzer[2]={0xff,0xff};/*蜂鸣器命令*/
const unsigned char code CMD_DoorTrigger[2]={0xfe,0xfe};/*内部开闸命令*/
const unsigned char code CMD_SetSystemId[2]={0xff,0x01};/*闸机ID设置命令*/
const unsigned char code CMD_nReloadWifi[2]={0xff,0x02};/*闸机重置WIFI命令*/
unsigned char UART_Fe_Seq  = 0;						//帧序号
unsigned char CommonDataBuffer[600]={0};	//数据缓存数组
void Exint0_DATA_deal(void);
void USART1_Send(unsigned char *dat,unsigned int len);
void USART4_Send(unsigned char *dat,unsigned int len);
void Arraycat(unsigned char *Data_Dest,unsigned char *Data_Src,unsigned int DestDataNum,unsigned int SrcDataNum);
unsigned int Fe_data_Organize(unsigned char CMD[],unsigned char Data[],unsigned int DataNum,bit OrgType);
char Muti_Read_Byte(void);
void WifiSeiralConfigMode(void);
void Muti_Send_Byte(unsigned char *Data,unsigned int len);
void Reply_State(unsigned char CMD[],unsigned char  State);
 bit Delete_Id_Info(UCHAR *IdData);
UCHAR Delete_IdOrErWeiMa_Info(UCHAR *IdData,bit SecletFlag);
unsigned long int Search_IdOrErWeiMa_Info(UCHAR *Data,bit SecletFlag);
unsigned char Store_IdOrErWeiMa_Data(unsigned char *Data,bit SecletFlag);
void Delay_ms(UINT ms);
void Delay_us(UINT ms);


void main(void){
//	UCHAR Test[3]={0x88,0x01,0x02};
	
IO_Init();
init_Uart();
Timer0Init();
INT_Init();
System_SetParaInit();
EA=1;
//		USART_Send(Test,sizeof(Test),1);
		Delay_ms(100);
		BuzzerTriggrt = 1;


while(1){
//		Delay_ms(1000);
//		SBUF = 0xaf;while(!(SCON&0x02));SCON &= ~0x02;
//if(Com1DataCounter != 0){
//	Muti_Send_Byte(Com1DataBuffer,Com1DataCounter);		//将上位机的数据直接下发到下面的单片机
//	Com1DataCounter=0;
//	memset(Com1DataBuffer,0x00,600);
//}
	
	P34 = 1;
	ENTM = 1;
if(HeartbeatTrigger == 1)
{
	UART_Fe_Seq++;
	USART1_Send(CommonDataBuffer,Fe_data_Organize(CMD_Heartbeat,&ReplySucc,1,0));
	HeartbeatTrigger = 0;
}

if(ResetWifiTrigger == 1)
{	/*由徐志坚禁止了没有收到服务器的命令而重启WiFi的功能*/
	//nResetWifi = 0;   
	//Delay_ms(500);
	//nResetWifi = 1;
	//ResetWifiOnOff  = 0;//关闭WIFI重置开关，WIFI重启以后，必须关闭重置开关防止WIFI模块连接服务器过程中再次重启。
	ResetWifiTrigger = 0;
}

if(Com1HaveDate)
{
	USART4_Send(Com1DataBuffer,Com1DataCounter);
	Com1DataCounter=0;
	memset(Com1DataBuffer,0x00,600);
	Com1HaveDate = 0;
}


if(Exint0Counter != 0)
{
	Exint0_DATA_deal();
}

	}

}

/****************************************************/
/* 定时器0中断入口函数					            */
/* 程序功能：定时器0中断处理函数*/
/* 入口参数:无										*/
/* 出口参数:无										*/
/****************************************************/
void Timer0_intrupt_function(void) interrupt 1 using 1
{ 
	//心跳包设定
	if(HeartbeatTrigger == 0)
 {
	 HeartbeatPacketCounter++;
		if(HeartbeatPacketCounter > HeartbeatTime){
		HeartbeatTrigger = 1;
		HeartbeatPacketCounter = 0;
		}
 }
	 //蜂鸣器设定
		if(BuzzerTriggrt == 1)
	 {
		 Buzzer = 0;
		 BuzzerCounter++;
			if(BuzzerCounter > BuzzerTime){
			Buzzer = 1;
			BuzzerTriggrt = 0;
			BuzzerCounter = 0; 
			}
	 }
	 	//服务器询问超时
	 if(ResetWifiOnOff){
		if(ResetWifiTrigger == 0)
	 {
			ResetWifiCounter++;
			if(ResetWifiCounter > ResetWifiTime){
			ResetWifiTrigger = 1;
			ResetWifiCounter = 0;
			}
	 }
 }
 }


void com1_isr(void) interrupt 4 using 1
{
unsigned char dat;
	if(SCON&0x01) 		 /*接收*/
		{
				dat = SBUF;
				P34 = 0;			//板子上的指示灯
				SCON &=~0x01;
			if(ENTM)				//透传模式
			{
				if(!CHKREQ)
				{
				REQ = 1;															//置REQ为有效
				if(!CHKREQ)
				{
					P2M1 = 0x00;												//P2为输出
					P2M0 = 0xff;
					P1M1 |= (1<<2);
					P1M0 &= ~(1<<2);										//CHACK初始化为输入
					P2 = dat;														//将数据送到P2端口
					Delay_us(10);
					STB = ~STB;
					while(CHACK);
				REQ = 0;		
				}else REQ = 0;
				}
				REQ = 0;
				P2M1 = 0xff;
				P2M0 = 0x00;		//初始化P2为输入						
			}else
					{			
						Com1HaveDate = 1;			
						Com1DataBuffer[Com1DataCounter++] = dat;
					}

		}			
}

void com2_isr(void) interrupt 8 using 1
{ 
  if(S2CON&0X01)  /*接收*/
   {S2CON &=~0X01;}			
}

void com3_isr(void) interrupt 17 using 1
{
	if(S3CON&0X01)  /*接收*/
	{	S3CON &=~0X01;}								   
}

void com4_isr(void) interrupt 18 using 1
{
	if(S4CON&0X01)  /*接收*/
		{	S4CON &=~0X01;}								   
}

void Exint0() interrupt 0 
{
	Exint0DataBuffer[Exint0Counter++] = Muti_Read_Byte();
	if(Exint0Counter>600) {Exint0Counter=0;}
}


void Exint0_DATA_deal(void){
unsigned int i = 0,j= 0,DataLength = 0;
unsigned int LastUpdatePacFlag = 0,FE_CS = 0;
unsigned long int DeleAddr = 0;
unsigned char  flag = 0,CS = 0,HeadState = 0,TempBuffer[6] = {0x00};
	Delay_ms(100);

	if(((Exint0DataBuffer[0] == FE_START[0])&&(Exint0DataBuffer[1] == FE_START[1])) || ((Exint0DataBuffer[0] == FE_START_ACK[0]) && (Exint0DataBuffer[1] == FE_START_ACK[1]) )){
				DataLength = Exint0DataBuffer[FE_LENGTH]*0x100+Exint0DataBuffer[FE_LENGTH+1];
				FE_CS=DataLength+FE_DATAADDR;/*指向校验码的位置*/
		for(i=0;i<FE_DATAADDR+DataLength;i++)
		CS+=Exint0DataBuffer[i];

			for(i=0,j=0;i<6,j<6;i++,j++){
			if(System_ID[j] == Exint0DataBuffer[SYSTEM_ID]+i)															//判断是否是本机编号
				flag++;
	}
//	SBUF = 0x8d;while(!(SCON&0x02));SCON &=~0x02;_nop_();
	if(CS == Exint0DataBuffer[FE_CS]){
//	for(i=0;i<Exint0Counter;i++){
//SBUF = Exint0DataBuffer[i];while(!(SCON&0x02));SCON &=~0x02;
//}
//	SBUF = 0xf9;while(!(SCON&0x02));SCON &=~0x02;
		UART_Fe_Seq = Exint0DataBuffer[FE_CODENUM];								//纪录帧序号
		//蜂鸣器
			if((CMD_Buzzer[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_Buzzer[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				switch(Exint0DataBuffer[FE_DATAADDR])
					{
					case 0x01 :
							BuzzerTime = BuzzerTimeNormal;
							break;
					case 0x02 :
							BuzzerTime = BuzzerTimeLong;
							break;
					case 0x03:
							BuzzerTime = BuzzerTimeShort;
							break;
					default:
							BuzzerTime = BuzzerTimeNormal;							
					}
				BuzzerTriggrt = 1;
			}//进入WIFI串口配置模式
			else if((CMD_WifiSerialConfigMode[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_WifiSerialConfigMode[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				WifiSeiralConfigMode();
				Reply_State(CMD_WifiSerialConfigMode,1);
			}		
			//服务器询问超时，重置WIFI模块
			else if((CMD_Heartbeat[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_Heartbeat[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(ResetWifiOnOff == 0){
				ResetWifiOnOff  = 1;//打开WIFI重置开关
				}
				ResetWifiCounter = 0;//计数器清零从头开始计时
				//Reply_State(CMD_ServerAck,1);
			}
			//查询上板软件版本号
			else if((CMD_AskAppVersionNum_Top[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_AskAppVersionNum_Top[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
					USART1_Send(Exint0DataBuffer,Exint0Counter);
			}
			//查询下板软件版本号
			else if((CMD_AskAppVersionNum_Bottom[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_AskAppVersionNum_Bottom[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
				USART1_Send(CommonDataBuffer,Fe_data_Organize(CMD_AskAppVersionNum_Bottom,AppVersionNum,4,1));
			}//删除所有身份证
			else if((CMD_DeleteAllID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
				for(DeleAddr=Id_StoreAddrTable[0][0];DeleAddr<Id_StoreAddrTable[10][1];DeleAddr+=0x10000){
//					SBUF = 0x87;while(!(SCON&0x02));SCON &=~0x02;
					sFLASH_64KBEraseSector(DeleAddr);
//					SBUF = 0x88;while(!(SCON&0x02));SCON &=~0x02;
					
				}
					Reply_State(CMD_DeleteAllID,1);
			}
			//删除所有二维码
			else if((CMD_DeleteAllErWeiMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllErWeiMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
				{
					for(DeleAddr=ErWeiMa_AddrTable[0][0];DeleAddr<ErWeiMa_AddrTable[15][1];DeleAddr+=0x10000){
					sFLASH_64KBEraseSector(DeleAddr);
					}
					Reply_State(CMD_DeleteAllErWeiMa,1);
			}
				//搜索游客身份证信息
			else if((CMD_SearchUerID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SearchUerID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				if(Search_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR],1)!=0){
				Delay_ms(100);
//				for(i=0;i<j;i++){
//					SBUF = CommonDataBuffer[i];while(!(SCON&0x02));SCON &=~0x02;_nop_();
//				}
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DoorTrigger,&CS,1,1));//发给下板开闸
				BuzzerTriggrt = 1;
			}

			}/*搜索游客二维码信息*/
			else if((CMD_SearchErWerMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SearchErWerMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				if(Search_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR],0)!=0){
				Delay_ms(100);
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DoorTrigger,&CS,1,1));//发给下板开闸
				BuzzerTriggrt = 1;
			}

			}//重启
			else if((CMD_ReStart[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_ReStart[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				BuzzerTriggrt = 1;
				Reply_State(CMD_ReStart,1);
				Delay_ms(1000);			
				IAP_CONTR = 0x20;
			}//设置闸机ID号
			else if((CMD_SetSystemId[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SetSystemId[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				if(Exint0DataBuffer[FE_DATAADDR] == 0x31){
				Reply_State(CMD_SetSystemId,1);
				}else{
					FLASH_W25Q64_Update (System_SetParaAddr, &Exint0DataBuffer[FE_DATAADDR], sizeof(System_ID));//更新系统ID信息
//						for(i=0;i<sizeof(System_ID);i++){
//					System_ID[i] = Exint0DataBuffer[FE_DATAADDR+i];
//					}
			}
						Reply_State(CMD_SetSystemId,1);
			}else if(CMD_nReloadWifi[0] == Exint0DataBuffer[FE_MESSTYPEADDR] &&  CMD_nReloadWifi[1] == Exint0DataBuffer[FE_MESSTYPEADDR+1]){
//					SBUF = 0x9d;while(!(SCON&0x02));SCON &=~0x02;_nop_();
					BuzzerTriggrt = 1;
					nReloadWifi = 0;
					Delay_ms(3000);
					nReloadWifi = 1;
					Delay_ms(500);//复位WIFI模块
					Reply_State(CMD_nReloadWifi,1);
					BuzzerTriggrt = 1;
				}//升级
			else if((CMD_UpdateSystem[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_UpdateSystem[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				ResetWifiOnOff  = 0;//升级关闭WIFI重置
				if(Exint0DataBuffer[FE_DATAADDR] == 0x01){
						if(Exint0DataBuffer[FE_DATAADDR+4]==Exint0DataBuffer[FE_DATAADDR+6]){//最后一个升级包
							if(sequential_write_flash_in_one_sector(SysCordAppSaveStartTempAddr+(Exint0DataBuffer[FE_DATAADDR+6]-1)*0x200, 512, &Exint0DataBuffer[FE_DATAADDR+9])){
							FLASH_PageErase(SysCordAppUpdataAddrFlag);
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag, 0x5A);//写升级标志
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag+1, 0xA5);
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag+2, Exint0DataBuffer[FE_DATAADDR+4]);//纪录升级数据包个数
							BuzzerTriggrt = 1;
							Reply_State(CMD_UpdateSystem,1);
							Delay_ms(1000);
							IAP_CONTR = 0x20;//重启进入升级
						}
					}else{			
								if(sequential_write_flash_in_one_sector(SysCordAppSaveStartTempAddr+(Exint0DataBuffer[FE_DATAADDR+6]-1)*0x200, 512, &Exint0DataBuffer[FE_DATAADDR+9])){
								BuzzerTime = BuzzerTimeShort;
								BuzzerTriggrt = 1;
								Reply_State(CMD_UpdateSystem,1);
							}
						}
				}else if(Exint0DataBuffer[FE_DATAADDR] == 0x31){				
				BuzzerTriggrt = 1;
				Reply_State(CMD_UpdateSystem,1);
				}
			}//下载身份证				
			else if((CMD_DownLoadUerID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DownLoadUerID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(Exint0DataBuffer[FE_DATAADDR] == 0x01){
				if(Store_IdOrErWeiMa_Data(&Exint0DataBuffer[FE_DATAADDR+1],1) == 0x01){
				Reply_State(CMD_DownLoadUerID,1);
				}
			}else {
				Reply_State(CMD_DownLoadUerID,1);
			}
			}
			//删除身份证
			else if((CMD_DeleteID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
					if(Exint0DataBuffer[FE_DATAADDR] == 0x01){
						if(Delete_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR+1],1) == 0x01){
						Reply_State(CMD_DeleteID,1);
						}
					}else {
					Reply_State(CMD_DeleteID,1);
				}
			}
			//下载二维码
			else if((CMD_DownLoadErWerMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DownLoadErWerMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(Exint0DataBuffer[FE_DATAADDR] == 0x01){
					if(Store_IdOrErWeiMa_Data(&Exint0DataBuffer[FE_DATAADDR+1],0) == 0x01){
					Reply_State(CMD_DownLoadErWerMa,1);
					}
			}else {
					Reply_State(CMD_DownLoadErWerMa,1);
				}
		}//删除二维码
			else if((CMD_DeleteErWeiMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteErWeiMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
				{
						if(Exint0DataBuffer[FE_DATAADDR] == 0x01){
							if(Delete_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR+1],0) == 0x01){
								Reply_State(CMD_DeleteErWeiMa,1);
								}
					}else {
							Reply_State(CMD_DeleteErWeiMa,1);
							}
				}	
				else	
					{
						USART1_Send(Exint0DataBuffer,Exint0Counter);
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
		USART1_Send(CommonDataBuffer,Reply_Num);
}else if(State == 0){
		Reply_Num =  Fe_data_Organize(CMD,&ReplyFail,1,0);
		USART1_Send(CommonDataBuffer,Reply_Num);
}
	memset(CommonDataBuffer,0x00,600);
}

//储存信息
unsigned char Store_IdOrErWeiMa_Data(unsigned char *Data,bit SecletFlag){
UINT 	i=0;
UINT 	ItemCounter=0;//该区域的用户信息数量
UCHAR	ItemCounterBuffer[2] = {0},Cursor = 0;
UCHAR TempIdBuffer[20]={0};
UCHAR StoreErWeiMaBuffer[10]={0};
UCHAR AttachData[4]={0xff,0xff,0xff,0xff};
//身份证
if(SecletFlag){
for(i=0;i<16;i++)
	{
		TempIdBuffer[i] = *Data++;	//暂存第一条信息
	}
	Arraycat(TempIdBuffer,AttachData,16,4);	//添加附加信息
	Cursor = TempIdBuffer[8];
	if(Search_IdOrErWeiMa_Info(TempIdBuffer,1) == 0){//信息不存在则添加
		ItemCounter = FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0]+1);//读取该区域储存的信息总量
//		 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
	//根据身份证校验码10种分类储存不同的区域
		if(ItemCounter == 0xffff)//初始化工作
		ItemCounter = 0;

		FLASH_W25Q64_Write ( Id_StoreAddrTable[Cursor&= 0x0f][0]+ItemCounter*20+60,TempIdBuffer, 20);//在尾部追加信息 每个区域前60字节预留

		ItemCounter ++;//数量增加1
	//	SBUF = 0xa6;while(!(SCON&0x02));SCON &= ~0x02;
		ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
		ItemCounterBuffer[1] = ItemCounter & 0x00ff;
		FLASH_W25Q64_Update (Id_StoreAddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//更新数量信息
//						 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
				FLASH_W25Q64_Read(CommonDataBuffer,Id_StoreAddrTable[Cursor&= 0x0f][0],600);//读入缓冲区
//			for(i=0;i<600;i++){
//			 SBUF = CommonDataBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();_nop_();
//			}
//	 SBUF = 0xa7;while(!(SCON&0x02));SCON &= ~0x02;
	}else{
		return 1;
	}
	return 1;
}else if(!SecletFlag)
//二维码
{
for(i=0;i<6;i++)
	{
		StoreErWeiMaBuffer[i] = *Data++;	//暂存第一条信息
	}
		Arraycat(StoreErWeiMaBuffer,AttachData,6,4);	//添加附加信息
		if(Search_IdOrErWeiMa_Info(StoreErWeiMaBuffer,0) == 0){//信息不存在则添加
		Cursor = StoreErWeiMaBuffer[5] ;
//		for(i=0;i<6;i++){
//		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}

	ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0]+1);//读取该区域储存的信息总量
//	 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
//根据校验码16种分类储存不同的区域
	if(ItemCounter == 0xffff)//初始化工作
	ItemCounter = 0;

//					for(i=0;i<10;i++){
//		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}
					
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
//			 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
//		for(i=0;i<600;i++){
//		 SBUF = CommonDataBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();_nop_();
//		}
	}else{
		return 1;
	}
	return 1;
}

return 1;
}
//搜索身份证信息 返回所在地址或者0（没有搜索到信息）
unsigned long int Search_IdOrErWeiMa_Info(UCHAR *Data,bit SecletFlag){

UCHAR TempIdData[9],*Result_Addr,Cursor = 0;
UCHAR TempErWeiMaBuffer[6]={0};
ULONG CurrentAddr = 0;
UINT i=0,ItemCounter=0,Comparei = 0;
	if( SecletFlag){
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
//				 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
				 Result_Addr = CommonDataBuffer;
					for(i=0;i<600;i+=20){
						if(strncmp(Result_Addr,TempIdData,9) != 0){
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
	}else if(!SecletFlag){
	for(i=0;i<6;i++){
		TempErWeiMaBuffer[i] = *Data++;
	}
//		for(i=0;i<6;i++){
//		SBUF = TempErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}
		Cursor = TempErWeiMaBuffer[5];
		ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor &= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor &= 0x0f][0]+1);//读取该区域储存的信息总量
			if(ItemCounter == 0xffff)
				ItemCounter = 0;
			for(CurrentAddr = ErWeiMa_AddrTable[Cursor&= 0x0f][0];CurrentAddr<ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30;CurrentAddr+=600){
				 FLASH_W25Q64_Read(CommonDataBuffer,CurrentAddr,600);//读入缓冲区
//				 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
				 Result_Addr = CommonDataBuffer;
					for(i=0;i<600;i+=10){
						if(strncmp(Result_Addr,TempErWeiMaBuffer,6) != 0){
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
		}
memset(CommonDataBuffer,0x00,600);
return 0;
}



//删除身份证信息  返回0（删除失败）或者1（删除成功）
UCHAR Delete_IdOrErWeiMa_Info(UCHAR *IdData,bit SecletFlag){
	UINT 	ItemCounter=0;//该区域的用户信息数量
	UCHAR	ItemCounterBuffer[2] = {0},Cursor = 0;
	UCHAR DelErWeiMaBuffer[6]={0};
	ULONG TempAddr = 0;
	UCHAR TempIdData[9] ={0};
	UCHAR TempFlashBuffer[20]= {0};
	UINT i;
		if(SecletFlag){
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
	}else if(!SecletFlag){
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
	}
	return 0;
}



void WifiSeiralConfigMode(void)
{
	ENTM = 0;//打开透传模式串口接收
	USART1_Send(&WifiInterSerialConfigModeCMD ,1 );
	Delay_ms(250);
	USART1_Send(&WifiInterSerialConfigModeCMD+1 ,1 );
	Delay_ms(250);
	USART1_Send(&WifiInterSerialConfigModeCMD+2 ,1 );
	
	Delay_ms(2000);
	USART1_Send(&WifiInterSerialConfigModeCMD+3 ,1 );
	Delay_ms(2000);
	USART1_Send(WifiInterTmModeCMD,sizeof(WifiInterTmModeCMD));//进入透传模式
	ENTM = 1;//关闭透传模式串口接收
}



///****************************************************/
///* 串口1中断处理函数					            */
///* 程序功能：串口1中断处理函数                    */
///* 入口参数:无										*/
///* 出口参数:无										*/
///****************************************************/
//void Uart1_intrupt_function(void)
//{

//}



///****************************************************/
///* 串口2中断处理函数					            */
///* 程序功能：串口2中断处理函数                    */
///* 入口参数:无										*/
///* 出口参数:无										*/
///****************************************************/
//void Uart2_intrupt_function(void)
//{					   

//}



///****************************************************/
///* 串口3中断处理函数					            */
///* 程序功能：串口3中断处理函数                    */
///* 入口参数:无										*/
///* 出口参数:无										*/
///****************************************************/
//void Uart3_intrupt_function(void)
//{
//}





///****************************************************/
///* 串口4中断处理函数					            */
///* 程序功能：串口4中断处理函数                    */
///* 入口参数:无										*/
///* 出口参数:无										*/
///****************************************************/
//void Uart4_intrupt_function(void)
//{
//}

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
/*****************************************************************************/
/*串口数据发送程序   														 */
/*入口参数：data 待发送的数据缓存                                            */
/*          len 待发送数据的个数                                             */
/*          UATR_Num   使用的串口号                                          */
/*   返回值： 无                                                             */
/*****************************************************************************/
void USART4_Send(unsigned char *dat,unsigned int len)
{  unsigned int i;

for(i=0;i<len;i++)	
{ 
	{S4BUF=*dat++;while(!(S4CON&0x02));S4CON &=~0x02;_nop_();_nop_();}
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
	
//		for(i=0;i<len;i++){
//					SBUF = *Data++;while(!(SCON&0x02));SCON &=~0x02;_nop_();
//				}
if(!CHKREQ)
{
REQ = 1;																//置REQ为有效
if(!CHKREQ)
{
	P2M1 = 0x00;												//P2为输出
	P2M0 = 0xff;
	P1M1 |= (1<<2);
	P1M0 &= ~(1<<2);															//CHACK初始化为输入
	while(i<len)
		{
			//SBUF=i;while(!(SCON&0x02));SCON &=~0x02;_nop_();_nop_();
			P2 = *(Data++);														//将数据送到P2端口
			//SBUF=P2;while(!(SCON&0x02));SCON &=~0x02;_nop_();_nop_();
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
入口函数: Data_Src		要添加的数组	Data_Dest	原数组
SrcDataNum 要添加的数量 DestDataNum  	原数组的数量
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
					OrgType	组织的类型 1为头是55 AA 0为头是AA 55
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

void Delay_us(UINT ms){
UINT k;
unsigned char i;
for(k=0;k<ms;k++){
	i = 3;
	while (--i);
}
}
