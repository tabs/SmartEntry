#ifndef SC50x0B_h
#define SC50x0B_h

#include "IAP15W4K61S4.h"

#define INPUTVALIDCARD         0x02	  //�������Ч�����֤
#define READCARDWRONG          0x03		//��������
#define CARDISABLE             0x04		//�˿�����
#define PASTDUE                0x05		//�˿�����
#define INVALIDTWODIMENSION    0x06	  //��Ч��ά��
#define TWODIMENSIONPASTDUE    0x07		//��ά����ڣ�����������
#define INVALIDFINGER          0x08		//��Чָ��
#define INPUTFINGER            0x09	  //�����·�����Чָ��
#define FINGERWRONG            0x0A		//ָ��ʶ�����
#define PUTFINGER              0x0B		//��¼��ָ��
#define PUTFINGERAGAIN         0x0C	  //���ٴ�¼��ָ��
#define FINGERPRINTSUCCESS     0x0D		//ָ��¼��ɹ�
#define FINGERPRINTFAIL        0x0E		//ָ��¼��ʧ��
#define DOOROPEN               0x0F		//�ſ���
#define WELCOME                0x10		//��ӭ����
#define GOODTRIP               0x11		//һ·˳��
#define SYSTEMUPGRADE          0x12		//ϵͳ����ά���У������
#define TWODIMENSIONWRONG      0x13		//��ά��ʶ�����
#define PUTTWODIMENSIONAGAIN   0x14		//���ٴ�ɨ���ά��

void SC50x0B_Init(void);
unsigned char SC50x0B_Out(unsigned char segdata);
unsigned char SC50x0B_Busy(void);
#endif	 