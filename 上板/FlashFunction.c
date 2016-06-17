
#include "flashfunction.h"


 
 
void FLASH_ByteWrite (FLADDR addr, char byte)
{
    IAP_CONTR = ENABLE_ISP;         //打开 IAP 功能, 设置Flash 操作等待时间
    IAP_CMD = 0x02;                 //IAP/ISP/EEPROM 字节编程命令
    IAP_ADDRH = (addr&0xff00)>>8;    //设置目标单元地址的高8 位地址
    IAP_ADDRL = (addr&0xff);    //设置目标单元地址的低8 位地址

    IAP_DATA = byte;                  //要编程的数据先送进IAP_DATA 寄存器
		EA=0;
    IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
    IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动
		_nop_();
		EA=1;	
    IAP_Disable();  //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
                    //一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
}
/****************************************************************************/
/* FLASH_ByteRead                                                           */ 
/*This routine reads a <byte> from the linear FLASH address <addr>.         */ 
/*                                                                          */ 
/****************************************************************************/
unsigned char FLASH_ByteRead (FLADDR addr)
{
    IAP_DATA = 0x00;
    IAP_CONTR = ENABLE_ISP;         //打开IAP 功能, 设置Flash 操作等待时间
    IAP_CMD = 0x01;                 //IAP/ISP/EEPROM 字节读命令

    IAP_ADDRH = (addr&0xff00)>>8;    //设置目标单元地址的高8 位地址
    IAP_ADDRL = (addr&0xff);    //设置目标单元地址的低8 位地址

		EA=0;
    IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
    IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动
		_nop_(); 
		EA=1;
    IAP_Disable();  //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
                    //一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
    return (IAP_DATA);
}



/****************************************************************************/
/* FLASH_PageErase                                                          */ 
/*This routine erases the FLASH page containing the linear FLASH addres     */ 
/* <addr>.                                                                  */ 
/****************************************************************************/
void FLASH_PageErase (FLADDR addr)
{
	
    IAP_CONTR = ENABLE_ISP;         //打开IAP 功能, 设置Flash 操作等待时间
    IAP_CMD = 0x03;                 //IAP/ISP/EEPROM 扇区擦除命令

    IAP_ADDRH = (addr&0xff00)>>8;    //设置目标单元地址的高8 位地址
    IAP_ADDRL = (addr&0xff);    //设置目标单元地址的低8 位地址

		EA=0;
    IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
    IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动
    _nop_();

		EA=1;
    IAP_Disable();  //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
	//一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
}
void IAP_Disable()
{
    //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
    //一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
    IAP_CONTR = 0;      //关闭IAP 功能
    IAP_CMD   = 0;      //清命令寄存器,使命令寄存器无命令,此句可不用
    IAP_TRIG  = 0;      //清命令触发寄存器,使命令触发寄存器无触发,此句可不用
    IAP_ADDRH = 0x80;
    IAP_ADDRL = 0;
}



/* 写数据进 数据Flash存储器, 只在同一个扇区内写，不保留原有数据 */
/* begin_addr,被写数据Flash开始地址；counter,连续写多少个字节； array[]，数据来源   */
UCHAR sequential_write_flash_in_one_sector(UINT begin_addr, UINT counter, UCHAR array[])
{
    UINT i = 0;
    UINT in_sector_begin_addr = 0;
    UINT sector_addr = 0;

    FLASH_PageErase(begin_addr);
//			SBUF = begin_addr>>24;while(!(SCON&0x02));SCON &= ~0x02;
//			SBUF = begin_addr>>16;while(!(SCON&0x02));SCON &= ~0x02;
//			SBUF = begin_addr>>8;while(!(SCON&0x02));SCON &= ~0x02;
//			SBUF = begin_addr;while(!(SCON&0x02));SCON &= ~0x02;
    for(i=0; i<counter; i++)
    {
        /* 写一个字节 */
        FLASH_ByteWrite(begin_addr, array[i]);
        /*  比较对错 */
        if (FLASH_ByteRead(begin_addr) != array[i])
        {
            IAP_Disable();
            return 0;
        }
//			 SBUF = array[i];while(!(SCON&0x02));SCON &= ~0x02;

        begin_addr++;
    }
    IAP_Disable();
    return  1;
}


/****************************************************/
/*以下部分为FLASH_W25Q64读写程序*/
/****************************************************/
/***********************************************
函数名称：sFLASH_SendByte
功    能：向W25Q64 FLASH发送一个字节数据。
入口参数：dat:发送的数据
返 回 值：无	
备    注：无
************************************************/
void sFLASH_SendByte(unsigned char dat)
{
unsigned char i;	       //定义计数器
//送出数据
for(i=0;i<8;i++)		 	
{	W25Q64_WCK_Clr();
//先发送高字节
if( (dat&0x80)==0x80 )
{W25Q64_WDI_Set();}
else
{W25Q64_WDI_Clr();}
 Delay_1us();	
W25Q64_WCK_Set();
//数据左移
	dat<<=1;								
}  
}
/***********************************************
函数名称：sFLASH_ReadByte
功    能：从W25Q64 FLASH读取一个字节数据。
入口参数：无
返 回 值：unsigned char：读出的数据。	
备    注：无
************************************************/
unsigned char sFLASH_ReadByte(void)
{
unsigned char i;
unsigned char dat = 0;
W25Q64_WDO_Set();
W25Q64_WCK_Clr();
for (i=0; i<8; i++)             //8位计数器
{W25Q64_WCK_Set();           //时钟线拉高
Delay_1us();
dat <<= 1;	                //数据右移一位
if (WDO) 
{dat |= 0x01;            //读取数据
}
W25Q64_WCK_Clr();           //时钟线拉低
}
return dat;
}
/***********************************************
函数名称：sFLASH_WriteEnable
功    能：写使能。
入口参数：无
返 回 值：无	
备    注：无
************************************************/
void sFLASH_WriteEnable(void)
{   //使能芯片 
W25Q64_WCS_Clr();
//发送写使能指令 
sFLASH_SendByte(sFLASH_CMD_WREN);
//禁止芯片 
W25Q64_WCS_Set();
}
/***********************************************
函数名称：sFLASH_WaitForWriteEnd
功    能：等待写结束
入口参数：无
返 回 值：无	
备    注：无
************************************************/
void sFLASH_WaitForWriteEnd(void)
{unsigned char flashstatus = 0;
//芯片使能 
W25Q64_WCS_Clr();
//发送读取寄存器指令
sFLASH_SendByte(sFLASH_CMD_RDSR);
//循环读取忙标记直到忙结束 
do
{	flashstatus = sFLASH_ReadByte();}
//判断忙标记
while ((flashstatus & sFLASH_WIP_FLAG) == 0x01); 
//芯片禁止 
W25Q64_WCS_Set();
}
/***********************************************
函数名称：sFLASH_ReadID
功    能：从W25Q64 FLASH读取器件ID号。
入口参数：无
返 回 值：unsigned int：读出的ID。	
备    注：无
************************************************/
unsigned int sFLASH_ReadID(void)
{
unsigned int Temp = 0; 
unsigned char Temp0 = 0, Temp1 = 0;
//使能flash
W25Q64_WCS_Clr();
//发送读flash  id号指令
sFLASH_SendByte(sFLASH_CMD_RDID);
//发送24位地址 地址为0
sFLASH_SendByte(0);
sFLASH_SendByte(0);
sFLASH_SendByte(0);
//读取返回的数据
Temp0 = sFLASH_ReadByte();
Temp1 = sFLASH_ReadByte();
//禁止flash
W25Q64_WCS_Set();
//组合数据
Temp = (Temp0 << 8) | Temp1;
return Temp;
}
/***********************************************
函数名称：sFLASH_WritePage
功    能：向W25Q64 FLASH写入一页数据。
入口参数：pBuffer:存放写入的数据的缓冲区，
			WriteAddr：开始写入的地址，
	NumByteToWrite：写入的字节数
返 回 值：无：读出的数据。	
备    注：无
************************************************/
void sFLASH_WritePage(unsigned char* pBuffer, FLADDR WriteAddr, unsigned int NumByteToWrite)
{//芯片写使能，写之前需写使能，否则无法写入 
sFLASH_WriteEnable();
//芯片使能 
W25Q64_WCS_Clr();
//发送写指令 
sFLASH_SendByte(sFLASH_CMD_PAGEPRO);
//发送24位地址 
sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
sFLASH_SendByte(WriteAddr & 0xFF);
//循环按字节写入 
while (NumByteToWrite--)
{sFLASH_SendByte(*pBuffer);
 pBuffer++;
}
//芯片禁止 
W25Q64_WCS_Set();
//等待写完成
sFLASH_WaitForWriteEnd();
}

/***********************************************
函数名称：sFLASH_ReadBuffer
功    能：从W25Q64 FLASH读取一串数据。
入口参数：pBuffer:接收该字符串的缓冲区
			ReadAddr：开始读的地址
	NumByteToRead：读的字节数
返 回 值：无	
备    注：无
************************************************/
void sFLASH_ReadBuffer(unsigned char * pBuffer, FLADDR ReadAddr, unsigned int NumByteToRead)
{//芯片使能 
W25Q64_WCS_Clr();

//发送读命令
sFLASH_SendByte(sFLASH_CMD_READ);

//发送24位地址 
sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
sFLASH_SendByte(ReadAddr & 0xFF);

//循环按字节读取
while (NumByteToRead--) 
{
	*pBuffer = sFLASH_ReadByte();
	pBuffer++;
}

//芯片禁止 
W25Q64_WCS_Set();
}
/***********************************************
函数名称：sFLASH_EraseSector
功    能：擦除W25Q64 FLASH的一个扇区。
入口参数：无
返 回 值：unsigned char：读出的数据。	
备    注：无
************************************************/
void sFLASH_EraseSector(FLADDR SectorAddr)
{   //芯片写使能
sFLASH_WriteEnable();
//芯片使能 
W25Q64_WCS_Clr();
//发送扇区擦除指令
sFLASH_SendByte(sFLASH_CMD_SE);
//发送24位地址
sFLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
sFLASH_SendByte((SectorAddr & 0xFF00) >> 8);
sFLASH_SendByte(SectorAddr & 0xFF);
//芯片禁止 
W25Q64_WCS_Set();
//等待写完成
sFLASH_WaitForWriteEnd();
}

/***********************************************
函数名称：sFLASH_64KBEraseSector
功    能：擦除W25Q64 FLASH的一个块。
入口参数：无
返 回 值：无
备    注：无
************************************************/
void sFLASH_64KBEraseSector(FLADDR SectorAddr)
{   //芯片写使能
sFLASH_WriteEnable();
//芯片使能 
W25Q64_WCS_Clr();
//发送扇区擦除指令
sFLASH_SendByte(sFLASH_CMD_BE);
//发送24位地址
sFLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
sFLASH_SendByte((SectorAddr & 0xFF00) >> 8);
sFLASH_SendByte(SectorAddr & 0xFF);
//芯片禁止 
W25Q64_WCS_Set();
//等待写完成
sFLASH_WaitForWriteEnd();
}

/***********************************************
函数名称：sFLASH_EraseSector
功    能：擦除W25Q64 FLASH整个芯片,
需要大概20s左右的时间。
入口参数：无
返 回 值：unsigned char：读出的数据。	
备    注：无
************************************************/
void sFLASH_EraseFlash(void)
{   //芯片写使能
  	sFLASH_WriteEnable();
  	//芯片使能 
  	W25Q64_WCS_Clr();
  	//发送扇区擦除指令
  	sFLASH_SendByte(sFLASH_CMD_CE);

  	//芯片禁止 
  	W25Q64_WCS_Set();
  	//等待写完成
  	sFLASH_WaitForWriteEnd();
}






void FLASH_W25Q64_Clear (FLADDR dest, unsigned numbytes)
{
  
 
   FLADDR dest_1_page_start;           // first address in 1st page
                                       // containing <dest>
   FLADDR dest_1_page_end;             // last address in 1st page
                                       // containing <dest>
   FLADDR dest_2_page_start;           // first address in 2nd page
                                       // containing <dest>
   FLADDR dest_2_page_end;             // last address in 2nd page
                                       // containing <dest>
   unsigned  numbytes_remainder;        // when crossing page boundary,
                                       // number of <src> bytes on 2nd page
   FLADDR FLASH_pagesize;            // size of FLASH page to update
 
   FLADDR  wptr;                       // write address
   FLADDR  rptr;                       // read address
 
   unsigned length;
	 unsigned Full_BlockNum = 0,NoFull_BlockNum = 0;
   FLASH_pagesize = FLASH_W25Q64_PAGESIZE;
 
   dest_1_page_start = dest & ~(FLASH_pagesize - 1);

   dest_1_page_end = dest_1_page_start + FLASH_pagesize - 1;

   dest_2_page_start = (dest + numbytes)  & ~(FLASH_pagesize - 1);

   dest_2_page_end = dest_2_page_start + FLASH_pagesize - 1;
	
   if (dest_1_page_end == dest_2_page_end) {
      // 1. Erase Scratch page		擦除草稿临时页
      FLASH_W25Q64_PageErase (FLASH_W25Q64_TEMP);
      // 2. Copy bytes from first byte of dest page to dest-1 to Scratch page
		 //从dest 0 到dest-1 比特复制到草稿临时页
      wptr = FLASH_W25Q64_TEMP;
      rptr = dest_1_page_start;
      length = dest - dest_1_page_start;
			Full_BlockNum = (length&0x0ff00)>>8;
			NoFull_BlockNum = length&0x00ff;

		  if(Full_BlockNum>0)/*超过一页后，整数页处理*/
	   { 
			 FLASH_W25Q64_Copy (wptr, rptr, Full_BlockNum,1);
			 //剩下的不是整数页按另外单独方式写入
			 FLASH_W25Q64_Copy (wptr + Full_BlockNum*256, rptr+ Full_BlockNum*256, NoFull_BlockNum,0);
		  }else{
				FLASH_W25Q64_Copy (wptr, rptr, NoFull_BlockNum,0);
	    }
      // 3. Copy from (dest+numbytes) to dest_page_end to Scratch page
 
      wptr = FLASH_W25Q64_TEMP + dest - dest_1_page_start + numbytes;
      rptr = dest + numbytes;
      length = dest_1_page_end - dest - numbytes + 1;
			Full_BlockNum = (length&0x0ff00)>>8;
			NoFull_BlockNum = length&0x00ff;
 		  if(Full_BlockNum>0)/*超过一页后，整数页处理*/
	   { 
			 FLASH_W25Q64_Copy (wptr, rptr, Full_BlockNum,1);
			 FLASH_W25Q64_Copy (wptr + Full_BlockNum*256, rptr+ Full_BlockNum*256, NoFull_BlockNum,0);
		  }else{
				FLASH_W25Q64_Copy (wptr, rptr, NoFull_BlockNum,0);
	    }
 
      // 4. Erase destination page
      FLASH_W25Q64_PageErase (dest_1_page_start);
 
      // 5. Copy Scratch page to destination page
      wptr = dest_1_page_start;
      rptr = FLASH_W25Q64_TEMP;
      length = FLASH_pagesize;
			Full_BlockNum = (length&0x0ff00)>>8;
			NoFull_BlockNum = length&0x00ff;
      		  if(Full_BlockNum>0)/*超过一页后，整数页处理*/
	   { 
			 FLASH_W25Q64_Copy (wptr, rptr, Full_BlockNum,1);
			 FLASH_W25Q64_Copy (wptr + Full_BlockNum*256, rptr+ Full_BlockNum*256, NoFull_BlockNum,0);
		  }else{
				FLASH_W25Q64_Copy (wptr, rptr, NoFull_BlockNum,0);
	    }
 
    
} else {
                            // value crosses page boundary
      // 1. Erase Scratch page
      FLASH_W25Q64_PageErase (FLASH_W25Q64_TEMP);
 
      // 2. Copy bytes from first byte of dest page to dest-1 to Scratch page
 
      wptr = FLASH_W25Q64_TEMP;
      rptr = dest_1_page_start;
      length = dest - dest_1_page_start;
			Full_BlockNum = (length&0x0ff00)>>8;
			NoFull_BlockNum = length&0x00ff;
      		  if(Full_BlockNum>0)/*超过一页后，整数页处理*/
	   { 
			 FLASH_W25Q64_Copy (wptr, rptr, Full_BlockNum,1);
			 //剩下的不是整数页按另外单独方式写入
			 FLASH_W25Q64_Copy (wptr + Full_BlockNum*256, rptr+ Full_BlockNum*256, NoFull_BlockNum,0);
		  }else{
				FLASH_W25Q64_Copy (wptr, rptr, NoFull_BlockNum,0);
	    }
 
      // 3. Erase destination page 1
      FLASH_W25Q64_PageErase (dest_1_page_start);
 
      // 4. Copy Scratch page to destination page 1
      wptr = dest_1_page_start;
      rptr = FLASH_W25Q64_TEMP;
      length = FLASH_pagesize;
			Full_BlockNum = (length&0x0ff00)>>8;
			NoFull_BlockNum = length&0x00ff;
      		  if(Full_BlockNum>0)/*超过一页后，整数页处理*/
	   { 
			 FLASH_W25Q64_Copy (wptr, rptr, Full_BlockNum,1);
			 FLASH_W25Q64_Copy (wptr + Full_BlockNum*256, rptr+ Full_BlockNum*256, NoFull_BlockNum,0);
		  }else{
				FLASH_W25Q64_Copy (wptr, rptr, NoFull_BlockNum,0);
	    }
 
      // now handle 2nd page
 
      // 5. Erase Scratch page
      FLASH_W25Q64_PageErase (FLASH_W25Q64_TEMP);
 
      // 6. Copy bytes from numbytes remaining to dest-2_page_end to Scratch page
 
      numbytes_remainder = numbytes - (dest_1_page_end - dest + 1);
      wptr = FLASH_W25Q64_TEMP + numbytes_remainder;
      rptr = dest_2_page_start + numbytes_remainder;
      length = FLASH_pagesize - numbytes_remainder;
			Full_BlockNum = (length&0x0ff00)>>8;
			NoFull_BlockNum = length&0x00ff;
      		  if(Full_BlockNum>0)/*超过一页后，整数页处理*/
	   { 
			 FLASH_W25Q64_Copy (wptr, rptr, Full_BlockNum,1);
			 FLASH_W25Q64_Copy (wptr + Full_BlockNum*256, rptr+ Full_BlockNum*256, NoFull_BlockNum,0);
		  }else{
				FLASH_W25Q64_Copy (wptr, rptr, NoFull_BlockNum,0);
	    }
 
      // 7. Erase destination page 2
      FLASH_W25Q64_PageErase (dest_2_page_start);
 
      // 8. Copy Scratch page to destination page 2
      wptr = dest_2_page_start;
      rptr = FLASH_W25Q64_TEMP;
      length = FLASH_pagesize;
			Full_BlockNum = (length&0x0ff00)>>8;
			NoFull_BlockNum = length&0x00ff;
      if(Full_BlockNum>0)/*超过一页后，整数页处理*/
	   { 
			 FLASH_W25Q64_Copy (wptr, rptr, Full_BlockNum,1);
			 FLASH_W25Q64_Copy (wptr + Full_BlockNum*256, rptr+ Full_BlockNum*256, NoFull_BlockNum,0);
		  }else{
				FLASH_W25Q64_Copy (wptr, rptr, NoFull_BlockNum,0);
	    }
    
}
 
}



//-----------------------------------------------------------------------------
// FLASH_W25Q64_Write
//-----------------------------------------------------------------------------
//
// This routine copies <numbytes> from <src> to the linear FLASH_W25Q64 address
// <dest>.
//
void FLASH_W25Q64_Write (FLADDR dest, char *src, FLADDR numbytes)
{
   FLADDR i;
   for (i = dest; i < dest+numbytes; i++) {
      FLASH_W25Q64_ByteWrite (i, *src++);
   }
}



void FLASH_W25Q64_Update (FLADDR dest, char *src, unsigned numbytes)
{
   // 1. Erase <numbytes> starting from <dest>
   FLASH_W25Q64_Clear (dest, numbytes);
   // 2. Write <numbytes> from <src> to <dest>
   FLASH_W25Q64_Write (dest, src, numbytes);
}
//-----------------------------------------------------------------------------
// FLASH_W25Q64_Read
//-----------------------------------------------------------------------------
//
// This routine copies <numbytes> from the linear FLASH_W25Q64 address <src> to
// <dest>.
//
char * FLASH_W25Q64_Read (char *dest, FLADDR src, unsigned numbytes)
{
   FLADDR i;
   for (i = 0; i < numbytes; i++) {
      *dest++ = FLASH_W25Q64_ByteRead (src+i);
   }
   return dest;
}
//-----------------------------------------------------------------------------
// FLASH_W25Q64_Copy
//-----------------------------------------------------------------------------
//
// This routine copies <numbytes> from <src> to the linear FLASH_W25Q64 address
// <dest>.
//
void FLASH_W25Q64_Copy (FLADDR dest, FLADDR src,unsigned numbytes,bit IsFullBlockCopy)
{
   FLADDR i;
	unsigned char DataBuffer[256];
	if(IsFullBlockCopy){
	for(i=0;i<numbytes;i++)
	     { 
			 sFLASH_ReadBuffer(DataBuffer, (src + i*256), 256);
		   sFLASH_WritePage(DataBuffer, (dest+i*256), 256);
 }
	}else{
		sFLASH_ReadBuffer(DataBuffer,src , numbytes);
		sFLASH_WritePage(DataBuffer, dest, numbytes);
	 }
 }

//-----------------------------------------------------------------------------
// FLASH_W25Q64_Fill
//-----------------------------------------------------------------------------
//
// This routine fills the FLASH_W25Q64 beginning at <addr> with <lenght> bytes.
//
void FLASH_W25Q64_Fill (FLADDR addr, ULONG length, unsigned char fill)
{
   FLADDR i;
   for (i = 0; i < length; i++) {
      FLASH_W25Q64_ByteWrite (addr+i, fill);
   }
}





void FLASH_W25Q64_PageErase (FLADDR addr){

sFLASH_EraseSector(addr);
}

void FLASH_W25Q64_ByteWrite (FLADDR addr, char byte)
{

  sFLASH_WritePage(&byte,addr,1);
}

unsigned char FLASH_W25Q64_ByteRead (FLADDR addr)
{
	unsigned char dat;

  sFLASH_ReadBuffer(&dat,addr,1);
	return dat;
}


void Delay_1us(void)		//延时1us
{
unsigned char i=1;
while(i--);	 //WDT_CONTR=0x34;
}
