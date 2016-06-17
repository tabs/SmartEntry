#include "flashfunction.h"
#include "init.h"
#include <string.h>
#define FE_MESSTYPEADDR 11 /*��Ϣ�����ֽ���֡�е���ʼλ��*/
#define FE_DATAADDR 13 /*�����ֽ���֡�е���ʼλ��*/
#define FE_CODENUM 10 //֡����ֽ���֡�е���ʼλ��
#define FE_LENGTH 2 //�������ݳ�����֡�е�λ��
#define SYSTEM_ID 4 //���屾�������֡�е�λ��
#define HeartbeatTime 1000*4					  //������ʱ��40s
#define BuzzerTimeNormal	10						//����һ�η��������ʱ�䣨����100ms��
#define BuzzerTimeLong 100							//����һ�η��������ʱ�䣨����1000ms��
#define BuzzerTimeShort 2								//���ٷ�������20ms��
#define ResetWifiTime 6000*8						//�ͷ�����ͨ�ų�ʱWIFI����ʱ��60*5s
#define Buzzer P36											//���������Ŷ���
#define nReloadWifi P37									//WIFI�ָ������������ţ���Ҫ���͵�ƽ����1s��
#define nResetWifi P07									//WIFI�������ţ���Ҫ���͵�ƽ����200MS��
#define SysCordAppUpdataAddrFlag 0x7E00
#define SysCordAppSaveStartTempAddr 0x8000 /*����Ӧ�����ݴ����ʼ��ַ*/
unsigned char const ReplySucc = 0x31;		
unsigned char const ReplyFail = 0x30;
unsigned int HeartbeatPacketCounter = 0;
unsigned int BuzzerCounter = 0;
unsigned int ResetWifiCounter = 0;
int  BuzzerTime = BuzzerTimeNormal;							  //��ʼ������һ�η��������ʱ�䣨����100ms��
bit HeartbeatTrigger;
bit BuzzerTriggrt;
bit ResetWifiTrigger;
bit ResetWifiOnOff;
bit ENTM;						//WIFIģ��͸���ʹ�������ģʽ�л�λ
bit Com1HaveDate;
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

unsigned int Exint0Counter = 0;						//�ⲿ�ж�0�������ݼ�����
unsigned char Exint0DataBuffer[600] = {0};
unsigned char Com1DataBuffer[600] = {0};
unsigned int Com1DataCounter = 0;
unsigned char System_ID[6]={0x03,0x71,0x00,0x01,0x00,0x01};
const unsigned char code WifiInterSerialConfigModeCMD[4] = {'+','+','+','a'};/*+++ a*/
const unsigned char code WifiInterTmModeCMD[8] = {'A','T','+','E','N','T','M',0x0d};/*AT+ENTM<CR>*/
const unsigned char code AppVersionNum[4] = {0x20,0x16,0x06,0x14};/*բ������汾���V2016.6.01*/
const unsigned char code FE_START[2]={0x55,0xaa};//֡ͷ
const unsigned char code FE_START_ACK[2]={0xaa,0x55};//�ظ���λ����֡ͷ
const unsigned char code CMD_Heartbeat[2]={0x00,0x01};/*����������*/
//const unsigned char code CMD_ServerAck[2]={0x01,0x01};/*�ͷ�����ͨ�ų�ʱ����WIFI*/
const unsigned char code CMD_ReStart[2]={0x00,0x02};/*��������*/
const unsigned char code CMD_UpdateSystem[2]={0x00,0x06};/*��������*/
const unsigned char code CMD_DownLoadUerID[2]={0x30,0x01};/*�������֤��Ϣ*/
const unsigned char code CMD_DownLoadErWerMa[2]={0x40,0x01};/*���ض�ά����Ϣ*/
const unsigned char code CMD_DeleteErWeiMa[2]={0x40,0x02};/*ɾ��������ά����Ϣ*/
const unsigned char code CMD_DeleteID[2]={0x30,0x02};/*ɾ���������֤��Ϣ*/
const unsigned char code CMD_DeleteAllErWeiMa[2]={0x40,0x03};/*ɾ�����ж�ά����Ϣ*/
const unsigned char code CMD_DeleteAllID[2]={0x30,0x03};/*ɾ���������֤��Ϣ*/
const unsigned char code CMD_SearchUerID[2]={0x80,0x01};/*�����ο����֤��Ϣ*/
const unsigned char code CMD_SearchErWerMa[2]={0x70,0x01};/*�����οͶ�ά����Ϣ*/
const unsigned char code CMD_AskAppVersionNum_Top[2]={0x60,0x01};/*��ѯբ���ϰ�����汾��*/
const unsigned char code CMD_AskAppVersionNum_Bottom[2]={0x60,0x02};/*��ѯբ���°�����汾��*/
const unsigned char code CMD_WifiSerialConfigMode[2]={0xf8,0x01};/*����WIFI��������ģʽ����*/
const unsigned char code CMD_Buzzer[2]={0xff,0xff};/*����������*/
const unsigned char code CMD_DoorTrigger[2]={0xfe,0xfe};/*�ڲ���բ����*/
const unsigned char code CMD_SetSystemId[2]={0xff,0x01};/*բ��ID��������*/
const unsigned char code CMD_nReloadWifi[2]={0xff,0x02};/*բ������WIFI����*/
unsigned char UART_Fe_Seq  = 0;						//֡���
unsigned char CommonDataBuffer[600]={0};	//���ݻ�������
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
//	Muti_Send_Byte(Com1DataBuffer,Com1DataCounter);		//����λ��������ֱ���·�������ĵ�Ƭ��
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
{	/*����־���ֹ��û���յ������������������WiFi�Ĺ���*/
	//nResetWifi = 0;   
	//Delay_ms(500);
	//nResetWifi = 1;
	//ResetWifiOnOff  = 0;//�ر�WIFI���ÿ��أ�WIFI�����Ժ󣬱���ر����ÿ��ط�ֹWIFIģ�����ӷ������������ٴ�������
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
/* ��ʱ��0�ж���ں���					            */
/* �����ܣ���ʱ��0�жϴ�����*/
/* ��ڲ���:��										*/
/* ���ڲ���:��										*/
/****************************************************/
void Timer0_intrupt_function(void) interrupt 1 using 1
{ 
	//�������趨
	if(HeartbeatTrigger == 0)
 {
	 HeartbeatPacketCounter++;
		if(HeartbeatPacketCounter > HeartbeatTime){
		HeartbeatTrigger = 1;
		HeartbeatPacketCounter = 0;
		}
 }
	 //�������趨
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
	 	//������ѯ�ʳ�ʱ
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
	if(SCON&0x01) 		 /*����*/
		{
				dat = SBUF;
				P34 = 0;			//�����ϵ�ָʾ��
				SCON &=~0x01;
			if(ENTM)				//͸��ģʽ
			{
				if(!CHKREQ)
				{
				REQ = 1;															//��REQΪ��Ч
				if(!CHKREQ)
				{
					P2M1 = 0x00;												//P2Ϊ���
					P2M0 = 0xff;
					P1M1 |= (1<<2);
					P1M0 &= ~(1<<2);										//CHACK��ʼ��Ϊ����
					P2 = dat;														//�������͵�P2�˿�
					Delay_us(10);
					STB = ~STB;
					while(CHACK);
				REQ = 0;		
				}else REQ = 0;
				}
				REQ = 0;
				P2M1 = 0xff;
				P2M0 = 0x00;		//��ʼ��P2Ϊ����						
			}else
					{			
						Com1HaveDate = 1;			
						Com1DataBuffer[Com1DataCounter++] = dat;
					}

		}			
}

void com2_isr(void) interrupt 8 using 1
{ 
  if(S2CON&0X01)  /*����*/
   {S2CON &=~0X01;}			
}

void com3_isr(void) interrupt 17 using 1
{
	if(S3CON&0X01)  /*����*/
	{	S3CON &=~0X01;}								   
}

void com4_isr(void) interrupt 18 using 1
{
	if(S4CON&0X01)  /*����*/
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
				FE_CS=DataLength+FE_DATAADDR;/*ָ��У�����λ��*/
		for(i=0;i<FE_DATAADDR+DataLength;i++)
		CS+=Exint0DataBuffer[i];

			for(i=0,j=0;i<6,j<6;i++,j++){
			if(System_ID[j] == Exint0DataBuffer[SYSTEM_ID]+i)															//�ж��Ƿ��Ǳ������
				flag++;
	}
//	SBUF = 0x8d;while(!(SCON&0x02));SCON &=~0x02;_nop_();
	if(CS == Exint0DataBuffer[FE_CS]){
//	for(i=0;i<Exint0Counter;i++){
//SBUF = Exint0DataBuffer[i];while(!(SCON&0x02));SCON &=~0x02;
//}
//	SBUF = 0xf9;while(!(SCON&0x02));SCON &=~0x02;
		UART_Fe_Seq = Exint0DataBuffer[FE_CODENUM];								//��¼֡���
		//������
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
			}//����WIFI��������ģʽ
			else if((CMD_WifiSerialConfigMode[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_WifiSerialConfigMode[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				WifiSeiralConfigMode();
				Reply_State(CMD_WifiSerialConfigMode,1);
			}		
			//������ѯ�ʳ�ʱ������WIFIģ��
			else if((CMD_Heartbeat[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_Heartbeat[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(ResetWifiOnOff == 0){
				ResetWifiOnOff  = 1;//��WIFI���ÿ���
				}
				ResetWifiCounter = 0;//�����������ͷ��ʼ��ʱ
				//Reply_State(CMD_ServerAck,1);
			}
			//��ѯ�ϰ�����汾��
			else if((CMD_AskAppVersionNum_Top[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_AskAppVersionNum_Top[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
					USART1_Send(Exint0DataBuffer,Exint0Counter);
			}
			//��ѯ�°�����汾��
			else if((CMD_AskAppVersionNum_Bottom[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_AskAppVersionNum_Bottom[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
				USART1_Send(CommonDataBuffer,Fe_data_Organize(CMD_AskAppVersionNum_Bottom,AppVersionNum,4,1));
			}//ɾ���������֤
			else if((CMD_DeleteAllID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1])){
				for(DeleAddr=Id_StoreAddrTable[0][0];DeleAddr<Id_StoreAddrTable[10][1];DeleAddr+=0x10000){
//					SBUF = 0x87;while(!(SCON&0x02));SCON &=~0x02;
					sFLASH_64KBEraseSector(DeleAddr);
//					SBUF = 0x88;while(!(SCON&0x02));SCON &=~0x02;
					
				}
					Reply_State(CMD_DeleteAllID,1);
			}
			//ɾ�����ж�ά��
			else if((CMD_DeleteAllErWeiMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DeleteAllErWeiMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
				{
					for(DeleAddr=ErWeiMa_AddrTable[0][0];DeleAddr<ErWeiMa_AddrTable[15][1];DeleAddr+=0x10000){
					sFLASH_64KBEraseSector(DeleAddr);
					}
					Reply_State(CMD_DeleteAllErWeiMa,1);
			}
				//�����ο����֤��Ϣ
			else if((CMD_SearchUerID[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SearchUerID[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				if(Search_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR],1)!=0){
				Delay_ms(100);
//				for(i=0;i<j;i++){
//					SBUF = CommonDataBuffer[i];while(!(SCON&0x02));SCON &=~0x02;_nop_();
//				}
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DoorTrigger,&CS,1,1));//�����°忪բ
				BuzzerTriggrt = 1;
			}

			}/*�����οͶ�ά����Ϣ*/
			else if((CMD_SearchErWerMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SearchErWerMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				if(Search_IdOrErWeiMa_Info(&Exint0DataBuffer[FE_DATAADDR],0)!=0){
				Delay_ms(100);
				Muti_Send_Byte(CommonDataBuffer,Fe_data_Organize(CMD_DoorTrigger,&CS,1,1));//�����°忪բ
				BuzzerTriggrt = 1;
			}

			}//����
			else if((CMD_ReStart[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_ReStart[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				BuzzerTriggrt = 1;
				Reply_State(CMD_ReStart,1);
				Delay_ms(1000);			
				IAP_CONTR = 0x20;
			}//����բ��ID��
			else if((CMD_SetSystemId[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_SetSystemId[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				if(Exint0DataBuffer[FE_DATAADDR] == 0x31){
				Reply_State(CMD_SetSystemId,1);
				}else{
					FLASH_W25Q64_Update (System_SetParaAddr, &Exint0DataBuffer[FE_DATAADDR], sizeof(System_ID));//����ϵͳID��Ϣ
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
					Delay_ms(500);//��λWIFIģ��
					Reply_State(CMD_nReloadWifi,1);
					BuzzerTriggrt = 1;
				}//����
			else if((CMD_UpdateSystem[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_UpdateSystem[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{	
				ResetWifiOnOff  = 0;//�����ر�WIFI����
				if(Exint0DataBuffer[FE_DATAADDR] == 0x01){
						if(Exint0DataBuffer[FE_DATAADDR+4]==Exint0DataBuffer[FE_DATAADDR+6]){//���һ��������
							if(sequential_write_flash_in_one_sector(SysCordAppSaveStartTempAddr+(Exint0DataBuffer[FE_DATAADDR+6]-1)*0x200, 512, &Exint0DataBuffer[FE_DATAADDR+9])){
							FLASH_PageErase(SysCordAppUpdataAddrFlag);
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag, 0x5A);//д������־
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag+1, 0xA5);
							FLASH_ByteWrite(SysCordAppUpdataAddrFlag+2, Exint0DataBuffer[FE_DATAADDR+4]);//��¼�������ݰ�����
							BuzzerTriggrt = 1;
							Reply_State(CMD_UpdateSystem,1);
							Delay_ms(1000);
							IAP_CONTR = 0x20;//������������
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
			}//�������֤				
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
			//ɾ�����֤
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
			//���ض�ά��
			else if((CMD_DownLoadErWerMa[0] ==  Exint0DataBuffer[FE_MESSTYPEADDR]) && (CMD_DownLoadErWerMa[1] ==  Exint0DataBuffer[FE_MESSTYPEADDR+1]))
			{
				if(Exint0DataBuffer[FE_DATAADDR] == 0x01){
					if(Store_IdOrErWeiMa_Data(&Exint0DataBuffer[FE_DATAADDR+1],0) == 0x01){
					Reply_State(CMD_DownLoadErWerMa,1);
					}
			}else {
					Reply_State(CMD_DownLoadErWerMa,1);
				}
		}//ɾ����ά��
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





//�ظ���λ��
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

//������Ϣ
unsigned char Store_IdOrErWeiMa_Data(unsigned char *Data,bit SecletFlag){
UINT 	i=0;
UINT 	ItemCounter=0;//��������û���Ϣ����
UCHAR	ItemCounterBuffer[2] = {0},Cursor = 0;
UCHAR TempIdBuffer[20]={0};
UCHAR StoreErWeiMaBuffer[10]={0};
UCHAR AttachData[4]={0xff,0xff,0xff,0xff};
//���֤
if(SecletFlag){
for(i=0;i<16;i++)
	{
		TempIdBuffer[i] = *Data++;	//�ݴ��һ����Ϣ
	}
	Arraycat(TempIdBuffer,AttachData,16,4);	//��Ӹ�����Ϣ
	Cursor = TempIdBuffer[8];
	if(Search_IdOrErWeiMa_Info(TempIdBuffer,1) == 0){//��Ϣ�����������
		ItemCounter = FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor&= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
//		 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
	//�������֤У����10�ַ��ഢ�治ͬ������
		if(ItemCounter == 0xffff)//��ʼ������
		ItemCounter = 0;

		FLASH_W25Q64_Write ( Id_StoreAddrTable[Cursor&= 0x0f][0]+ItemCounter*20+60,TempIdBuffer, 20);//��β��׷����Ϣ ÿ������ǰ60�ֽ�Ԥ��

		ItemCounter ++;//��������1
	//	SBUF = 0xa6;while(!(SCON&0x02));SCON &= ~0x02;
		ItemCounterBuffer[0] = (ItemCounter & 0xff00)>>8;
		ItemCounterBuffer[1] = ItemCounter & 0x00ff;
		FLASH_W25Q64_Update (Id_StoreAddrTable[Cursor&= 0x0f][0], ItemCounterBuffer, 2);//����������Ϣ
//						 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
				FLASH_W25Q64_Read(CommonDataBuffer,Id_StoreAddrTable[Cursor&= 0x0f][0],600);//���뻺����
//			for(i=0;i<600;i++){
//			 SBUF = CommonDataBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();_nop_();
//			}
//	 SBUF = 0xa7;while(!(SCON&0x02));SCON &= ~0x02;
	}else{
		return 1;
	}
	return 1;
}else if(!SecletFlag)
//��ά��
{
for(i=0;i<6;i++)
	{
		StoreErWeiMaBuffer[i] = *Data++;	//�ݴ��һ����Ϣ
	}
		Arraycat(StoreErWeiMaBuffer,AttachData,6,4);	//��Ӹ�����Ϣ
		if(Search_IdOrErWeiMa_Info(StoreErWeiMaBuffer,0) == 0){//��Ϣ�����������
		Cursor = StoreErWeiMaBuffer[5] ;
//		for(i=0;i<6;i++){
//		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}

	ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor&= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
//	 SBUF = 0xa5;while(!(SCON&0x02));SCON &= ~0x02;
//����У����16�ַ��ഢ�治ͬ������
	if(ItemCounter == 0xffff)//��ʼ������
	ItemCounter = 0;

//					for(i=0;i<10;i++){
//		SBUF = StoreErWeiMaBuffer[i];while(!(SCON&0x02));SCON &= ~0x02;_nop_();
//			}
					
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
//�������֤��Ϣ �������ڵ�ַ����0��û����������Ϣ��
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
		ItemCounter = FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor &= 0x0f][0])*256+FLASH_W25Q64_ByteRead(Id_StoreAddrTable[Cursor &= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
			if(ItemCounter == 0xffff)
				ItemCounter = 0;
		for(CurrentAddr = Id_StoreAddrTable[Cursor&= 0x0f][0];CurrentAddr<Id_StoreAddrTable[Cursor&= 0x0f][0]+ItemCounter*20+60;CurrentAddr+=600){
				 FLASH_W25Q64_Read(CommonDataBuffer,CurrentAddr,600);//���뻺����
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
		ItemCounter = FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor &= 0x0f][0])*256+FLASH_W25Q64_ByteRead(ErWeiMa_AddrTable[Cursor &= 0x0f][0]+1);//��ȡ�����򴢴����Ϣ����
			if(ItemCounter == 0xffff)
				ItemCounter = 0;
			for(CurrentAddr = ErWeiMa_AddrTable[Cursor&= 0x0f][0];CurrentAddr<ErWeiMa_AddrTable[Cursor&= 0x0f][0]+ItemCounter*10+30;CurrentAddr+=600){
				 FLASH_W25Q64_Read(CommonDataBuffer,CurrentAddr,600);//���뻺����
//				 SBUF = 0xa2;while(!(SCON&0x02));SCON &= ~0x02;
				 Result_Addr = CommonDataBuffer;
					for(i=0;i<600;i+=10){
						if(strncmp(Result_Addr,TempErWeiMaBuffer,6) != 0){
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
		}
memset(CommonDataBuffer,0x00,600);
return 0;
}



//ɾ�����֤��Ϣ  ����0��ɾ��ʧ�ܣ�����1��ɾ���ɹ���
UCHAR Delete_IdOrErWeiMa_Info(UCHAR *IdData,bit SecletFlag){
	UINT 	ItemCounter=0;//��������û���Ϣ����
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
	}else if(!SecletFlag){
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
	}
	return 0;
}



void WifiSeiralConfigMode(void)
{
	ENTM = 0;//��͸��ģʽ���ڽ���
	USART1_Send(&WifiInterSerialConfigModeCMD ,1 );
	Delay_ms(250);
	USART1_Send(&WifiInterSerialConfigModeCMD+1 ,1 );
	Delay_ms(250);
	USART1_Send(&WifiInterSerialConfigModeCMD+2 ,1 );
	
	Delay_ms(2000);
	USART1_Send(&WifiInterSerialConfigModeCMD+3 ,1 );
	Delay_ms(2000);
	USART1_Send(WifiInterTmModeCMD,sizeof(WifiInterTmModeCMD));//����͸��ģʽ
	ENTM = 1;//�ر�͸��ģʽ���ڽ���
}



///****************************************************/
///* ����1�жϴ�����					            */
///* �����ܣ�����1�жϴ�����                    */
///* ��ڲ���:��										*/
///* ���ڲ���:��										*/
///****************************************************/
//void Uart1_intrupt_function(void)
//{

//}



///****************************************************/
///* ����2�жϴ�����					            */
///* �����ܣ�����2�жϴ�����                    */
///* ��ڲ���:��										*/
///* ���ڲ���:��										*/
///****************************************************/
//void Uart2_intrupt_function(void)
//{					   

//}



///****************************************************/
///* ����3�жϴ�����					            */
///* �����ܣ�����3�жϴ�����                    */
///* ��ڲ���:��										*/
///* ���ڲ���:��										*/
///****************************************************/
//void Uart3_intrupt_function(void)
//{
//}





///****************************************************/
///* ����4�жϴ�����					            */
///* �����ܣ�����4�жϴ�����                    */
///* ��ڲ���:��										*/
///* ���ڲ���:��										*/
///****************************************************/
//void Uart4_intrupt_function(void)
//{
//}

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
/*****************************************************************************/
/*�������ݷ��ͳ���   														 */
/*��ڲ�����data �����͵����ݻ���                                            */
/*          len ���������ݵĸ���                                             */
/*          UATR_Num   ʹ�õĴ��ں�                                          */
/*   ����ֵ�� ��                                                             */
/*****************************************************************************/
void USART4_Send(unsigned char *dat,unsigned int len)
{  unsigned int i;

for(i=0;i<len;i++)	
{ 
	{S4BUF=*dat++;while(!(S4CON&0x02));S4CON &=~0x02;_nop_();_nop_();}
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
	
//		for(i=0;i<len;i++){
//					SBUF = *Data++;while(!(SCON&0x02));SCON &=~0x02;_nop_();
//				}
if(!CHKREQ)
{
REQ = 1;																//��REQΪ��Ч
if(!CHKREQ)
{
	P2M1 = 0x00;												//P2Ϊ���
	P2M0 = 0xff;
	P1M1 |= (1<<2);
	P1M0 &= ~(1<<2);															//CHACK��ʼ��Ϊ����
	while(i<len)
		{
			//SBUF=i;while(!(SCON&0x02));SCON &=~0x02;_nop_();_nop_();
			P2 = *(Data++);														//�������͵�P2�˿�
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
P2M0 = 0x00;		//��ʼ��P2Ϊ����
}
/********************************************************
��������:Arraycat
��������:���������ƴ��
��ں���: Data_Src		Ҫ��ӵ�����	Data_Dest	ԭ����
SrcDataNum Ҫ��ӵ����� DestDataNum  	ԭ���������
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
					OrgType	��֯������ 1Ϊͷ��55 AA 0Ϊͷ��AA 55
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

void Delay_us(UINT ms){
UINT k;
unsigned char i;
for(k=0;k<ms;k++){
	i = 3;
	while (--i);
}
}
