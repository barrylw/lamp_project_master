#include "FRAM.h"
#include "stm32f10x_spi.h"

u8 FRAMReadWriteByte(u8 data)
{
  while(SPI_I2S_GetFlagStatus(FRAM_SPI,SPI_I2S_FLAG_TXE)==RESET);
  SPI_I2S_SendData(FRAM_SPI, data);

  while(SPI_I2S_GetFlagStatus(FRAM_SPI,SPI_I2S_FLAG_RXNE)==RESET);
  return (u8)(SPI_I2S_ReceiveData(FRAM_SPI));
}

void FRAM_GPIO_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(FRAM_CS_GPIO_CLK | FRAM_SPI_SCK_GPIO_CLK | FRAM_SPI_MISO_GPIO_CLK | FRAM_SPI_MOSI_GPIO_CLK , ENABLE);
  
   /*!< Configure sRF_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = FRAM_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(FRAM_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sRF_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = FRAM_SPI_MOSI_PIN;
  GPIO_Init(FRAM_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sRF_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = FRAM_SPI_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(FRAM_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
  
  /*!< Configure sRF_CS_PIN pin: sRF CS pin */
  GPIO_InitStructure.GPIO_Pin = FRAM_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(FRAM_CS_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_SetBits(GPIOA, GPIO_Pin_4);
}


void FRAM_sRF_SPI_Config(void)
{
  SPI_InitTypeDef SPI_InitStructure;
  
  RCC_APB2PeriphClockCmd(FRAM_SPI_CLK, ENABLE);
 
  FRAM_CS_HIGH();
  
  /* Disable sRF_SPI */
  SPI_Cmd(FRAM_SPI, DISABLE);
  
  /*!< SPI configuration */
  SPI_I2S_DeInit(FRAM_SPI);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(FRAM_SPI, &SPI_InitStructure);

  /*!< Enable the sRF_SPI  */
  SPI_Cmd(FRAM_SPI, ENABLE);
}

void FRAM_init(void)
{
  FRAM_GPIO_config();
  FRAM_sRF_SPI_Config();
}

void set_write_enable_latch(void)
{
    FRAM_CS_LOW();
    FRAMReadWriteByte(0x06);
    FRAM_CS_HIGH();  
}

void reset_write_enable_latch(void)
{
    FRAM_CS_LOW();
    FRAMReadWriteByte(0x04);
    FRAM_CS_HIGH();  
}

u8 read_status_reg(void)
{
   u8 retVal;
   FRAM_CS_LOW();
   FRAMReadWriteByte(0x05);
   retVal = FRAMReadWriteByte(0xAA);
   FRAM_CS_HIGH();  
   return retVal;
}

void write_status_reg(u8 val)
{
   FRAM_CS_LOW();
   FRAMReadWriteByte(0x01);
   FRAMReadWriteByte(val);
   FRAM_CS_HIGH();  
}

void read_memory_code(u16 addr, u8 *buf, u16 length)
{
   FRAM_CS_LOW();
   FRAMReadWriteByte(0x03);
                    
   FRAMReadWriteByte((u8)(addr/256));
   FRAMReadWriteByte((u8)(addr%256));
   
   for (u16 i = 0; i < length; i++)
   {
      buf[i] = FRAMReadWriteByte(0xAA);
   }
   FRAM_CS_HIGH(); 
}

void write_memory_code(u16 addr, u8 *buf, u16 length)
{
   if ((read_status_reg() & 0x02) == 0)
   {
      set_write_enable_latch();
   }
   
   FRAM_CS_LOW();
   FRAMReadWriteByte(0x02);
   FRAMReadWriteByte((u8)(addr/256));
   FRAMReadWriteByte((u8)(addr%256));
   for (u16 i = 0; i < length; i++)
   {
       FRAMReadWriteByte(buf[i]);
   }
   FRAM_CS_HIGH(); 
   
   reset_write_enable_latch();
   
}

void write_memory_const_value(u16 addr, u8 value, u16 length)
{
   if ((read_status_reg() & 0x02) == 0)
   {
      set_write_enable_latch();
   }
   
   FRAM_CS_LOW();
   FRAMReadWriteByte(0x02);
   FRAMReadWriteByte((u8)(addr/256));
   FRAMReadWriteByte((u8)(addr%256));
   for (u16 i = 0; i < length; i++)
   {
       FRAMReadWriteByte(value);
   }
   FRAM_CS_HIGH(); 
   
   reset_write_enable_latch();
}

void read_device_ID(u8 *buf)
{
  FRAM_CS_LOW();
  FRAMReadWriteByte(0x9F);
  buf[0] = FRAMReadWriteByte(0xAA);
  buf[1] = FRAMReadWriteByte(0xAA);
  buf[2] = FRAMReadWriteByte(0xAA);
  buf[3] = FRAMReadWriteByte(0xAA);
  FRAM_CS_HIGH();  
}

void fast_read_memory_code(u16 addr, u8 *buf, u16 length)//fast to 33M
{
   FRAM_CS_LOW();
   FRAMReadWriteByte(0x0B);
   FRAMReadWriteByte((u8)(addr/256));
   FRAMReadWriteByte((u8)(addr%256));
   FRAMReadWriteByte(0xAA);
   for (u16 i = 0; i < length; i++)
   {
      buf[i] = FRAMReadWriteByte(0xAA);
   }
   FRAM_CS_HIGH(); 
}

void FRAM_test(void)
{
  u8 framTest[] = "I am a girl";
  u8 ffff[15];
  
  FRAM_init();
  
  read_device_ID(framTest);
  
  write_memory_code(0 , framTest, sizeof(framTest));
  
  read_memory_code(0, ffff,  sizeof(framTest));
}

