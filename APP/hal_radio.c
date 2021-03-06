/**
  ******************************************************************************
  * @file    hal_radio.c 
  * @author  William Liang
  * @version V1.0.0
  * @date    09/10/2013
  * @brief   This file contains the initialization and handle of the radio frequency.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "hal_radio.h"
#include "Uart.h"
#include "Rtc.h"
#include "sx1276-LoRa.h"
#include "sx1276-Fsk.h"


u8 test[0x70] ;
/** @addtogroup RADIO
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

extern u8 phyCurrentChannel;
extern u8 phyCurrentChannelGroup;
extern u8 phyCCAMode;
extern u32 phyChannelsSupported;
extern u8 phyTransmitPower;
extern u8 phyRSSIThreshold;
extern u32 sRFTestStartTime;
extern u32 sRFTestPeriod;
extern u8 phyCCATimout;
/* BEGIN: Added by Barry, 2014/3/6 */
extern st_RF_LoRa_DypeDef g_sRF;
extern tLoRaSettings LoRaSettings;
/* END:   Added by Barry, 2014/3/6 */


ST_EVENT_CONTROL sRFTransmitEvent = {0xFF, 0};
ST_EVENT_CONTROL sRFReceiveEvent = {0xFF, 0};
ST_EVENT_CONTROL sRFCCAEvent = {0xFF, 0};

ST_EVENT_CONTROL sRFTestMasterEvent = {0xFF, 0};
ST_EVENT_CONTROL sRFTestSlaverEvent = {0xFF, 0};

ST_EVENT_METHOD RFEvents[] =
{
  {&sRFTransmitEvent, TimeOutHandle},
  {&sRFReceiveEvent, TimeOutHandle},
  {&sRFTestMasterEvent, NULL},
  {&sRFTestSlaverEvent, NULL},
  {&sRFCCAEvent, NULL},
};


extern u8 test2[];

extern sRF_FSK g_fsk ;
extern st_RF_LoRa_DypeDef g_RF_LoRa;
bool g_slaveMode = true;
//static u16 txcount = 0;
//static u16 rxCount = 0;
extern u8 tedtbuf[];
struct etimer timer_rf; 

#define RF_RESET_TIME_S       1800

static u8 pid[6] = {0x01, 0x00, 0x55, 0x07, 0x00, 0x00};
extern u8 g_UartRxBuffer[];

u8 read_version[4]  = {0x02, 0x00, 0x00, 0x6A}; //读版本号
u8 cmd_read_data[4] = {0x0F, 0x00, 0x00, 0x61}; //读数据，读所有数据

/*
ttypedef enum
{
  START_645_POINT =0,
  ADDR_645_POINT = 1,
  CONTROL_645_POINT = 8,
  DATA_LENGTH_645_POINT = 9,
  DATA_MARK_645_POINT = 10,
  DATA_PACKET_645_POINT = 14
}em_645_point;
*/

/* 不同为0， 相同为1 */
u8 cmp(u8 * buf1, u8* buf2, u8 length)
{
  for (u8 i = 0; i < length; i++)
  {
     if ( buf1[i] != buf2[i] )
     {
       /* 不同 */
       return 0;
     }
  }
    /* 相同 */
   return 1;
}


void hal_radio_645_process(void)
{
  u8 len_645;
  u8 info_sourceadd[16]; 
  
  printf(" radio receive ok \r\n");
 
 if ((g_RF_LoRa.rf_DataBuffer[START_645_POINT] == 0x68) && (g_RF_LoRa.rf_DataBuffer[START_645_POINT + 7] == 0x68) && 
       (g_RF_LoRa.rf_DataBuffer[ g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 11] == 0x16))
 {
     if (GetChecksum(g_RF_LoRa.rf_DataBuffer, 10 + g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT]) == g_RF_LoRa.rf_DataBuffer[g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 10])
     {
       len_645 = g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 12;
       MemCpy( info_sourceadd, &g_RF_LoRa.rf_DataBuffer[len_645],  18);
        
       if (g_RF_LoRa.rf_DataBuffer[CONTROL_645_POINT] & 0x80)
       {
          if ((cmp(read_version, &g_RF_LoRa.rf_DataBuffer[DATA_MARK_645_POINT] , 4) == 0))
          {
              g_UartRxBuffer[0] = 0x68;

              g_UartRxBuffer[1] = (g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 12 + 31) & 0xFF;
              g_UartRxBuffer[2] = ((g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 12 + 31)>>8) & 0xFF;

              g_UartRxBuffer[3] = 0x81;

              //信息域，源地址，目的地址
              MemCpy(&g_UartRxBuffer[4], info_sourceadd,18);
           
              g_UartRxBuffer[22] = 0x13;
              
              g_UartRxBuffer[23] = 0x01;
              g_UartRxBuffer[24] = 0x00;
              
              g_UartRxBuffer[25] = 0x00;
              g_UartRxBuffer[26] = 0x00;
              g_UartRxBuffer[27] = 0x02;
              
              g_UartRxBuffer[28] = g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 12;
              
              MemCpy(&g_UartRxBuffer[29],&g_RF_LoRa.rf_DataBuffer,g_UartRxBuffer[28]);
              
              g_UartRxBuffer[g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 12 + 31 -2] = GetChecksum(&g_UartRxBuffer[3],  g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 12 + 31 -5);
             
              g_UartRxBuffer[g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 12 + 31 -1] = 0x16;
              
              Uart_Send(COM1, g_UartRxBuffer, g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 12 + 31);
              
              for (u8 i = 0; i < g_RF_LoRa.rf_DataBuffer[DATA_LENGTH_645_POINT] + 12 + 31; i++)
              {
                printf("%.2x ", g_UartRxBuffer[i]);
              }
              printf("\r\n");
          }
          else
          {
              g_UartRxBuffer[0] = 0x68;

              g_UartRxBuffer[1] = 36;
              g_UartRxBuffer[2] = 0;

              g_UartRxBuffer[3] = 0x81;

              //信息域，源地址，目的地址
              MemCpy(&g_UartRxBuffer[4], info_sourceadd,18);

              g_UartRxBuffer[22] = 0x03;

              g_UartRxBuffer[23] = 0x01;
              g_UartRxBuffer[24] = 0x00;
              
              MemCpy(&g_UartRxBuffer[25], &g_RF_LoRa.rf_DataBuffer[DATA_PACKET_645_POINT],9);
            
              g_UartRxBuffer[34] = GetChecksum(&g_UartRxBuffer[3], 31);
              g_UartRxBuffer[35] = 0x16;

              Uart_Send(COM1, g_UartRxBuffer, 36);
              
              for (u8 i = 0; i < 36; i++)
              {
                printf("%.2x ", g_UartRxBuffer[i]);
              }
              printf("\r\n");
          }
       }
    
     }
 }
}
/*****************************************************************************
 Prototype    : hal_RunRFEvents
 Description  : brief  Handle Radio events in main loop.
 Input        : ST_EVENT_METHOD *pEvents  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
PROCESS(hal_RF_process, "radio_process ");
#ifdef USE_LORA_MODE
PROCESS_THREAD(hal_RF_process, ev, data)
{
  PROCESS_BEGIN();
  
  while(1)
  {
    PROCESS_WAIT_EVENT(); 

    if (ev == PROCESS_EVENT_MSG)
    {
        if (*((tRFLRStates*)data) == RFLR_STATE_TX_RUNNING)   
        {
            etimer_set(&timer_rf, CLOCK_CONF_SECOND*2);
            //printf("tx start\r\n");
        }
        else if (*((tRFLRStates*)data) == RFLR_STATE_TX_DONE)  
        {
           //printf("tx done\r\n");
           SX1276LoRa_Receive_Packet(false); 
        }
        else  if (*((tRFLRStates*)data) == RFLR_STATE_RX_RECEIVEING) 
        {
           etimer_set(&timer_rf, CLOCK_CONF_SECOND*2);
           //printf("rx start\r\n");
        } 
        else if (*((tRFLRStates*)data) == RFLR_STATE_RX_RUNNING) 
        {
          printf("rssi = %f, snr = %d\r\n", SX1276LoRaGetPacketRssi(), SX1276LoRaGetPacketSnr());
       
          if (g_RF_LoRa.rf_DataBufferValid)
          {
              g_RF_LoRa.rf_DataBufferValid = false;
              
              LED_On(TX_LED);
             
              //Delayms(100);
              
              hal_radio_645_process();
         
              /*
              hal_UartDMATx(COM1, g_RF_LoRa.rf_DataBuffer, g_RF_LoRa.rf_RxPacketSize);
             
          
         
              for (u8 i = 0; i < g_RF_LoRa.rf_RxPacketSize; i++ )
              {
                printf("%x ",g_RF_LoRa.rf_DataBuffer[i] );
              }
              printf("\r\n");
              */
        }
    }
    else if (ev == PROCESS_EVENT_TIMER)
    {
       if ((struct etimer *)data == &timer_rf)
       {
         if (SX1276LoRaGetRFState() == RFLR_STATE_TX_RUNNING)   
         {
            printf("tx time out\r\n");
         }
         if (SX1276LoRaGetRFState() ==  RFLR_STATE_RX_RECEIVEING)
         {
            printf("rx time out\r\n");
         }  
         hal_sRF_InitSPI();
         SX1276LoRa_Receive_Packet(false);
       }
    }
  }
 }
  PROCESS_END();
}
#else
  
PROCESS_THREAD(hal_RF_process, ev, data)
{
  PROCESS_BEGIN();

  while(1)
  {
    PROCESS_WAIT_EVENT(); 

    if (ev == PROCESS_EVENT_MSG)
    {
        if (*((tRFStates*)data) == RF_STATE_TX_RUNNING)   
        {
            printf("tx start\r\n");
        }
        else if (*((tRFStates*)data) == RF_STATE_TX_DONE)  
        {
           printf("tx done\r\n");
           SX1276Fsk_recrive_Packet();
        }
        else  if (*((tRFStates*)data) == RF_STATE_RX_PREAMBLE) 
        {
            //printf("rx start\r\n");
            etimer_set(&timer_rf, CLOCK_CONF_SECOND*3);
        }
        else if (*((tRFStates*)data) == RF_STATE_RX_DONE) 
        {
          #if 0
           printf("rx done\r\n");
           printf("rssi = %f\r\n",g_fsk.rssi);
           for (u8 i = 0; i < g_fsk.packetLenth; i++ )
           {
              printf("%d ",g_fsk.buffer[i]);
           }
           printf("\r\n");
          #endif
         
           hal_UartDMATx(COM1, g_fsk.buffer,g_fsk.packetLenth);
           SX1276Fsk_recrive_Packet();
        }
    }
    else if (ev == PROCESS_EVENT_TIMER)
    {
         if ((struct etimer *)data == &timer_rf)
         {
           if (g_fsk.states == RF_STATE_TX_RUNNING)   
           {
              printf("tx time out\r\n");
           }
           else
           {
              hal_sRF_InitSPI();
              SX1276Fsk_recrive_Packet();
              printf("rx time out\r\n");
           }  
         }
       
    }
 }
 PROCESS_END();
}
#endif

struct etimer timer_RFreset;
PROCESS(hal_RF_reset, "radio_reset");
PROCESS_THREAD(hal_RF_reset, ev, data)
{
  PROCESS_BEGIN();

  while(1)
  {
    etimer_set(&timer_RFreset, CLOCK_CONF_SECOND*RF_RESET_TIME_S);
    
    PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_TIMER) && ((struct etimer *)data == &timer_RFreset));

    while(1)
    {
       #ifdef USE_LORA_MODE
       if ((SX1276LoRaGetRFState() != RFLR_STATE_TX_RUNNING) &&  (SX1276LoRaGetRFState() != RFLR_STATE_RX_RECEIVEING))
      #else
        if ((g_fsk.states != RF_STATE_TX_RUNNING) && (g_fsk.states != RF_STATE_RX_PREAMBLE) && (g_fsk.states != RF_STATE_RX_SYNC) && (g_fsk.states != RF_STATE_RX_RUNNING)) 
      #endif
       {
         hal_InitRF();
         break;
       }
       else
       {
          process_poll(&hal_RF_reset);
          PROCESS_WAIT_EVENT(); 
       }
    }
    
  }
  PROCESS_END();
}



#if 0
struct etimer test_send_timer; 
PROCESS(hal_long_send, "long_send_process ");

PROCESS_THREAD(hal_long_send, ev, data)
{
  static u8 length = 50;
  static bool first = true;
  
  PROCESS_BEGIN();
  
  if (ev == PROCESS_EVENT_INIT)
  {
    if (data != NULL)
    {
      length = *((u8*)data);
    }
  }
  
  while(1)
  {
    if (first)
    {
      printf("start the tx timer!\r\n");
      first = false;
      etimer_set(&test_send_timer, CLOCK_CONF_SECOND*2);
    }
    PROCESS_WAIT_EVENT();
    
    if (ev == PROCESS_EVENT_TIMER)
    {
      txcount++;
      tedtbuf[0] = (u8)(txcount>>8);
      tedtbuf[1] = (u8)(txcount & 0xFF);
      
      #ifndef USE_LORA_MODE
        SX1276Fsk_Send_Packet(tedtbuf, length);
      #else
        SX1276LoRa_Send_Packet(tedtbuf, length);
      #endif
        etimer_set(&test_send_timer, CLOCK_CONF_SECOND*2);
      
    }
  }
  PROCESS_END();
}
#endif


PROCESS(apl_update_slaveNode_process, "update_slave");
PROCESS_THREAD(apl_update_slaveNode_process, ev, data)
{
    static struct etimer update_slave_timer; 
    static u8 buf[200];
    static ST_update_slave_info update_slave;
    static u32 left_data;
    static bool update_status_paused = false;
    

    PROCESS_BEGIN();
    
    while (1)
    {
         PROCESS_WAIT_EVENT();
         
         if (ev == PROCESS_EVENT_MSG)
         {
              if (((ST_UPDATE*)data)->status != UPDATE_SALVE)
              {     
                 printf("no update file\r\n");
                 continue;
              }
              else
              {
                  update_status_paused = false;
                  printf("start update slave node\r\n");
                  update_slave.version            = ((ST_UPDATE*)data)->version;
                  update_slave.lastBytes          = ((ST_UPDATE*)data)->lastBytes;
                  update_slave.total_packets      = ((ST_UPDATE*)data)->total_packets;
                  left_data                       = (((ST_UPDATE*)data)->total_packets - 1)*128 +  update_slave.lastBytes;
                  update_slave.total_bytes        = left_data;

                  update_slave.current_packet_No  = 0;
                  update_slave.packet_length      =  (left_data >=64)? 64: left_data;   
                  etimer_set(&update_slave_timer, 1000);
              }
         }/*
         else if (ev == UPDATE_STOP)
         {
           update_status_paused = true;
           etimer_stop(&update_slave_timer);
           PROCESS_WAIT_EVENT();
         }
         else if (ev == UPDATE_CONTINUE)
         {
             if (update_status_paused)
             {
                 update_status_paused = false;
                 printf("continue update\r\n");
                 etimer_set(&update_slave_timer, 100);
             }
         }*/
         else if (  (ev == PROCESS_EVENT_TIMER) && ((struct etimer *)data == &update_slave_timer) )
         {
           if ( update_slave.current_packet_No < update_slave.total_packets)
           {
                buf[0]  = 0x68;                                    //长度10
                MemSet(&buf[1], 0x99, 6); 
                buf[7]  = 0x68;
                buf[8]  = 0x14; //写
                buf[9]  = update_slave.packet_length + 4 +9;          //L
                
                buf[10] = 0x01;                                    // 长度4
                buf[11] = 0x00;
                buf[12] = 0x00;
                buf[13] = 0x6A;
                
               buf[14] = (u8)((update_slave.version >> 8) & 0xFF);
               buf[15] = (u8)(update_slave.version  & 0xFF);
               
               buf[16] = (u8)((update_slave.total_bytes >> 8) & 0xFF);
               buf[17] = (u8)(update_slave.total_bytes  & 0xFF);
               
               buf[18] = (u8)((update_slave.total_packets >> 8) & 0xFF);
               buf[19] = (u8)(update_slave.total_packets  & 0xFF);
               
               buf[20] = (u8)((update_slave.current_packet_No >> 8) & 0xFF);
               buf[21] = (u8)(update_slave.current_packet_No  & 0xFF);
               
               buf[22] = (u8)(update_slave.packet_length  & 0xFF);   
            
               GDflash_read(update_slave.current_packet_No*64, &buf[23],  update_slave.packet_length); // update_slave.packet_length
           
               for (u8 i = 0; i < buf[9]; i++)
               {
                 buf[10 + i] += 0x33;
               }
                buf[23 + update_slave.packet_length] = GetChecksum(buf,  23 + update_slave.packet_length);
                buf[24 + update_slave.packet_length] = 0x16;
                
                printf("update packet %d\r\n",  update_slave.current_packet_No);
                SX1276LoRa_Send_Packet(buf, 25 + update_slave.packet_length);
                etimer_set(&update_slave_timer, 3000);
                PROCESS_WAIT_EVENT_UNTIL ( (ev == PROCESS_EVENT_TIMER) && ((struct etimer *)data == &update_slave_timer) );
                
                left_data                     -= update_slave.packet_length;
                update_slave.packet_length     =  (left_data >=64)? 64: left_data;
                update_slave.current_packet_No++;
          }
          else
          {
              printf("update finish\r\n");
          }
          
      }
    }
    PROCESS_END();
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
u8 spiReadWriteByte(u8 data)
{
  while(SPI_I2S_GetFlagStatus(sRF_SPI,SPI_I2S_FLAG_TXE)==RESET);
  SPI_I2S_SendData(sRF_SPI, data);

  while(SPI_I2S_GetFlagStatus(sRF_SPI,SPI_I2S_FLAG_RXNE)==RESET);
  return (u8)(SPI_I2S_ReceiveData(sRF_SPI));
}

/*****************************************************************************
 Prototype    : SX1276WriteBuffer
 Description  : spi write buffer
 Input        : uint8_t addr     
                uint8_t *buffer  
                uint8_t size     
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    sRF_CS_LOW();
    spiReadWriteByte(addr|0x80);

    for(u8 i = 0;i < size; i++)
    {
      spiReadWriteByte(buffer[i]);
    }
    sRF_CS_HIGH();  
}

/*****************************************************************************
 Prototype    : SX1276ReadBuffer
 Description  : none
 Input        : uint8_t addr     
                uint8_t *buffer  
                uint8_t size     
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
  sRF_CS_LOW();
   
  spiReadWriteByte(addr);

  for(u8 i = 0;i < size; i++)
  {
    buffer[i] = spiReadWriteByte(0xFF);
  }
  sRF_CS_HIGH();
}

/*****************************************************************************
 Prototype    : SX1276Write
 Description  : 1276 write Reg
 Input        : uint8_t addr  
                uint8_t data  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276Write( uint8_t addr, uint8_t data )
{
    SX1276WriteBuffer( addr, &data, 1 );
}

/*****************************************************************************
 Prototype    : SX1276Read
 Description  : 1276 read Reg
 Input        : uint8_t addr   
                uint8_t *data  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276Read( uint8_t addr, uint8_t *data )
{
    SX1276ReadBuffer( addr, data, 1 );
}

/*****************************************************************************
 Prototype    : SX1276WriteFifo
 Description  : none
 Input        : uint8_t *buffer  
                uint8_t size     
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1276WriteBuffer( sRF_FIFO_ARRD, buffer, size );
}

/*****************************************************************************
 Prototype    : SX1276ReadFifo
 Description  : none
 Input        : uint8_t *buffer  
                uint8_t size     
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1276ReadBuffer( sRF_FIFO_ARRD, buffer, size );
}


/*****************************************************************************
 Prototype    : SX1276SetReset
 Description  : none
 Input        : uint8_t state  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276SetReset( uint8_t state )
{
  if( state == RADIO_RESET_ON )
  {
    // Set RESET pin to 0
    GPIO_WriteBit( sRF_RESET_PORT, sRF_RESET_PIN, Bit_RESET );
  }
  else
  {
    GPIO_WriteBit( sRF_RESET_PORT, sRF_RESET_PIN, Bit_SET );
  }
}

/*****************************************************************************
 Prototype    : hal_sRF_ITConfig
 Description  : none
 Input        : en_GDOx_IrqLine irqLine   
                FunctionalState NewState  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void hal_sRF_ITConfig(uint32_t irqLine, FunctionalState NewState)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  
  EXTI_InitStructure.EXTI_Line = irqLine;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = NewState;
  EXTI_Init(&EXTI_InitStructure); 
}

void hal_sRF_FSK_ITConfig( uint32_t irqLine, FunctionalState NewState)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  
  EXTI_InitStructure.EXTI_Line = irqLine;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = NewState;
  EXTI_Init(&EXTI_InitStructure); 
}

/*****************************************************************************
 Prototype    : hal_Init_RF_pins
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void hal_Init_RF_pins(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
 RCC_APB2PeriphClockCmd(sRF_CS_GPIO_CLK | sRF_SPI_MOSI_GPIO_CLK | sRF_SPI_MISO_GPIO_CLK |
                         sRF_SPI_SCK_GPIO_CLK | sRF_DIOx_SCK | sRF_RESET_SCK, ENABLE);

  hal_DIOx_ITConfig(all,DISABLE);
  
  /*!< sRF_SPI Periph clock enable */
  RCC_APB1PeriphClockCmd(sRF_SPI_CLK, ENABLE);

  /* Enable sRF SPI DMA clock */
  RCC_AHBPeriphClockCmd(sRF_SPI_DMA_CLK, ENABLE);

  /* Disable sRF_IRQ_EXTI clock */
   RCC_APB2PeriphResetCmd(RCC_APB2Periph_AFIO, DISABLE);
   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = sRF_RESET_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init( sRF_RESET_PORT, &GPIO_InitStructure );
  
  /*!< Configure sRF_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = sRF_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(sRF_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sRF_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = sRF_SPI_MOSI_PIN;
  GPIO_Init(sRF_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sRF_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = sRF_SPI_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(sRF_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
  
  /*!< Configure sRF_CS_PIN pin: sRF CS pin */
  GPIO_InitStructure.GPIO_Pin = sRF_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(sRF_CS_GPIO_PORT, &GPIO_InitStructure);

   /*!< Configure sRF_IRQ_PINs pin: DDO0~GDO5 */
  GPIO_InitStructure.GPIO_Pin = sRF_DIO0_PIN | sRF_DIO1_PIN | sRF_DIO2_PIN
                                             | sRF_DIO3_PIN  ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(sRF_DIOx_PORT, &GPIO_InitStructure);
  
  /*TX LED pin*/
  GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init( GPIOA, &GPIO_InitStructure );
  
  /* Enable AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

}

/*****************************************************************************
 Prototype    : GPIO_int_Config
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void GPIO_int_Config(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  EXTI_ClearITPendingBit(DIO0_IRQ);
  EXTI_ClearITPendingBit(DIO1_IRQ);
  EXTI_ClearITPendingBit(DIO2_IRQ);
  EXTI_ClearITPendingBit(DIO3_IRQ);
  
 /* Connect EXTI0 Line to PC.0 pin */
  GPIO_EXTILineConfig(sRF_DIOx_PORT_SOURCE, sRF_DIO0_PIN_SOURCE);
  /* Connect EXTI1 Line to PC.1 pin */
  GPIO_EXTILineConfig(sRF_DIOx_PORT_SOURCE, sRF_DIO1_PIN_SOURCE);
  /* Connect EXTI2 Line to PC.2 pin */
  GPIO_EXTILineConfig(sRF_DIOx_PORT_SOURCE, sRF_DIO2_PIN_SOURCE);
  /* Connect EXTI3 Line to PC.3 pin */
  GPIO_EXTILineConfig(sRF_DIOx_PORT_SOURCE, sRF_DIO3_PIN_SOURCE);

  hal_DIOx_ITConfig(all,DISABLE);
}

/*****************************************************************************
 Prototype    : hal_sRF_SPI_Config
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void hal_sRF_SPI_Config(void)
{
  SPI_InitTypeDef SPI_InitStructure;
 
  /*!< Deselect the RF: Chip Select high */
  sRF_CS_HIGH();
  
  /* Disable sRF_SPI */
  SPI_Cmd(sRF_SPI, DISABLE);
  
  /*!< SPI configuration */
  SPI_I2S_DeInit(sRF_SPI);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(sRF_SPI, &SPI_InitStructure);

  /*!< Enable the sRF_SPI  */
  SPI_Cmd(sRF_SPI, ENABLE);
}

/*****************************************************************************
 Prototype    : init_SPI_DMA
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
#ifdef SPI_DMA_FIFO
void init_SPI_DMA(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  /* Enable SPI Rx and Tx request */
  SPI_I2S_DMACmd(sRF_SPI, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

  /* sRF_SPI_DMA_RX_Channel configuration ---------------------------------------------*/
  DMA_DeInit(sRF_SPI_DMA_RX_Channel);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)sRF_SPI_DR_Base;
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(sRF_SPI_DMA_RX_Channel, &DMA_InitStructure);

  /* sRF_SPI_DMA_TX_Channel configuration ---------------------------------------------*/
  DMA_DeInit(sRF_SPI_DMA_TX_Channel);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)sRF_SPI_DR_Base;
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(sRF_SPI_DMA_TX_Channel, &DMA_InitStructure);
}
#endif
/*****************************************************************************
 Prototype    : hal_sRF_InitSPI
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void hal_sRF_InitSPI(void)
{
  hal_Init_RF_pins();
  hal_sRF_SPI_Config();
  
  #ifdef SPI_DMA_FIFO
  init_SPI_DMA();
  #endif
}

#if 0
/*****************************************************************************
 Prototype    : hal_sRF_ReadRegister
 Description  : none
 Input        : u8 reg  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
u8 hal_sRF_ReadRegister(u8 reg)
{
  if (reg > TOTAL_REGISTER_NUMBER)
  {
    printf("regsiter input error\r\n");
    return 0;
  }
  
  /*!< Deselect the Radio: Chip Select high */
  sRF_CS_HIGH();
  
  /*!< Select the radio by pulling low the nSEL pin */
  sRF_CS_LOW();
  
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(sRF_SPI, reg);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /* read from the SPI bus */
  SPI_I2S_ReceiveData(sRF_SPI);

  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(sRF_SPI, sRF_DUMMY_BYTE);
  
  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  reg = SPI_I2S_ReceiveData(sRF_SPI);

  /*!< Deselect the radio by pulling high the nSEL pin */
  sRF_CS_HIGH();
  
  return reg;
}

/*****************************************************************************
 Prototype    : hal_sRF_WriteRegister
 Description  : none
 Input        : u8 reg  
                u8 val  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void hal_sRF_WriteRegister(u8 reg, u8 val)
{
  reg |= 0x80;
  
  /*!< Deselect the Radio: Chip Select high */
  sRF_CS_HIGH();
  
  /*!< Select the radio by pulling low the nSEL pin */
  sRF_CS_LOW();
  
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(sRF_SPI, reg);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /* read from the SPI bus */
  SPI_I2S_ReceiveData(sRF_SPI);

  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(sRF_SPI, val);
  
  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /* read from the SPI bus */
  SPI_I2S_ReceiveData(sRF_SPI);

  /*!< Deselect the radio by pulling high the nSEL pin */
  sRF_CS_HIGH();
}
#endif

/*****************************************************************************
 Prototype    : hal_sRF_DMA_Read
 Description  : none
 Input        : u8 startReg  
                u8 *pBuffer  
                u8 length    
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
#ifdef SPI_DMA_FIFO
void hal_sRF_DMA_Read(u8 startReg, u8 *pBuffer, u8 length)
{
  /* Allocate storage for CPU status register      */
#if OS_CRITICAL_METHOD == 3u
  OS_CPU_SR cpu_sr = 0u;
#endif

  if (startReg > TOTAL_REGISTER_NUMBER)
  {
    return;
  }

  if (length > 0)
  {
    /*!< Deselect the Radio: Chip Select high */
    sRF_CS_HIGH();
    
    /*!< Select the radio by pulling low the nSEL pin */
    sRF_CS_LOW();
    
    /*!< Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_TXE) == RESET);

    /*!< Send byte through the SPI1 peripheral */
    SPI_I2S_SendData(sRF_SPI, startReg);

    /*!< Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(sRF_SPI, SPI_I2S_FLAG_RXNE) == RESET);

    /* read from the SPI bus */
    SPI_I2S_ReceiveData(sRF_SPI);

    OS_ENTER_CRITICAL();
    
    sRF_SPI->CR1 |= SPI_Direction_2Lines_RxOnly;
    
    /* sRF_SPI_DMA_RX_Channel configuration ---------------------------------------------*/
    sRF_SPI_DMA_RX_Channel->CMAR = (u32)pBuffer;
    sRF_SPI_DMA_RX_Channel->CNDTR = (u32)length;
    
    /* Enable sRF_SPI_DMA_RX_Channel and TC interrupt*/
    sRF_SPI_DMA_RX_Channel->CCR |= DMA_CCR1_EN | DMA_CCR1_TCIE;

    OS_EXIT_CRITICAL();
  }
 
}
#endif
/*****************************************************************************
 Prototype    : hal_sRF_DMA_Write
 Description  : none
 Input        : u8 *pBuffer  
                u8 length    
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
#ifdef SPI_DMA_FIFO
void hal_sRF_DMA_Write(u8 *pBuffer, u8 length)
{
  if (length > 0)
  {
    /*!< Deselect the Radio: Chip Select high */
    sRF_CS_HIGH();
    
    /*!< Select the radio by pulling low the nSEL pin */
    sRF_CS_LOW();
    
    /* sRF_SPI_DMA_TX_Channel configuration ---------------------------------------------*/
    sRF_SPI_DMA_TX_Channel->CMAR = (u32)pBuffer;
    sRF_SPI_DMA_TX_Channel->CNDTR = (u32)length;
    
    /* Enable sRF_SPI_DMA_TX_Channel */
    sRF_SPI_DMA_TX_Channel->CCR |= DMA_CCR1_EN | DMA_CCR1_TCIE;
  }
  else
  {
    printf("length input error\r\n");
  }
}
#endif
/*****************************************************************************
 Prototype    : hal_sRF_Config
 Description  : none 
 Input        : u8 startReg  
                u8 *pBuffer  
                u8 length    
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
#ifdef SPI_DMA_FIFO
void hal_sRF_Config(u8 startReg, u8 *pBuffer, u8 length)
{
  if (startReg >= TOTAL_REGISTER_NUMBER)
  {
    printf("startReg input error\r\n");
    return;
  }
  if (length > 0)
  {
    hal_sRF_InitSPI();
    *pBuffer = startReg | 0x80;
    hal_sRF_DMA_Write(pBuffer, length + 1);
   
  }
}
#endif
/*****************************************************************************
 Prototype    : hal_sRF_Read
 Description  : DMA read FIFO
 Input        : u8 startReg  
                u8 *pBuffer  
                u8 length    
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
#ifdef SPI_DMA_FIFO
void hal_sRF_Read(u8 startReg, u8 *pBuffer, u8 length)
{
  if (length > 0)
  {
    hal_sRF_InitSPI();
    hal_sRF_DMA_Read(startReg,pBuffer,length);
  }
}
#endif

/*****************************************************************************
 Prototype    : hal_sRF_readFIFO_DMA
 Description  : none
 Input        : u8 * pBuffer指向接收数组，数组的第一个字节为长度字节
                u8 length     
 Output       : None
 Return Value : 
 Date         : 2014/3/21
 Author       : Barry
*****************************************************************************/
#ifdef SPI_DMA_FIFO
void hal_sRF_readFIFO_DMA(u8 * pBuffer, u8 length)
{

  hal_sRF_DMA_Read(0,pBuffer+1,length);  
}

/*****************************************************************************
 Prototype    : hal_sRF_writeFIFO_DMA
 Description  : none
 Input        : u8 * pBuffer 指向开始发送的数组，数组的第一个字节为FIFO地址 
                u8   length  实际发送的字节数   
 Output       : None
 Return Value : 
 Date         : 2014/3/21
 Author       : Barry
*****************************************************************************/
void hal_sRF_writeFIFO_DMA(u8 * pBuffer, u8 length)
{
  hal_sRF_Config(0,pBuffer,length);
}
#endif


/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/


/*****************************************************************************
 Prototype    : DMA1_Channel2_IRQHandler
 Description  : This function handles SPI1 Rx Transfer Complete interrupt
                and Transfer Error interrupt.
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
#ifdef SPI_DMA_FIFO
void DMA1_Channel2_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_IT_TC2) != RESET)
  {
    /* hal_sRF_InitSPI();初始化放在主循环中去 */
    RxEndProcess(TRUE);
  }
  
  /*!< Deselect the RADIO: Chip Select high */
  //sRF_CS_HIGH();
  
  /* Disable the selected SPI peripheral */
  //sRF_SPI->CR1 &= 0xFFBF;
  
  /* Clear the DMA1 interrupt pending bits */
  DMA1->IFCR = DMA1_IT_TC2 | DMA1_IT_HT2 | DMA1_IT_TE2;

  /* Disable the selected DMA1_Channel2 */
  DMA1_Channel2->CCR &= (u16)(~DMA_CCR1_EN);
}

/*****************************************************************************
 Prototype    : DMA1_Channel3_IRQHandler
 Description  : This function handles SPI1 Tx Transfer Complete interrupt
                and Transfer Error interrupt.
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void DMA1_Channel3_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_IT_TC3) != RESET)
  {
    SX1276LoRaStartTransmit();
  }
  
  /* Clear the DMA1 interrupt pending bits */
  DMA1->IFCR = DMA1_IT_TC3 | DMA1_IT_HT3 | DMA1_IT_TE3;
  
  /* Disable the selected DMA1_Channel3 */
  DMA1_Channel3->CCR &= (u16)(~DMA_CCR1_EN);
  
}
#endif

/*****************************************************************************
 Prototype    : hal_sRF_Transmit
 Description  : PHY send
 Input        : u8 *pBuffer  
                u8 length    
                u8 channel   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2014/10/29
    Author       : liwei
    Modification : Created function

*****************************************************************************/
void hal_sRF_Transmit(u8 *pBuffer, u8 length, u8 channel)
{
  SX1276LoRa_Send_Packet(pBuffer,length);
}


/*****************************************************************************
 Prototype    : hal_InitRF
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void hal_InitRF(void)
{ 
  etimer_stop(&timer_rf);
  hal_sRF_InitSPI();
  GPIO_int_Config();
#ifdef USE_LORA_MODE
  SX1276_lora_init(true);
#else
  SX1276_lora_init(false);
#endif
  printf("reset RF\r\n");
}


/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/

