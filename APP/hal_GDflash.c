#include "hal_gdflash.h"
#include "stm32f10x.h"
#include "FRAM.h"

#if 0
/*****************************************************************************
 Prototype    : hal_Init_RF_pins
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void hal_Init_gdflash_pins(void)
{
  
}



void hal_gdflash_SPI_Config(void)
{

}



#endif

/*****************************************************************************
 Prototype    : hal_sRF_SPI_Config
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void GDflash_init(void)
{
    spiFlash_FRAMFlash_init();
}
/*****************************************************************************
 Prototype    : spiReadWriteByte
 Description  : spi basic function
 Input        : u8 data  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
u8 GDflash_ReadWriteByte(u8 data)
{
  return  FRAMReadWriteByte(data);
    
}

void GDflash_write_enable(bool enable)
{
  DGFLASH_CS_LOW();
  
  if (enable)
  {
    GDflash_ReadWriteByte(GDFLASH_WRITE_ENABLE);
  }
  else
  {
    GDflash_ReadWriteByte(GDFLASH_WRITE_DISABLE);
  }
  
  DGFLASH_CS_HIGH();
}


u8 GDflash_read_status_reg(void)
{   
   u8 status_value;
  
   DGFLASH_CS_LOW(); 
   GDflash_ReadWriteByte(GDFLASH_READ_STATUS_REG);
   status_value = GDflash_ReadWriteByte(DGFLASH_DUMMY_BYTE);
   DGFLASH_CS_HIGH(); 
   
   return status_value;
}

void GDflash_write_status_reg(u8 value)
{
  DGFLASH_CS_LOW(); 
  GDflash_ReadWriteByte(GDFLASH_WRITE_STATUS_REG);
  GDflash_ReadWriteByte(value);
  DGFLASH_CS_HIGH(); 
}

void GDflash_read_datas(u32 startAddr, u8 *buf, u32 length, bool fastRead)
{
  while ((GDflash_read_status_reg() & 0x01) == 1 );
  
  DGFLASH_CS_LOW(); 
  
  if (fastRead)
  {
    GDflash_ReadWriteByte(GDFLASH_FAST_READ);
  }
  else
  {
    GDflash_ReadWriteByte(GDFLASH_READ_DATA);
  }
  
  GDflash_ReadWriteByte( (u8)((startAddr>> 16) &0xFF));
  GDflash_ReadWriteByte( (u8)((startAddr>> 8) &0xFF));
  GDflash_ReadWriteByte( (u8)(startAddr &0xFF));
  
  if (fastRead)
  {
    GDflash_ReadWriteByte(DGFLASH_DUMMY_BYTE);
  }

  for (u32 i = 0; i < length; i++)
  {
    buf[i] =  GDflash_ReadWriteByte(DGFLASH_DUMMY_BYTE);
  }
  
  DGFLASH_CS_HIGH();   
}



/*****************************************************************************
 Prototype    : spiReadWriteByte
 Description  : spi basic function
 Input        : u8 data  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
��д���ַ����ÿҳ�Ŀ�ͷʱ������ҳ��ʣ��ռ�С��Ҫд������ݴ�С����ô���ݽ����ص�ǰ
��ַ��ʼ������ҳ�󣬴Ӵ�ҳ�Ŀ�ʼ�����ټ�����䣬ֱ�����
д�����ݲ��ܳ���ÿҳ�Ĵ�С256�ֽڣ������Ĳ������ܱ�֤���256�ֽڵ����ݱ�д��flash
*****************************************************************************/
void GDflash_page_program(u32 startAddr, u8 *buf, u16 length, bool fastWrite)
{
  while ((GDflash_read_status_reg() & 0x01) == 1 );
  
  GDflash_write_enable(TRUE);
  
  
  DGFLASH_CS_LOW(); 
  
  if (fastWrite)
  {
    GDflash_ReadWriteByte(GDFLASH_PAGE_PROGRAM_F2);
  }
  else
  {
    GDflash_ReadWriteByte(GDFLASH_PAGE_PROGRAM_02);
  }

  GDflash_ReadWriteByte( (u8)((startAddr>> 16) &0xFF));
  GDflash_ReadWriteByte( (u8)((startAddr>> 8) &0xFF));
  GDflash_ReadWriteByte( (u8)(startAddr &0xFF));
  
  for (u16 i = 0; i < length; i++)
  {
    GDflash_ReadWriteByte( buf[i]);
  }
  
  DGFLASH_CS_HIGH();
}


/*
 Any address inside the sector is a valid 
address for the Sector Erase (SE) command
*/
void GDflash_erase(u32 startAddr, GDFLASH_ERASE_TYPE erase_type)
{
  while ((GDflash_read_status_reg() & 0x01) == 1 );
  
  GDflash_write_enable(TRUE);
  
  DGFLASH_CS_LOW(); 
  
  switch (erase_type)
  {
    case SECTOR_ERASE:
     GDflash_ReadWriteByte(GDFLASH_SECTOR_ERASE); 
    break;
    
    case BLOCK_ERASE_32K:
     GDflash_ReadWriteByte(GDFLASH_BLOCK_ERASE_32); 
    break;
    
    case BLOCK_ERASE_64K:
     GDflash_ReadWriteByte(GDFLASH_BLOCK_ERASE_64); 
    break;
    
    case CHIP_ERASE:
     GDflash_ReadWriteByte(GDFLASH_CHIP_ERASE); 
    break;
    
    default:
    break;
  } 
  
  if (erase_type != CHIP_ERASE)
  {
    GDflash_ReadWriteByte( (u8)((startAddr>> 16) &0xFF));
    GDflash_ReadWriteByte( (u8)((startAddr>> 8) &0xFF));
    GDflash_ReadWriteByte( (u8)(startAddr &0xFF));
  }
  
  DGFLASH_CS_HIGH();
}


/*****************************************************************************
 *

 *
 *
 *
 *
 *
 ��д���ַ����ÿҳ�Ŀ�ͷʱ������ҳ��ʣ��ռ�С��Ҫд������ݴ�С����ô���ݽ����ص�ǰ
 ��ַ��ʼ������ҳ�󣬴Ӵ�ҳ�Ŀ�ʼ�����ټ�����䣬ֱ�����
 д�����ݲ��ܳ���ÿҳ�Ĵ�С256�ֽڣ������Ĳ������ܱ�֤���256�ֽڵ����ݱ�д��flash

 ����д�뷽��,ֻ��д�����ܲ������ڵ�һ����������ʱ���ȫƬ����
*****************************************************************************/
#define GDFLASH_PAGE_SIZE  256

void GDflash_write(u32 startAddr, u8 *buf, u32 length)
{
   u32 current_startAddr  = startAddr;
   u8  first_page_byte    = GDFLASH_PAGE_SIZE - (startAddr % GDFLASH_PAGE_SIZE);
   u8 * current_data_ptr  = buf;
   u32 left_length        = length;
   u16 integrated_Pages; 
   u16 end_page_byte; 

   if (length > first_page_byte)
   {
       GDflash_page_program( current_startAddr, current_data_ptr, first_page_byte, FALSE); 

       current_startAddr = current_startAddr + first_page_byte;
       current_data_ptr += first_page_byte;
       left_length       = length - first_page_byte;
       integrated_Pages  = left_length / GDFLASH_PAGE_SIZE;
       end_page_byte     = left_length % GDFLASH_PAGE_SIZE;

      
      for (u16 i = 0; i < integrated_Pages; i++)
      {
        GDflash_page_program( current_startAddr, current_data_ptr, GDFLASH_PAGE_SIZE, FALSE); 
        current_startAddr += GDFLASH_PAGE_SIZE;
        current_data_ptr  += GDFLASH_PAGE_SIZE;
      }

      if (end_page_byte > 0)
      {
        GDflash_page_program( current_startAddr, current_data_ptr, end_page_byte, FALSE); 
      } 
   }
   else
   {
       GDflash_page_program( current_startAddr, current_data_ptr, length, FALSE); 
   }
}



void GDfalsh_read_identification(u8 *buf)
{
  DGFLASH_CS_LOW();
  
  GDflash_ReadWriteByte(0x9F);
  
  buf[0] = GDflash_ReadWriteByte(0xAA);
  buf[1] = GDflash_ReadWriteByte(0xAA);
  buf[2] = GDflash_ReadWriteByte(0xAA);
  
 DGFLASH_CS_HIGH();
}



void GDflash_128KByte_erase(void)
{
    GDflash_erase(0,  BLOCK_ERASE_64K);
    GDflash_erase(65536,  BLOCK_ERASE_64K);
}

void GDflash_read(u32 startAddr, u8 *buf, u32 length)
{
    GDflash_read_datas(startAddr, buf, length, FALSE);
}






void GDflash_test(void)
{
   u8 buf[] = "hello,world!";
   u8 buf1[20];
   u8 buf2[20];
   
   GDflash_init();

   GDflash_erase(4096,BLOCK_ERASE_64K);
   GDflash_read_datas(4096 , buf1, sizeof(buf), FALSE);
   GDflash_page_program(4096 , buf, sizeof(buf),TRUE);
   GDflash_read_datas(4096 , buf2, sizeof(buf), FALSE);
}



