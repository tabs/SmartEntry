/*ds1302��ʾʱ�䣨ͨ�����ڵ������֣�*/
#include "DS1302.h"




void ds1302_reset()//��SCK=0��RST=1
{
  RST=0;//ֹͣ���ݴ���
  SCK=0;//ʱ����������
  RST=1;//���ݲ���
}

//дһ���ֽ�
void ds1302_write_byte(uint8 dat)
{
  uint8 i;
  
 for(i=0;i<8;i++)
 {
   SDA=dat & 0x01;
   SCK=1;
   SCK=0;
   dat>>=1;
 }
}

//��һ���ֽ�
//���������������ʼ��8 ��SCLK ���ڣ�����һ���������ֽڣ������ֽ��ں�8 ��SCLK���ڵ��½������
//ע�⣬��һ�������ֽڵĵ�һ���½��ط����������ֵ����һλ��д��
//��Note that the first data bit to be transmitted occurs on the first falling edge after the last bit of the command byte is written. ����
//Ҳ���������ֽ����һ���ֽ�д���������½���ʹ�õ�һ������λ����ȡ

uint8 ds1302_read_byte()
{
 uint8 i,dat=0;//��dat�洢����������
 for(i=0;i<8;i++)
 {
    
	dat>>=1;	
	if(SDA==1)
	  dat=dat | 0x80;//��Ϊ�ӵ�λ��ʼ��
	SCK=1;
	SCK=0;	//��������Ϳ���֪������������Ӧ���ж�	if(SDA==1)��Ȼ�����SCK=1��SCK=0

 }
 return dat;
}

//���д����
void ds1302_clear_WP()
{
  ds1302_reset();
  RST=1;
  ds1302_write_byte(0x8E);//д���ַ
  ds1302_write_byte(0x00);//д������
  //д���
  SDA=0;
  RST=0;
  
}

//����д����
void ds1302_set_WP()
{
  ds1302_reset();
  RST=1;
  ds1302_write_byte(0x8E);//д���ַ
  ds1302_write_byte(0x80);//д������
  //д���
  SDA=0;
  RST=0;
  
}



//д��ds1302
void ds1302_write(uint8 addr,uint8 dat)
{
  ds1302_reset();
  RST=1;
  ds1302_write_byte(addr);//д���ַ
  ds1302_write_byte(dat);//д������
  //д���
  RST=0;
  SDA=0;
}

//��ds1302������
uint8 ds1302_read(uint8 addr)
{
  uint8 temp=0;//�洢������������
  ds1302_reset();
  RST=1;
  ds1302_write_byte(addr);//д���ַ
  temp=ds1302_read_byte();//д������
  //д���
  RST=0;
  SDA=0;
  return temp;
}

void delay(uint16 n)
{ 
  while(n--);
}


/**
 * �趨ʱ������
 �����漰�����ƺ�BCDת��  ��Ϊds1302��BCD �����ǻ���ֻ��ʶ������
 ���磺����ds1302Ϊ50  0101 0000  ��ô����д�����ds1302_write��80��
*/
void ds1302_set_time(uint8 *timedata)
{
 	uint8 i;
	uint8 temp=0;
	for(i=0;i<7;i++)
	{
	  temp=timedata[i]/10;
	  timedata[i]=timedata[i]%10;
	  timedata[i]=timedata[i]+temp*16;
	}
	ds1302_clear_WP();//��ds1302����ǰ�Ƚ�wpλ����
	temp=DS1302_W_ADDR;//д����ʼλ��
	for(i=0;i<7;i++)
	{
	  ds1302_write(temp,timedata[i]);
	  temp+=2;
	}
	ds1302_set_WP();

}

/**
 * ��ʱ�����ݣ�BCD��ʽ��
*/
void ds1302_read_time(uint8 *timedata)
{
   uint8 i,temp;
   temp=DS1302_R_ADDR;//����ʼλ��
   for(i=0;i<7;i++)
   {
     timedata[i]=ds1302_read(temp);
	 temp+=2;
   }
}



///**
// * ������ת����ASC�벢ͨ��UART���ͳ�ȥ
//*/
//void UART_Send_Dat(uint8 dat)  //ע���ǹ���16ȡ��
//{
//   UART_Send_Byte(dat/16+'0');
//   UART_Send_Byte(dat%16+'0');	 
//}

