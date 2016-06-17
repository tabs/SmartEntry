#ifndef SC50x0B_h
#define SC50x0B_h

#include "IAP15W4K61S4.h"

#define INPUTVALIDCARD         0x02	  //请放入有效的身份证
#define READCARDWRONG          0x03		//读卡错误
#define CARDISABLE             0x04		//此卡禁用
#define PASTDUE                0x05		//此卡过期
#define INVALIDTWODIMENSION    0x06	  //无效二维码
#define TWODIMENSIONPASTDUE    0x07		//二维码过期，请重新生成
#define INVALIDFINGER          0x08		//无效指纹
#define INPUTFINGER            0x09	  //请重新放入有效指纹
#define FINGERWRONG            0x0A		//指纹识别错误
#define PUTFINGER              0x0B		//请录入指纹
#define PUTFINGERAGAIN         0x0C	  //请再次录入指纹
#define FINGERPRINTSUCCESS     0x0D		//指纹录入成功
#define FINGERPRINTFAIL        0x0E		//指纹录入失败
#define DOOROPEN               0x0F		//门开了
#define WELCOME                0x10		//欢迎光临
#define GOODTRIP               0x11		//一路顺风
#define SYSTEMUPGRADE          0x12		//系统升级维护中，请见谅
#define TWODIMENSIONWRONG      0x13		//二维码识别错误
#define PUTTWODIMENSIONAGAIN   0x14		//请再次扫描二维码

void SC50x0B_Init(void);
unsigned char SC50x0B_Out(unsigned char segdata);
unsigned char SC50x0B_Busy(void);
#endif	 