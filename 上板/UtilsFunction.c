#include "flashfunction.h"
#include "LD9900MT.h"
#include "DS1302.h"
#include "SC50X0B.h"
#include "init.h"
#define BuzzerTimeNormal	0x01						//����һ�η��������ʱ�䣨����100ms��
#define BuzzerTimeLong 0x02								//����һ�η��������ʱ�䣨����3000ms��
#define BuzzerTimeShort 0x03							//���ٷ�������20ms��
#define FE_MESSTYPEADDR 11 /*��Ϣ�����ֽ���֡�е���ʼλ��*/
#define FE_DATAADDR 13 /*�����ֽ���֡�е���ʼλ��*/
#define FE_CODENUM 10 //֡����ֽ���֡�е���ʼλ��
#define FE_LENGTH 2 //�������ݳ�����֡�е�λ��
#define SYSTEM_ID 4 //���屾�������֡�е�λ��
#define FindCard_TimerCounterMAX 50 /*ÿ��0.5s���һ�����֤�Ƿ����*/
#define ShieldCMD_OpenOrCloseTime 300	/*���ؿ�բ�����η�������բʱ��3S*/
#define ResetU980Time 100*300			/*��ά�붨ʱ����300S*/
#define SYSTEM_ID 4 //���屾�������֡�е�λ��
#define SysCordAppUpdataAddrFlag 0x7E00
#define SysCordAppSaveStartTempAddr 0x8000 /*����Ӧ�����ݴ����ʼ��ַ*/
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
unsigned char BluetoothRecBuffer[30] = {0x00};//�����������ݻ���
unsigned int BTCounter = 0;//�����������������
unsigned char FingerDataBuffer[600] = {0x00}  ;
unsigned int DoorCounter = 0;
unsigned int ResetU980Counter = 0;
unsigned int ShieldCMD_OpenOrCloseCounter = 0;
unsigned char System_ID[6]={0x03,0x71,0x00,0x01,0x00,0x01};
const unsigned char code AppVersionNum[4] = {0x20,0x16,0x06,0x14};/*բ������汾���V2016.5.26*/
const unsigned char code FE_START[2]={0x55,0xaa};
const unsigned char code FE_START_ACK[2]={0xaa,0x55};//�ظ���λ����֡ͷ
const unsigned char code CMD_Heartbeat[2]={0x00,0x01};/*����������*/
//const unsigned char code CMD_ServerAck[2]={0x01,0x01};/*�ͷ�����ͨ�ų�ʱ����WIFI*/
const unsigned char code CMD_ReStart[2]={0x00,0x02};/*��������*/
const unsigned char code CMD_UpdateSystem[2]={0x00,0x06};/*�ϰ���������*/
const unsigned char code CMD_OpenOrClose[2]={0x00,0x03};/*ǿ�ƿ���բ*/
const unsigned char code CMD_SearchUerID[2]={0x80,0x01};/*�����ο����֤��Ϣ*/
const unsigned char code CMD_SearchErWerMa[2]={0x70,0x01};/*�����οͶ�ά����Ϣ*/
const unsigned char code CMD_DownLoadUerID[2]={0x30,0x01};/*�������֤��Ϣ*/
const unsigned char code CMD_UploadUerID[2]={0x00,0x19};/*��ˮ�����ϴ����֤*/
const unsigned char code CMD_DownLoadErWerMa[2]={0x40,0x01};/*���ض�ά����Ϣ*/
const unsigned char code CMD_DownLoadFingerData[2]={0x50,0x01};/*����ָ����Ϣ*/
const unsigned char code CMD_UploadErWeiMa[2]={0x00,0x29};/*��ˮ�����ϴ���ά��*/
const unsigned char code CMD_FingerUploadStreamInfo[2]={0x00,0x39};/*��ˮ�����ϴ�ָ�ƶ�Ӧ��ID*/
const unsigned char code CMD_DeleteFingerID[2]={0x50,0x02};/*ɾ������ָ����Ϣ*/
const unsigned char code CMD_DeleteErWeiMa[2]={0x40,0x02};/*ɾ��������ά����Ϣ*/
const unsigned char code CMD_DeleteID[2]={0x30,0x02};/*ɾ���������֤��Ϣ*/
const unsigned char code CMD_DeleteAllErWeiMa[2]={0x40,0x03};/*ɾ�����ж�ά����Ϣ*/
const unsigned char code CMD_DeleteAllID[2]={0x30,0x03};/*ɾ���������֤��Ϣ*/
const unsigned char code CMD_DeleteAllUserID[2]={0x88,0x88};/*ɾ�������û�ID��Ϣ*/
const unsigned char code CMD_FirstRecordFinger[2]={0x31, 0x03};/*��һ��¼ȡָ��*/
const unsigned char code CMD_SecondRecordFinger[2]={0x32, 0x03};/*�ڶ���¼ȡָ��*/
const unsigned char code CMD_EmptyFingerRecord[2]={0x50, 0x03};/*���ָ�ƿ�*/
const unsigned char code CMD_AskAppVersionNum_Top[2]={0x60,0x01};/*��ѯբ���ϰ�����汾��*/
const unsigned char code CMD_AskAppVersionNum_Bottom[2]={0x60,0x02};/*��ѯբ���°�����汾��*/
const unsigned char code CMD_WifiSerialConfigMode[2]={0xf8,0x01};/*����WIFI��������ģʽ����*/
const unsigned char code CMD_Buzzer[2]={0xff,0xff};/*����������*/
const unsigned char code CMD_DoorTrigger[2]={0xfe,0xfe};/*�ڲ���բ����*/
const unsigned char code CMD_SetSystemId[2]={0xff,0x01};/*բ��ID��������*/
const unsigned char code CMD_nReloadWifi[2]={0xff,0x02};/*բ������WIFI����*/
const unsigned char code CMD_SetDoorOpenTime[2]={0xff,0x03};/*բ������ʱ����������*/
const unsigned char code CMD_DeleteSystem_SetPara[2]={0xff,0x04};/*ɾ��ϵͳ��������*/
const unsigned char code FIND_CARD[10]={0xaa, 0xaa, 0xaa, 0x96, 0x69, 0x00, 0x03, 0x20, 0x01, 0x22};
const unsigned char code SELECT_CARD[10]={0xaa, 0xaa, 0xaa, 0x96, 0x69, 0x00, 0x03, 0x20, 0x02, 0x21};
const unsigned char code READ_CARD[10]={0xaa, 0xaa, 0xaa, 0x96, 0x69, 0x00, 0x03, 0x30, 0x01, 0x32};

//0x1000-0x7E00  ?C_C51STARTUP(1000H)

//���֤���滮�ִ����ַ��,ÿ����Ϣռ��20�ֽ�
//���֤����������֤�����һλУ���뻮��Ϊ11������ÿ������128KB ÿ�����֤��Ϣ����ʱ��20���ֽ�
//ÿ�����򹲿��Դ���6553-3����Ϣ������ǰ3*20=60�ֽ��������渽�ӵ���Ϣ�����統ǰ����Ĵ���������ÿ������128KB��ռ��1408KB�ռ�
const unsigned long int code Id_StoreAddrTable[11][2] = {{0x000000,0x1ffff},{0x20000,0x3ffff},{0x40000,0x5ffff},{0x60000,0x7ffff},
{0x80000,0x9ffff},{0xa0000,0xbffff},{0xc0000,0xdffff},{0xe0000,0xfffff},{0x100000,0x11ffff},{0x120000,0x13ffff},{0x140000,0x15ffff}};
//��ά�봢�滮�ִ����ַ��ÿ����Ϣռ��10�ֽ�
//��ά�봢������ֻ����ۼӵõ�һ��int������ȡ��4λ��Ϊ��������0x00-0x0f(16������)
//ÿ�����򹲿��Դ���6553-3����Ϣ������ǰ3*10=30�ֽ��������渽�ӵ���Ϣ�����統ǰ����Ĵ���������ÿ������64KB��ռ��1024KB�ռ�
const unsigned long int code ErWeiMa_AddrTable[16][2] = {{0x160000,0x16ffff},{0x170000,0x17ffff},{0x180000,0x18ffff},{0x190000,0x19ffff},
{0x1a0000,0x1affff},{0x1b0000,0x1bffff},{0x1c0000,0x1cffff},{0x1d0000,0x1dffff},{0x1e0000,0x1effff},{0x1f0000,0x1fffff},{0x200000,0x20ffff},
{0x210000,0x21ffff},{0x220000,0x22ffff},{0x230000,0x23ffff},{0x240000,0x24ffff},{0x250000,0x25ffff}};
//�û�id-ָ�Ʊ�ŵĴ���Ͷ�ά�����ͬ
const unsigned long int code FingerId_AddrTable[16][2] = {{0x260000,0x26ffff},{0x270000,0x27ffff},{0x280000,0x28ffff},{0x290000,0x29ffff},
{0x2a0000,0x2affff},{0x2b0000,0x2bffff},{0x2c0000,0x2cffff},{0x2d0000,0x2dffff},{0x2e0000,0x2effff},{0x2f0000,0x2fffff},{0x300000,0x30ffff},
{0x310000,0x31ffff},{0x320000,0x32ffff},{0x330000,0x33ffff},{0x340000,0x34ffff},{0x350000,0x35ffff}};
const unsigned char code EWMBuffer_Mima[11]={0xDB,0xCA,0xFE,0x63,0x78,0x12,0x43,0x67,0x35,0x23,0x98};
unsigned char UART_Fe_Seq  = 0;						//֡���
unsigned char CommonDataBuffer[600]={0};//���ݻ�������
unsigned char FingerPrcBuffer[600]={0};//ָ�������ݴ洦������
unsigned int Exint0Counter = 0;						//�ⲿ�ж�0�������ݼ�����
unsigned char Exint0DataBuffer[600] = {0};
unsigned char Uart4FindCartACK[200] = {0};//���֤��Ϣ��������
unsigned int Uart4ACKCounter = 0;//���֤���ݽ��ռ�����
unsigned char UserCardReadState=0; 
unsigned char Flag_FindCard_500msOver=0;
unsigned int FindCard_TimerCounter=0;


unsigned char SetTime[7]={30,52,22,27,03,04,16};//���ʱ��������10-08-15 23:59:50   7��������
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
/* ��ʱ��0�ж���ں���					            */
/* �����ܣ���ʱ��0�жϴ�����������д����������ʽ*/
/* ��ڲ���:��										*/
/* ���ڲ���:��										*/
/****************************************************/
void Timer0_intrupt_function(void) interrupt 1 using 1
{ 
	/*���Ŷ���*/
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
	//���֤
	if((Flag_FindCard_500msOver==0)&&(UserCardReadState==STATE_NOCARD))
	 {FindCard_TimerCounter++;
	  if(FindCard_TimerCounter>FindCard_TimerCounterMAX)
	   {Flag_FindCard_500msOver=1;FindCard_TimerCounter=0;}}
 //���ö�ά��ģ��
 if(ResetU980Trigger == 0)
 {
	 ResetU980Counter++;
	 if(ResetU980Counter>ResetU980Time)
	 {
		 ResetU980Trigger =1;
		 ResetU980Counter = 0;
	 }
 }
//����ָ����֤��ķ���˿�բ
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
{ if(S2CON&0x01)  /*����*/
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
	if(S3CON&0x01)  /*����*/
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
if(S4CON & 0x01)  /*����*/
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

void Exint0() interrupt 0       //INT0�ж����
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
		FE_CS=DataLength+FE_DATAADDR;/*ָ��У�����λ��*/
		for(i=0;i<FE_DATAADDR+DataLength;i++)//����У����
		CS+=Exint0DataBuffer[i];
		
			for(i=0,j=0;i<6,j<6;i++,j++){
			if(System_ID[j] == Exint0DataBuffer[SYSTEM_ID+i])															//�ж��Ƿ��Ǳ������
				flag++;
	}
//		for(i=0;i<Exint0Counter;i++){
//SBUF = Exint0DataBuffer[i];while(!(SCON&0x02));SCON &=~0x02;
//}
	if(CS == Exint0DataBuffer[FE_CS] && flag == 0x06){//У������ȷ
		UART_Fe_Seq = Exint0DataBuffer[FE_CODENUM];								//��¼֡���
//		for(i=0;i<Exint0Counter;i++){
//SBUF = Exint0DataBuffer[i];while(!(SCON&0x02));SCON &=~0x02;
//}
		//ǿ�ƿ���բ
			if((CMD_OpenOrClose[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_OpenOrClose[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]) && !ShieldOpenOrClose)
			{	
				DoorTrigger = 1;
				Reply_State(CMD_OpenOrClose,1);
				Buzzer_Beep(BuzzerTimeNormal);
			}//����WIFI��������ģʽ
			if((CMD_WifiSerialConfigMode[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_WifiSerialConfigMode[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
			}//�������ظ�
			if((CMD_Heartbeat[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_Heartbeat[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
			}			
			//����
			if((CMD_ReStart[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_ReStart[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_ReStart,&flag,1,1));
				Delay_ms(1000);
				Buzzer_Beep(BuzzerTimeNormal);
				IAP_CONTR = 0x20;
			}
			//�ڲ���բ
			if((CMD_DoorTrigger[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DoorTrigger[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				DoorTrigger = 1;
			}//��ѯ�ϰ�����汾��
			if((CMD_AskAppVersionNum_Top[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_AskAppVersionNum_Top[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_AskAppVersionNum_Top,AppVersionNum,4,0));
			}//��ѯ�°�����汾��
			if((CMD_AskAppVersionNum_Bottom[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_AskAppVersionNum_Bottom[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
				Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
			}			
			//����բ��ID��
			if((CMD_SetSystemId[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SetSystemId[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				FLASH_W25Q64_Update (System_SetParaAddr, &Exint0DataBuffer[FE_DATAADDR], sizeof(System_ID));//����ϵͳID��Ϣ
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_SetSystemId,&Exint0DataBuffer[FE_DATAADDR],6,1));
				Delay_ms(1000);
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_ReStart,&flag,1,1));//����
				IAP_CONTR = 0x20;
			}//�������
			if((CMD_UpdateSystem[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_UpdateSystem[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
					if(Exint0DataBuffer[FE_DATAADDR] == 0x00){
						if(Exint0DataBuffer[FE_DATAADDR+4]==Exint0DataBuffer[FE_DATAADDR+6]){//���һ��������
							if(sequential_write_flash_in_one_sector(SysCordAppSaveStartTempAddr+(Exint0DataBuffer[FE_DATAADDR+6]-1)*0x200, 512, &Exint0DataBuffer[FE_DATAADDR+9])){
							FLASH_PageErase(SysCordAppUpdataAddrFlag);
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag, 0x5A);//д������־
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag+1, 0xA5);
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag+2, Exint0DataBuffer[FE_DATAADDR+4]);//��¼�������ݰ�����
							Reply_State(CMD_UpdateSystem,1);
							Buzzer_Beep(BuzzerTimeNormal);
							Delay_ms(1000);
							IAP_CONTR = 0x20;//������������
						}
						}else{			
								if(sequential_write_flash_in_one_sector(SysCordAppSaveStartTempAddr+(Exint0DataBuffer[FE_DATAADDR+6]-1)*0x200, 512, &Exint0DataBuffer[FE_DATAADDR+9])){
								Reply_State(CMD_UpdateSystem,1);
								Buzzer_Beep(BuzzerTimeShort);
							}	
					}
				}else if( Exint0DataBuffer[FE_DATAADDR] == 0x01)
					{
						Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);//����������ת����U2���°壩	
					}
		}//ɾ���������֤
			if((CMD_DeleteAllID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
					for(DeleAddr=Id_StoreAddrTable[0][0];DeleAddr<Id_StoreAddrTable[10][1];DeleAddr+=0x10000){
					//SBUF = 0x76;while(!(SCON&0x02));SCON &=~0x02;
					sFLASH_64KBEraseSector(DeleAddr);
					//SBUF = 0x77;while(!(SCON&0x02));SCON &=~0x02;
					}
					Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DeleteAllID,&CS,1,1));
			}//ɾ�����ж�ά��
			if((CMD_DeleteAllErWeiMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllErWeiMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
					for(DeleAddr=ErWeiMa_AddrTable[0][0];DeleAddr<ErWeiMa_AddrTable[15][1];DeleAddr+=0x10000){
					sFLASH_64KBEraseSector(DeleAddr);
					}
					Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DeleteAllErWeiMa,&CS,1,1));
			}
			//ɾ������ָ��ID
			if((CMD_DeleteAllUserID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllUserID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
					for(DeleAddr=FingerId_AddrTable[0][0];DeleAddr<FingerId_AddrTable[15][1];DeleAddr+=0x10000){
					sFLASH_64KBEraseSector(DeleAddr);
					}
					Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DeleteAllUserID,&CS,1,1));
			}
			//�������֤
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
					//�����°��ӽ��մ���
					Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
				}
			}
			//ɾ�����֤
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
			//�����°��ӽ���ɾ��
					Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
				}
			}
			//���ض�ά��
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
			//�����°��ӽ��մ���
						Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
					}
			}
			//ɾ����ά��
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
			//�����°��ӽ���ɾ��
						Muti_Send_Byte(Exint0DataBuffer,Exint0Counter);
					}
			}//ָ��ͬ������
			if((CMD_DownLoadFingerData[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DownLoadFingerData[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(LD9900MT_DownChar_ReDataProc(&Exint0DataBuffer[FE_DATAADDR+5],&Exint0DataBuffer[FE_DATAADDR+1]))
				{
					Reply_State(CMD_DownLoadFingerData,1);
				}else
				{
					Reply_State(CMD_DownLoadFingerData,0);
				}
			}//ָ�Ƶ���ɾ��
				if((CMD_DeleteFingerID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteFingerID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				//���ҵ�ָ��ID��Ӧ��ָ�Ʊ�ţ���ָ��ģ����е���ɾ��������Ȼ�����FLASH�ж�Ӧ��ָ��ID��ָ�Ʊ�Ŷ�Ӧ��
				FLASH_W25Q64_Read(FingerStoreNumberBuffer,Search_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR] ,0x03)+6,2);//ָ�Ʊ�Ŷ��뻺����
//				S4BUF = FingerStoreNumberBuffer[0];while(!(S4CON&0x02));S4CON &= ~0x02;
//				S4BUF = FingerStoreNumberBuffer[1];while(!(S4CON&0x02));S4CON &= ~0x02;
				FingerStoreNumber = FingerStoreNumberBuffer[0]*256+ FingerStoreNumberBuffer[1];
				if(LD9900MT_DeleteChar(FingerStoreNumber ,1) == 0x00)//ɾ��ָ��ģ���е�����
				{
//				S4BUF = 0x88;while(!(S4CON&0x02));S4CON &= ~0x02;
					Delete_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR],0x03);//���FLASH�ж�Ӧ��ָ��ID��ָ�Ʊ�Ŷ�Ӧ��
					Reply_State(CMD_DeleteFingerID,1);
				}else
				{
					Reply_State(CMD_DeleteFingerID,1);
				}
			}			
			//���ָ�ƿ�
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
				//��һ��¼ȡָ��		
				if((CMD_FirstRecordFinger[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_FirstRecordFinger[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
					{
						//SBUF=0xab;while(!(SCON&0X02));SCON &=~0X02;Delay_ms(100);
						Delay_ms(100);		
						TimeOverCounter = 0;
					while(1 == SC50x0B_Busy());
					SC50x0B_Out(PUTFINGER);
					while(LD9900MT_GetImage(600) != 0x00 & TimeOverCounter < 10)
					{						//һֱ�ȴ���ȡָ��ͼ��ɹ�  ��ೢ��10��
						FirstState = 0;																					//��״̬λΪ0
						TimeOverCounter++;
						Buzzer_Beep(BuzzerTimeNormal);
					}
					if(TimeOverCounter < 10)
					{					
							if(LD9900MT_GenChar(0x01,600) == 0x00){											//�������������� CharBuffer 1							
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
					/*�ڶ���¼ȡָ��*/
				if((CMD_SecondRecordFinger[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SecondRecordFinger[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))/*��һ��¼ȡָ��*/
				{
						Delay_ms(100);
						TimeOverCounter = 0;//����������
					while(1 == SC50x0B_Busy());
					SC50x0B_Out(PUTFINGERAGAIN);
						while(LD9900MT_GetImage(600) != 0x00 & TimeOverCounter < 10){						//һֱ�ȴ���ȡָ��ͼ��ɹ�	 ��ೢ��10��
						SecondState = 0;																				//��״̬λΪ0
						TimeOverCounter++;	
						Buzzer_Beep(BuzzerTimeNormal);
						}
						if(TimeOverCounter < 10){
						Buzzer_Beep(BuzzerTimeLong);
						SecondState = 1;
						if(LD9900MT_GenChar(0x02,600) == 0x00){									//�������������� CharBuffer 2	
						if(FirstState == 1 & SecondState == 1 & LD9900MT_Search(0x02,0x0000,1000,FingerStoreNumberBuffer) != 0x00){
						FirstState = 0;	
						SecondState = 0;
								if(LD9900MT_CreateTemplate() == 0x00){									//����ģ��
										if(LD9900MT_Store(0x02,LD9900MT_SearchEmpty()) == 0x00){						//����ģ���CharBuffer1��CharBuffer2�е�������һ���ģ�����ģ�嵽�����1
											//FLASH_W25Q64_Write (Search_IdOrErWeiMa_Info(UserId,0x03)+6, FingerStoreNumber, 2);
//											ItemCounter = FLASH_W25Q64_ByteRead(FingerId_AddrTable[0][0])*256+FLASH_W25Q64_ByteRead(FingerId_AddrTable[0][0]+1);//��ȡ�����򴢴����Ϣ����
//											FLASH_W25Q64_Update (FingerId_AddrTable[0][0]+(ItemCounter-1)*10+36, FingerStoreNumber, 2);//����������Ϣ
//									FLASH_W25Q64_Read(CommonDataBuffer,FingerId_AddrTable[0][0],600);//���뻺����
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

//�ظ���λ��
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




//������Ϣ
unsigned char Store_IdOrErWeiMa_Data(unsigned char *Data,UCHAR SecletFlag){
UINT 	i=0;
UINT 	ItemCounter=0;//��������û���Ϣ����
UCHAR	ItemCounterBuffer[2] = {0},Cursor = 0;
UCHAR TempIdBuffer[20]={0};
UCHAR StoreErWeiMaBuffer[10]={0};
UCHAR StoreFingerIdBuffer[10]={0};
UCHAR AttachData[6]={0xff,0xff,0xff,0xff,0xff,0xff};
//���֤
if(SecletFlag == 0x01){
for(i=0;i<16;i++)
	{
		TempIdBuffer[i] = *Data++;	//�ݴ��һ����Ϣ
	}
	Arraycat(TempIdBuffer,AttachData,16,4);	//��Ӹ�����Ϣ
	Cursor = TempIdBuffer[8];
	if(Search_IdOrErWeiMa_Info(TempIdBuffer,1) == 0){//��Ϣ�����������
		ItemCounter = FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
		 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
	//�������֤У����10�ַ��ഢ�治ͬ������
		if(ItemCounter == 0xffff)//��ʼ������
		ItemCounter = 0;

		FLASH_W25Q64_Write ( Id_StoreAddrTable[Cursor&= 0x0f][0]+ItemCounter*20+60,TempIdBuffer, 20);//��β��׷����Ϣ ÿ������ǰ60�ֽ�Ԥ��

		ItemCounter ++;//��������1
	//	SBUF = 0xa6;while(!(SCON&0x02));SCON &= ~0x02;
		ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
		ItemCounterBuffer[1] = ItemCounter & 0x00ff;
		FLASH_W25Q64_Update (Id_StoreAddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//����������Ϣ
//				SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
//				FLASH_W25Q64_Read(CommonDataBuffer,Id_StoreAddrTable[Cursor&= 0x0f][0],600);//���뻺����
//			for(i=0;i<600;i++){
//			 SBUF = CommonDataBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();_nop_();
//			}
//	 SBUF = 0xa7;while(!(SCON&0x02));SCON &= ~0x02;
	}else{
		return 1;
	}
	return 1;
}else if(SecletFlag == 0x00)
//��ά��
{
for(i=0;i<6;i++)
	{
		StoreErWeiMaBuffer[i] = *Data++;	//�ݴ��һ����Ϣ
	}
		Arraycat(StoreErWeiMaBuffer,AttachData,6,4);	//��Ӹ�����Ϣ
		if(Search_IdOrErWeiMa_Info(StoreErWeiMaBuffer,0x00) == 0){//��Ϣ�����������
		Cursor = StoreErWeiMaBuffer[5] ;
//		for(i=0;i<6;i++){
//		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}

	ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
//	 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
//����У����16�ַ��ഢ�治ͬ������
	if(ItemCounter == 0xffff)//��ʼ������
	ItemCounter = 0;

					for(i=0;i<10;i++){
		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
			}
					
	FLASH_W25Q64_Write (ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30, StoreErWeiMaBuffer, 10);//��β��׷����Ϣ ÿ������ǰ30�ֽ�Ԥ��
	ItemCounter ++;//��������1
//	SBUF = 0xa6;while(!(SCON&0x02));SCON &= ~0x02;
	ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
	ItemCounterBuffer[1] = ItemCounter & 0x00ff;
	FLASH_W25Q64_Update (ErWeiMa_AddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//����������Ϣ
//	 SBUF = 0xa7;while(!(SCON&0x02));SCON &= ~0x02;
	//	FLASH_W25Q64_Write ( ErWeiMa_AddrTable[1][0]+5*10+30,erweima, 10);
//	memset(CommonDataBuffer,0x00,600);
		FLASH_W25Q64_Read(CommonDataBuffer,ErWeiMa_AddrTable[Cursor&= 0x0f][0],600);//���뻺����
			 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
		for(i=0;i<600;i++){
		 SBUF = CommonDataBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();_nop_();
		}
		return 1;
	}else{
		return 1;
	}
	return 1;
}//�û�ID
else if(SecletFlag == 0x03)
{
for(i=0;i<4;i++)
	{
		StoreFingerIdBuffer[i] = *Data++;	//�ݴ��һ����Ϣ
	}
		if(Search_IdOrErWeiMa_Info(StoreFingerIdBuffer,0x03) == 0){//��Ϣ�����������
		Arraycat(StoreFingerIdBuffer,AttachData,4,6);	//��Ӹ�����Ϣ
		Cursor = StoreFingerIdBuffer[0]+StoreFingerIdBuffer[1]+StoreFingerIdBuffer[2]+StoreFingerIdBuffer[3] ;
//		for(i=0;i<6;i++){
//		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}

	ItemCounter = FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor&= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
//	 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
//����У����16�ַ��ഢ�治ͬ������
	if(ItemCounter == 0xffff)//��ʼ������
	ItemCounter = 0;

//					for(i=0;i<10;i++){
//		S4BUF = StoreFingerIdBuffer[i];while(!(S4CON&0x02));S4CON &= ~0x02;_nop_();
//			}
					
	FLASH_W25Q64_Write (FingerId_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30, StoreFingerIdBuffer, 10);//��β��׷����Ϣ ÿ������ǰ30�ֽ�Ԥ��
	ItemCounter ++;//��������1
//	SBUF = 0xa6;while(!(SCON&0x02));SCON &= ~0x02;
	ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
	ItemCounterBuffer[1] = ItemCounter & 0x00ff;
	FLASH_W25Q64_Update (FingerId_AddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//����������Ϣ
//	 SBUF = 0xa7;while(!(SCON&0x02));SCON &= ~0x02;
//	memset(CommonDataBuffer,0x00,600);
//		FLASH_W25Q64_Read(CommonDataBuffer,FingerId_AddrTable[Cursor&= 0x0f][0],600);//���뻺����
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


//�������֤��Ϣ �������ڵ�ַ����0��û����������Ϣ��
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
		ItemCounter = FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor &= 0x0f][0])*256+FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor &= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
			if(ItemCounter == 0xffff)
				ItemCounter = 0;
		for(CurrentAddr = Id_StoreAddrTable[Cursor&= 0x0f][0];CurrentAddr<Id_StoreAddrTable[Cursor&= 0x0f][0]+ItemCounter*20+60;CurrentAddr+=600){
				 FLASH_W25Q64_Read(CommonDataBuffer,CurrentAddr,600);//���뻺����
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
		ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor &= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor &= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
			if(ItemCounter == 0xffff)
				ItemCounter = 0;
			for(CurrentAddr = ErWeiMa_AddrTable[Cursor&= 0x0f][0];CurrentAddr<ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30;CurrentAddr+=600){
				 FLASH_W25Q64_Read(CommonDataBuffer,CurrentAddr,600);//���뻺����
				 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
				 Result_Addr = CommonDataBuffer;
					for(i=0;i<600;i+=10){
						if(Bytencmp(Result_Addr,TempErWeiMaBuffer,6) != 0){
						Result_Addr+=10;
						Comparei++;//��¼����
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
		ItemCounter = FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor &= 0x0f][0])*256+FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor &= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
			if(ItemCounter == 0xffff)
				ItemCounter = 0;
			for(CurrentAddr = FingerId_AddrTable[Cursor&= 0x0f][0];CurrentAddr<FingerId_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30;CurrentAddr+=600){
				memset(CommonDataBuffer,0x00,600);
				FLASH_W25Q64_Read(CommonDataBuffer,CurrentAddr,600);//���뻺����
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
						Comparei++;//��¼����
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
�������ƣ�Bytencmp
�������ܣ��ȶ�
��ں�����*src  *dest  len
�����������
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

//ɾ�����֤��Ϣ  ����0��ɾ��ʧ�ܣ�����1��ɾ���ɹ���
UCHAR Delete_IdOrErWeiMa_Info(UCHAR *IdData,UCHAR SecletFlag){
	UINT 	ItemCounter=0;//��������û���Ϣ����
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
		ItemCounter = FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
		TempAddr = Search_IdOrErWeiMa_Info(TempIdData,1);
		if(TempAddr == 0){
		return 1;
		}else{
			FLASH_W25Q64_Read(TempFlashBuffer,Id_StoreAddrTable[Cursor&= 0x0f][0]+ItemCounter*20+60,20);
			FLASH_W25Q64_Update (TempAddr, TempFlashBuffer, 20);//��β����һ����Ϣ׷�ӵ�ɾ����λ��
			memset(TempFlashBuffer,0xff,20);
			FLASH_W25Q64_Update (Id_StoreAddrTable[Cursor &= 0x0f][0]+ItemCounter*20+60,TempFlashBuffer , 20);//ɾ��ĩβ��һ����Ϣ
			ItemCounter--;//������1
			ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
			ItemCounterBuffer[1] = ItemCounter & 0x00ff;
			FLASH_W25Q64_Update (Id_StoreAddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//����������Ϣ
			return 1;
		}
	}else if(SecletFlag == 0x00){
		for(i=0;i<6;i++)
		DelErWeiMaBuffer[i] = *IdData++;
		Cursor  = DelErWeiMaBuffer[5];
		ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
		TempAddr = Search_IdOrErWeiMa_Info(DelErWeiMaBuffer,0);
		if(TempAddr == 0){
		return 1;
		}else{
			FLASH_W25Q64_Read(TempFlashBuffer,ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30,10);
			FLASH_W25Q64_Update (TempAddr, TempFlashBuffer, 10);//��β����һ����Ϣ׷�ӵ�ɾ����λ��
			memset(TempFlashBuffer,0xff,10);
			FLASH_W25Q64_Update ((ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30),TempFlashBuffer , 10);//ɾ��ĩβ��һ����Ϣ
			ItemCounter--;//������1
			ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
			ItemCounterBuffer[1] = ItemCounter & 0x00ff;
			FLASH_W25Q64_Update (ErWeiMa_AddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//����������Ϣ
			return 1;
		}
	}else if(SecletFlag == 0x03){
		for(i=0;i<4;i++)
		DelFingerIdBuffer[i] = *IdData++;
		Cursor = DelFingerIdBuffer[0]+DelFingerIdBuffer[1]+DelFingerIdBuffer[2]+DelFingerIdBuffer[3] ;
		ItemCounter = FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(FingerId_AddrTable[Cursor&= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
		TempAddr = Search_IdOrErWeiMa_Info(DelFingerIdBuffer,3);
		if(TempAddr == 0){
		return 1;
		}else{
			FLASH_W25Q64_Read(TempFlashBuffer,FingerId_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30,10);
			FLASH_W25Q64_Update (TempAddr, TempFlashBuffer, 10);//��β����һ����Ϣ׷�ӵ�ɾ����λ��
			memset(TempFlashBuffer,0xff,10);
			FLASH_W25Q64_Update ((FingerId_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30),TempFlashBuffer , 10);//ɾ��ĩβ��һ����Ϣ
			ItemCounter--;//������1
			ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
			ItemCounterBuffer[1] = ItemCounter & 0x00ff;
			FLASH_W25Q64_Update (FingerId_AddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//����������Ϣ
			return 1;
		}
	}
	return 0;
}



/*****************************************************************************/
/*���֤�շ��������*/
/*�����鵱�յ���Ч֡�������һ���������                                     */
/*��ڲ�������												 */
/*   ����ֵ�� ���ͣ�Ϊ��ǰDataSortBuffer������Ч���ݵĸ���                    */
/*֡�ṹΪ ֡ͷ ���ݳ��� Ŀ���ַ ���к� ���� ���� У��� ֡β               */
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
				if( Uart4FindCartACK[6] == 0x04){//Ѱ��ʧ��
	//		SBUF=Uart4ACKCounter>>8;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
	//		SBUF=Uart4ACKCounter;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
//			for(i=0;i<Uart4ACKCounter;i++){
//				SBUF=Uart4FindCartACK[i];
//				while(!(SCON&0x02));
//				SCON &=~0x02;
//				_nop_();
//			}
		}else if(Uart4FindCartACK[6] == 0x08){//Ѱ���ɹ�
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
				USART4_Send(SELECT_CARD,sizeof(SELECT_CARD));//ѡ��
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
					if( Uart4FindCartACK[6] == 0x04){//ѡ��ʧ��

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
			}else if(Uart4FindCartACK[6] == 0x0c){//ѡ���ɹ�
			UserCardReadState = STATE_READCARD;
//				SBUF=0x05;while(!(SCON&0x02));SCON &=~0x02;
//				for(i=0;i<Uart4ACKCounter;i++){
//				SBUF=Uart4FindCartACK[i];while(!(SCON&0x02));SCON &=~0x02;

//			}
//					SBUF=0xff;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
				Uart4ACKCounter = 0;
				USART4_Send(READ_CARD,sizeof(READ_CARD));//����
					
		}
	}
			
		if(UserCardReadState == STATE_READCARD){
//				for(i=0;i<Uart4ACKCounter;i++){
//				SBUF=Uart4FindCartACK[i];while(!(SCON&0x02));SCON &=~0x02;
//			}
//				SBUF=0x19;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
			Delay_ms(500);
					if( Uart4FindCartACK[6] == 0x04){//����ʧ��
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
			}else if(Uart4FindCartACK[6] == 0x08){//�����ɹ�
				
//				SBUF=0x05;while(!(SCON&0x02));SCON &=~0x02;
				for(j=0,i=136;j<18,i<172;j++,i+=2){
				IdNumberAscii[j]=Uart4FindCartACK[i];
			}
					for(i=0;i<8;i++){
					 Id_BCD_Buffer[i] = (IdNumberAscii[i*2]-0x30)<<4;
					 Id_BCD_Buffer[i] |= (IdNumberAscii[i*2+1]-0x30);
					}
				if(IdNumberAscii[17] == 0x58){//ĩβΪX��������ת��ΪBCD 1010��ʮ����10  ʮ������Ϊ0x0a
				Id_BCD_Buffer[8]  = (IdNumberAscii[16] - 0x30)<<4;
				Id_BCD_Buffer[8] |= 0x0a;
			}else{
				Id_BCD_Buffer[8]  = (IdNumberAscii[16] - 0x30)<<4;
				Id_BCD_Buffer[8] |= (IdNumberAscii[17] - 0x30);
			}
			
//			for(j=0;j<9;j++){
//			SBUF=Id_BCD_Buffer[j];while(!(SCON&0x02));SCON &=~0x02;
//			}
			Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_UploadUerID,Id_BCD_Buffer,9,0));//��ˮ�����ϴ����֤
			if(Search_IdOrErWeiMa_Info(Id_BCD_Buffer,1)!=0){
				DoorTrigger = 1;
				Buzzer_Beep(BuzzerTimeNormal);
			}else {		
			Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_SearchUerID,Id_BCD_Buffer,9,1));//�����°������ο���Ϣ
			}

						UserCardReadState = STATE_NOCARD;
						Flag_FindCard_500msOver=0;
		}
		
		}
//	SBUF=0x39;while(!(SCON&0x02));SCON &=~0x02;	_nop_();
	Uart4ACKCounter = 0;
}











/***********************************************
�������ƣ�Op_Finger_Module
�������ܣ���ѯָ��ģ��ʵ���ѻ���֤
��ں�������
�����������
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
{											//һֱ�ȴ���ȡָ��ͼ��ɹ�
	State = 0;																			//��״̬λΪ0
	cnt++;			
}
if(cnt < 1)
	{
//		SBUF = 0x08;while(!(SCON&0x02));SCON &= ~0x02;
	if(LD9900MT_GenChar(0x01,550) == 0x00)											//�������������� CharBuffer 1
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
	ShieldOpenOrClose=1;/*����ǿ�ƿ�բ��������ο�բ*/
	//Search_IdOrErWeiMa_Info(FingerStoreNumber,0x03)+6
	//FLASH_W25Q64_Read(TempFingerId,SearchFingerStoreNumber(FingerStoreNumber),4);
	//Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_FingerUploadStreamInfo,&TempFingerId,4,0));//��ˮ����ָ�� �˾������ڷ�����ǰ�棬�������
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
			FLASH_W25Q64_Read(TempBuffer,i+6,2);//���뻺����
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
	/*�������ݽ��յ��ļ��ܶ�ά�����ݣ�����������Ϊ��eaf2c850402470500c1aa8*/
	//EWMBuffer1[0]='e';EWMBuffer1[1]='a';EWMBuffer1[2]='f';EWMBuffer1[3]='2';EWMBuffer1[4]='c';EWMBuffer1[5]='8';
	//EWMBuffer1[6]='5';EWMBuffer1[7]='0';EWMBuffer1[8]='4';EWMBuffer1[9]='0';EWMBuffer1[10]='2';EWMBuffer1[11]='4';
	//EWMBuffer1[12]='7';EWMBuffer1[13]='0';EWMBuffer1[14]='5';EWMBuffer1[15]='0';EWMBuffer1[16]='0';EWMBuffer1[17]='c';
	//EWMBuffer1[18]='1';EWMBuffer1[19]='a';EWMBuffer1[20]='a';EWMBuffer1[21]='8';
	for(i=0;i<22;i++)/*�����յ�������ת��Ϊʮ����������*/
	  {SBUF=EWMBuffer1[i];while(!(SCON&0X02));SCON &=~0X02;
	   if(EWMBuffer1[i]>0x60) {EWMBuffer1[i]= EWMBuffer1[i]-0x57;}
	   else {EWMBuffer1[i]= EWMBuffer1[i]-0x30;}
	    }
	for(i=0;i<11;i++)/*�ϳ����ݲ�����*/
	  {EWMBuffer[i]= EWMBuffer1[2*i]*16+EWMBuffer1[2*i+1];/*������������ݺϳ�ʮ����������*/
	   EWMBuffer[i]=EWMBuffer[i]^EWMBuffer_Mima[i];}/*��������*/
	for(i=0;i<11;i++) CS+=EWMBuffer[i];

//��������:���������ƴ��
//��ں���: Data_Dest			ԭ����
//				 Data_Src				Ҫ��ӵ�����
//				 DestDataNum  	ԭ���������
//				 SrcDataNum 		Ҫ��ӵ�����
//���ں���:��
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
	Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_UploadErWeiMa,ErWeiMaBCDBuffer,6,0));//��ˮ�����ϴ���ά��
	if(Search_IdOrErWeiMa_Info(ErWeiMaBCDBuffer,0) != 0){
					DoorTrigger = 1;
					Buzzer_Beep(BuzzerTimeNormal);
	}else {		
	Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_SearchErWerMa,ErWeiMaBCDBuffer,6,1));//�����°������ο���Ϣ
	}
						EWMCounter = 0;Flag_ErWeiMaReceiveStart=0;
}


/***********************************************
�������ƣ�Uart3_DATA_deal
�������ܣ��������ô�����
��ں�������
�����������
***********************************************/

void Uart3_DATA_deal(void){
unsigned char DatBuffer = 0x00;
	Delay_ms(10);
	
 if((CMD_nReloadWifi[0] ==  BluetoothRecBuffer[0]) && (CMD_nReloadWifi[1] ==  BluetoothRecBuffer[1])){
	Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_nReloadWifi,&DatBuffer,1,1));
 }
else if((CMD_SetSystemId[0] ==  BluetoothRecBuffer[0]) && (CMD_SetSystemId[1] ==  BluetoothRecBuffer[1]))
		{	
			FLASH_W25Q64_Update (System_SetParaAddr, &BluetoothRecBuffer[2], sizeof(System_ID));//����ϵͳID��Ϣ
			Delay_ms(500);
			Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_SetSystemId, &BluetoothRecBuffer[2],6,1));
			Delay_ms(2000);
			Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_ReStart,&DatBuffer,1,1));
			IAP_CONTR = 0x20;
		}
else if((CMD_SetDoorOpenTime[0] ==  BluetoothRecBuffer[0]) && (CMD_SetDoorOpenTime[1] ==  BluetoothRecBuffer[1]))
		{	
			FLASH_W25Q64_Update (System_SetParaAddr+10, &BluetoothRecBuffer[2], 2);//����ϵͳ����ʱ��
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
/*�������ݷ��ͳ���   														 */
/*��ڲ�����data �����͵����ݻ���                                            */
/*          len ���������ݵĸ���                                             */
/*          UATR_Num   ʹ�õĴ��ں�                                          */
/*   ����ֵ�� ��                                                             */
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
�������ƣ�Muti_Send_Byte
�������ܣ�����1���ֽڵ����ݣ�
��ں�����
���ں������յ����ַ�
********************************************************/
char Muti_Read_Byte(void)
{  
unsigned char dat;
	
dat = P2;

P1M1 &= ~(1<<2);									//CHACK��ʼ��Ϊ���	
P1M0 |= (1<<2);
CHACK = 1;
_nop_();
CHACK = 0;
return dat;
} 


/*******************************************************
�������ƣ�Muti_Send_Byte
�������ܣ�����������ʼһ�λỰ���������ݣ�
��ں�����Ҫת�͵�����Data  ���ݳ���len
���ں�������
********************************************************/
void Muti_Send_Byte(unsigned char *Data,unsigned int len)
{ 
unsigned int i=0; 
//SBUF=0xbb;while(!(SCON&0X02));SCON &=~0X02;    

if(!CHKREQ)
{
REQ = 1;															//��REQΪ��Ч
if(!CHKREQ)
{
	P2M1 = 0x00;												//P2Ϊ���
	P2M0 = 0xff;
	P1M1 |= (1<<2);
	P1M0 &= ~(1<<2);										//CHACK��ʼ��Ϊ����
	while(i<len)
		{
			Delay_us(100);			
			P2 = *(Data+i);									//�������͵�P2�˿�
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
P2M0 = 0x00;		//��ʼ��P2Ϊ����
}

/********************************************************
��������:Arraycat
��������:���������ƴ��
��ں���: Data_Dest			ԭ����
				 Data_Src				Ҫ��ӵ�����
				 DestDataNum  	ԭ���������
				 SrcDataNum 		Ҫ��ӵ�����
���ں���:��
��ע:��
********************************************************/
void Arraycat(unsigned char *Data_Dest,unsigned char *Data_Src,unsigned int DestDataNum,unsigned int SrcDataNum)
 {unsigned int i;
  for(i=0;i<SrcDataNum;i++)
   {*(Data_Dest+DestDataNum+i)=*(Data_Src+i);} 
  }
/*****************************************************************************
��������֡��֯���� 
��������:Fe_data_Organize
��ڲ�����	CMD ��ǰ����������                                                      
					Data ��ǰ�Ĳ�������                                       
          DataNum   ��ǰ�Ĳ������ݵĸ���                                    					 
����ֵ�� ���ͣ�Ϊ��ǰUART1_RE_DATA������Ч���ݵĸ���                  
��ע:������֯��ͨ�ó���ֻ�ô�����صĲ������Լ��ϳɷ���֡������ṹ������� 
	֡�ṹΪ ֡ͷ ���ݳ��� Ŀ���ַ ���к� ���� ���� У��� ֡β            
*****************************************************************************/
unsigned int Fe_data_Organize(unsigned char CMD[],unsigned char Data[],unsigned int DataNum,bit OrgType)
 { 
	unsigned char Organize_DataBuffer[2]={0};
	unsigned int i=0;
	unsigned int StartPointer0=0,StartPointer1=0; 
	/************************************************************************/
	/*��֯֡*/
	/*��֯֡ͷ*/
		if(OrgType){
	StartPointer1=2;
	Arraycat(&CommonDataBuffer[0],&FE_START[0],StartPointer0,StartPointer1);/*��֯֡ͷ*/
	}else if(!OrgType){
	StartPointer1=2;
	Arraycat(&CommonDataBuffer[0],&FE_START_ACK[0],StartPointer0,StartPointer1);/*��֯֡ͷ*/
	}
	StartPointer0=StartPointer0+StartPointer1;
	Organize_DataBuffer[0]=(DataNum&0xff00)>>8;/*׷�����ݳ���*/
	Organize_DataBuffer[1]=DataNum&0xff;
	StartPointer1=2;
	Arraycat(&CommonDataBuffer[0],&Organize_DataBuffer[0],StartPointer0,StartPointer1);
	StartPointer0=StartPointer0+StartPointer1;
	StartPointer1=6;
	Arraycat(&CommonDataBuffer[0],&System_ID[0],StartPointer0,StartPointer1);/*׷������ID*/
	StartPointer0=StartPointer0+StartPointer1;
	CommonDataBuffer[StartPointer0]=UART_Fe_Seq;
	StartPointer0++;
	StartPointer1=2;
	Arraycat(&CommonDataBuffer[0],&CMD[0],StartPointer0,StartPointer1);/*׷�������*/
	StartPointer0=StartPointer0+StartPointer1;
	if(DataNum>0) 
	 {StartPointer1=DataNum;
	  Arraycat(&CommonDataBuffer[0],&Data[0],StartPointer0,StartPointer1);/*׷������*/
	  StartPointer0=StartPointer0+StartPointer1; }
	 Organize_DataBuffer[0]=0;
	for(i=0;i<StartPointer0;i++)/*����У���*/
		Organize_DataBuffer[0]=Organize_DataBuffer[0]+CommonDataBuffer[i];
//		S4BUF =StartPointer0>>8;while(!(S4CON&0x02));S4CON &=~0x02;_nop_();
//		S4BUF =StartPointer0;while(!(S4CON&0x02));S4CON &=~0x02;_nop_();
//		S4BUF =Organize_DataBuffer[0];while(!(S4CON&0x02));S4CON &=~0x02;_nop_();
		Organize_DataBuffer[1]=0x16;
		Arraycat(&CommonDataBuffer[0],&Organize_DataBuffer[0],StartPointer0,2);/*׷��У��ͺ�֡β*/
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


