#include "SC50X0B.h"

extern void Delay_ms(unsigned int ms);
extern void Delay_us(unsigned int ms);

#define SC50x0BBUSY_BIT    (1<<2)
#define SC50x0BOUT_BIT     (1<<5)

#define SC50x0B_OUT        P4
#define SC50x0B_IN         P4

#define SC50x0B_OUT_H   SC50x0B_OUT |= SC50x0BOUT_BIT
#define SC50x0B_OUT_L   SC50x0B_OUT &= ~SC50x0BOUT_BIT

/*��ʾ������ʱ��ó�������Ĭ��ʱ��Ƶ�ʣ������ͬ����ʱ���������*/
/*
**Delay_ms(400);//600us
**Delay_ms(3);//3ms
**Delay_ms(150);//200us
*/
/*
void Delay_ms(unsigned char num)
{
  unsigned char i=0;
  int j=0;
  for(i=0;i<num;i++)
  {
    for(j=0;j<2100;j++)
	{
	   
	}
  }
}

void Delay_ms(int num)
{
  unsigned int i=0;
  int j=0;
  for(i=0;i<num;i++)
  {
    for(j=0;j<1;j++)
	{
	   
	}
  }
}
*/
//����IC���һ���ֽ�  ���Ƶ�����3��1
unsigned char SC50x0B_Out(unsigned char segdata)
{
    unsigned char i;
    
     SC50x0B_OUT_L;
     Delay_ms(3); 				 //3ms
     for(i=0;i < 8;i++)
     {
        SC50x0B_OUT_H; 
        if(segdata & 0x01)
        {
           Delay_us(600);  		 //600us
           SC50x0B_OUT_L;
           Delay_us(200);  		 //200us
        }
        else
        {
           Delay_us(200);  		 //200us
           SC50x0B_OUT_L;
           Delay_us(600);  		 //600us
        }
        segdata >>= 1;
     }
      SC50x0B_OUT_H;
      return 1;
}


//����оƬ��æ����鵱ǰ�Ƿ񲥷����,busy����1���򷵻�0
unsigned char SC50x0B_Busy(void)
{
  if(SC50x0B_IN & SC50x0BBUSY_BIT)
    return  0;
  else 
    return 1;
}