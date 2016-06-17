
#include "flashfunction.h"


 
 
void FLASH_ByteWrite (FLADDR addr, char byte)
{
    IAP_CONTR = ENABLE_ISP;         //�� IAP ����, ����Flash �����ȴ�ʱ��
    IAP_CMD = 0x02;                 //IAP/ISP/EEPROM �ֽڱ������
    IAP_ADDRH = (addr&0xff00)>>8;    //����Ŀ�굥Ԫ��ַ�ĸ�8 λ��ַ
    IAP_ADDRL = (addr&0xff);    //����Ŀ�굥Ԫ��ַ�ĵ�8 λ��ַ

    IAP_DATA = byte;                  //Ҫ��̵��������ͽ�IAP_DATA �Ĵ���
		EA=0;
    IAP_TRIG = 0x5A;   //���� 5Ah,����A5h ��ISP/IAP �����Ĵ���,ÿ�ζ������
    IAP_TRIG = 0xA5;   //����A5h ��ISP/IAP ����������������
		_nop_();
		EA=1;	
    IAP_Disable();  //�ر�IAP ����, ����ص����⹦�ܼĴ���,ʹCPU ���ڰ�ȫ״̬,
                    //һ��������IAP �������֮����ر�IAP ����,����Ҫÿ�ζ���
}
/****************************************************************************/
/* FLASH_ByteRead                                                           */ 
/*This routine reads a <byte> from the linear FLASH address <addr>.         */ 
/*                                                                          */ 
/****************************************************************************/
unsigned char FLASH_ByteRead (FLADDR addr)
{
    IAP_DATA = 0x00;
    IAP_CONTR = ENABLE_ISP;         //��IAP ����, ����Flash �����ȴ�ʱ��
    IAP_CMD = 0x01;                 //IAP/ISP/EEPROM �ֽڶ�����

    IAP_ADDRH = (addr&0xff00)>>8;    //����Ŀ�굥Ԫ��ַ�ĸ�8 λ��ַ
    IAP_ADDRL = (addr&0xff);    //����Ŀ�굥Ԫ��ַ�ĵ�8 λ��ַ

		EA=0;
    IAP_TRIG = 0x5A;   //���� 5Ah,����A5h ��ISP/IAP �����Ĵ���,ÿ�ζ������
    IAP_TRIG = 0xA5;   //����A5h ��ISP/IAP ����������������
		_nop_(); 
		EA=1;
    IAP_Disable();  //�ر�IAP ����, ����ص����⹦�ܼĴ���,ʹCPU ���ڰ�ȫ״̬,
                    //һ��������IAP �������֮����ر�IAP ����,����Ҫÿ�ζ���
    return (IAP_DATA);
}



/****************************************************************************/
/* FLASH_PageErase                                                          */ 
/*This routine erases the FLASH page containing the linear FLASH addres     */ 
/* <addr>.                                                                  */ 
/****************************************************************************/
void FLASH_PageErase (FLADDR addr)
{
	
    IAP_CONTR = ENABLE_ISP;         //��IAP ����, ����Flash �����ȴ�ʱ��
    IAP_CMD = 0x03;                 //IAP/ISP/EEPROM ������������

    IAP_ADDRH = (addr&0xff00)>>8;    //����Ŀ�굥Ԫ��ַ�ĸ�8 λ��ַ
    IAP_ADDRL = (addr&0xff);    //����Ŀ�굥Ԫ��ַ�ĵ�8 λ��ַ

		EA=0;
    IAP_TRIG = 0x5A;   //���� 5Ah,����A5h ��ISP/IAP �����Ĵ���,ÿ�ζ������
    IAP_TRIG = 0xA5;   //����A5h ��ISP/IAP ����������������
    _nop_();

		EA=1;
    IAP_Disable();  //�ر�IAP ����, ����ص����⹦�ܼĴ���,ʹCPU ���ڰ�ȫ״̬,
	//һ��������IAP �������֮����ر�IAP ����,����Ҫÿ�ζ���
}
void IAP_Disable()
{
    //�ر�IAP ����, ����ص����⹦�ܼĴ���,ʹCPU ���ڰ�ȫ״̬,
    //һ��������IAP �������֮����ر�IAP ����,����Ҫÿ�ζ���
    IAP_CONTR = 0;      //�ر�IAP ����
    IAP_CMD   = 0;      //������Ĵ���,ʹ����Ĵ���������,�˾�ɲ���
    IAP_TRIG  = 0;      //��������Ĵ���,ʹ������Ĵ����޴���,�˾�ɲ���
    IAP_ADDRH = 0x80;
    IAP_ADDRL = 0;
}



/* д���ݽ� ����Flash�洢��, ֻ��ͬһ��������д��������ԭ������ */
/* begin_addr,��д����Flash��ʼ��ַ��counter,����д���ٸ��ֽڣ� array[]��������Դ   */
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
        /* дһ���ֽ� */
        FLASH_ByteWrite(begin_addr, array[i]);
        /*  �Ƚ϶Դ� */
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
/*���²���ΪFLASH_W25Q64��д����*/
/****************************************************/
/***********************************************
�������ƣ�sFLASH_SendByte
��    �ܣ���W25Q64 FLASH����һ���ֽ����ݡ�
��ڲ�����dat:���͵�����
�� �� ֵ����	
��    ע����
************************************************/
void sFLASH_SendByte(unsigned char dat)
{
unsigned char i;	       //���������
//�ͳ�����
for(i=0;i<8;i++)		 	
{	W25Q64_WCK_Clr();
//�ȷ��͸��ֽ�
if( (dat&0x80)==0x80 )
{W25Q64_WDI_Set();}
else
{W25Q64_WDI_Clr();}
 Delay_1us();	
W25Q64_WCK_Set();
//��������
	dat<<=1;								
}  
}
/***********************************************
�������ƣ�sFLASH_ReadByte
��    �ܣ���W25Q64 FLASH��ȡһ���ֽ����ݡ�
��ڲ�������
�� �� ֵ��unsigned char�����������ݡ�	
��    ע����
************************************************/
unsigned char sFLASH_ReadByte(void)
{
unsigned char i;
unsigned char dat = 0;
W25Q64_WDO_Set();
W25Q64_WCK_Clr();
for (i=0; i<8; i++)             //8λ������
{W25Q64_WCK_Set();           //ʱ��������
Delay_1us();
dat <<= 1;	                //��������һλ
if (WDO) 
{dat |= 0x01;            //��ȡ����
}
W25Q64_WCK_Clr();           //ʱ��������
}
return dat;
}
/***********************************************
�������ƣ�sFLASH_WriteEnable
��    �ܣ�дʹ�ܡ�
��ڲ�������
�� �� ֵ����	
��    ע����
************************************************/
void sFLASH_WriteEnable(void)
{   //ʹ��оƬ 
W25Q64_WCS_Clr();
//����дʹ��ָ�� 
sFLASH_SendByte(sFLASH_CMD_WREN);
//��ֹоƬ 
W25Q64_WCS_Set();
}
/***********************************************
�������ƣ�sFLASH_WaitForWriteEnd
��    �ܣ��ȴ�д����
��ڲ�������
�� �� ֵ����	
��    ע����
************************************************/
void sFLASH_WaitForWriteEnd(void)
{unsigned char flashstatus = 0;
//оƬʹ�� 
W25Q64_WCS_Clr();
//���Ͷ�ȡ�Ĵ���ָ��
sFLASH_SendByte(sFLASH_CMD_RDSR);
//ѭ����ȡæ���ֱ��æ���� 
do
{	flashstatus = sFLASH_ReadByte();}
//�ж�æ���
while ((flashstatus & sFLASH_WIP_FLAG) == 0x01); 
//оƬ��ֹ 
W25Q64_WCS_Set();
}
/***********************************************
�������ƣ�sFLASH_ReadID
��    �ܣ���W25Q64 FLASH��ȡ����ID�š�
��ڲ�������
�� �� ֵ��unsigned int��������ID��	
��    ע����
************************************************/
unsigned int sFLASH_ReadID(void)
{
unsigned int Temp = 0; 
unsigned char Temp0 = 0, Temp1 = 0;
//ʹ��flash
W25Q64_WCS_Clr();
//���Ͷ�flash  id��ָ��
sFLASH_SendByte(sFLASH_CMD_RDID);
//����24λ��ַ ��ַΪ0
sFLASH_SendByte(0);
sFLASH_SendByte(0);
sFLASH_SendByte(0);
//��ȡ���ص�����
Temp0 = sFLASH_ReadByte();
Temp1 = sFLASH_ReadByte();
//��ֹflash
W25Q64_WCS_Set();
//�������
Temp = (Temp0 << 8) | Temp1;
return Temp;
}
/***********************************************
�������ƣ�sFLASH_WritePage
��    �ܣ���W25Q64 FLASHд��һҳ���ݡ�
��ڲ�����pBuffer:���д������ݵĻ�������
			WriteAddr����ʼд��ĵ�ַ��
	NumByteToWrite��д����ֽ���
�� �� ֵ���ޣ����������ݡ�	
��    ע����
************************************************/
void sFLASH_WritePage(unsigned char* pBuffer, FLADDR WriteAddr, unsigned int NumByteToWrite)
{//оƬдʹ�ܣ�д֮ǰ��дʹ�ܣ������޷�д�� 
sFLASH_WriteEnable();
//оƬʹ�� 
W25Q64_WCS_Clr();
//����дָ�� 
sFLASH_SendByte(sFLASH_CMD_PAGEPRO);
//����24λ��ַ 
sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
sFLASH_SendByte(WriteAddr & 0xFF);
//ѭ�����ֽ�д�� 
while (NumByteToWrite--)
{sFLASH_SendByte(*pBuffer);
 pBuffer++;
}
//оƬ��ֹ 
W25Q64_WCS_Set();
//�ȴ�д���
sFLASH_WaitForWriteEnd();
}

/***********************************************
�������ƣ�sFLASH_ReadBuffer
��    �ܣ���W25Q64 FLASH��ȡһ�����ݡ�
��ڲ�����pBuffer:���ո��ַ����Ļ�����
			ReadAddr����ʼ���ĵ�ַ
	NumByteToRead�������ֽ���
�� �� ֵ����	
��    ע����
************************************************/
void sFLASH_ReadBuffer(unsigned char * pBuffer, FLADDR ReadAddr, unsigned int NumByteToRead)
{//оƬʹ�� 
W25Q64_WCS_Clr();

//���Ͷ�����
sFLASH_SendByte(sFLASH_CMD_READ);

//����24λ��ַ 
sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
sFLASH_SendByte(ReadAddr & 0xFF);

//ѭ�����ֽڶ�ȡ
while (NumByteToRead--) 
{
	*pBuffer = sFLASH_ReadByte();
	pBuffer++;
}

//оƬ��ֹ 
W25Q64_WCS_Set();
}
/***********************************************
�������ƣ�sFLASH_EraseSector
��    �ܣ�����W25Q64 FLASH��һ��������
��ڲ�������
�� �� ֵ��unsigned char�����������ݡ�	
��    ע����
************************************************/
void sFLASH_EraseSector(FLADDR SectorAddr)
{   //оƬдʹ��
sFLASH_WriteEnable();
//оƬʹ�� 
W25Q64_WCS_Clr();
//������������ָ��
sFLASH_SendByte(sFLASH_CMD_SE);
//����24λ��ַ
sFLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
sFLASH_SendByte((SectorAddr & 0xFF00) >> 8);
sFLASH_SendByte(SectorAddr & 0xFF);
//оƬ��ֹ 
W25Q64_WCS_Set();
//�ȴ�д���
sFLASH_WaitForWriteEnd();
}

/***********************************************
�������ƣ�sFLASH_64KBEraseSector
��    �ܣ�����W25Q64 FLASH��һ���顣
��ڲ�������
�� �� ֵ����
��    ע����
************************************************/
void sFLASH_64KBEraseSector(FLADDR SectorAddr)
{   //оƬдʹ��
sFLASH_WriteEnable();
//оƬʹ�� 
W25Q64_WCS_Clr();
//������������ָ��
sFLASH_SendByte(sFLASH_CMD_BE);
//����24λ��ַ
sFLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
sFLASH_SendByte((SectorAddr & 0xFF00) >> 8);
sFLASH_SendByte(SectorAddr & 0xFF);
//оƬ��ֹ 
W25Q64_WCS_Set();
//�ȴ�д���
sFLASH_WaitForWriteEnd();
}

/***********************************************
�������ƣ�sFLASH_EraseSector
��    �ܣ�����W25Q64 FLASH����оƬ,
��Ҫ���20s���ҵ�ʱ�䡣
��ڲ�������
�� �� ֵ��unsigned char�����������ݡ�	
��    ע����
************************************************/
void sFLASH_EraseFlash(void)
{   //оƬдʹ��
  	sFLASH_WriteEnable();
  	//оƬʹ�� 
  	W25Q64_WCS_Clr();
  	//������������ָ��
  	sFLASH_SendByte(sFLASH_CMD_CE);

  	//оƬ��ֹ 
  	W25Q64_WCS_Set();
  	//�ȴ�д���
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
      // 1. Erase Scratch page		�����ݸ���ʱҳ
      FLASH_W25Q64_PageErase (FLASH_W25Q64_TEMP);
      // 2. Copy bytes from first byte of dest page to dest-1 to Scratch page
		 //��dest 0 ��dest-1 ���ظ��Ƶ��ݸ���ʱҳ
      wptr = FLASH_W25Q64_TEMP;
      rptr = dest_1_page_start;
      length = dest - dest_1_page_start;
			Full_BlockNum = (length&0x0ff00)>>8;
			NoFull_BlockNum = length&0x00ff;

		  if(Full_BlockNum>0)/*����һҳ������ҳ����*/
	   { 
			 FLASH_W25Q64_Copy (wptr, rptr, Full_BlockNum,1);
			 //ʣ�µĲ�������ҳ�����ⵥ����ʽд��
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
 		  if(Full_BlockNum>0)/*����һҳ������ҳ����*/
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
      		  if(Full_BlockNum>0)/*����һҳ������ҳ����*/
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
      		  if(Full_BlockNum>0)/*����һҳ������ҳ����*/
	   { 
			 FLASH_W25Q64_Copy (wptr, rptr, Full_BlockNum,1);
			 //ʣ�µĲ�������ҳ�����ⵥ����ʽд��
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
      		  if(Full_BlockNum>0)/*����һҳ������ҳ����*/
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
      		  if(Full_BlockNum>0)/*����һҳ������ҳ����*/
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
      if(Full_BlockNum>0)/*����һҳ������ҳ����*/
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


void Delay_1us(void)		//��ʱ1us
{
unsigned char i=1;
while(i--);	 //WDT_CONTR=0x34;
}
