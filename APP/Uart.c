/**
  ************************************************************************************
  * @file    hal_uart.c
  * @author  William Liang
  * @version V1.0.0
  * @date    07/22/2013
  * @brief   This file contains the initialization & interrupt handlers of the uarts.
  ************************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/


#include "Uart.h"
#include "Cmd.h"
#include "MCP.h"
#include "Led.h"
#include "sx1276-LoRa.h"
#include "hal_gdflash.h"
/** @addtogroup USART
  * @{
  */


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
USART_TypeDef *COM_USART[COMn] = {RBL_COM1, RBL_COM2}; 
const uint32_t COM_BAUD[COMn] = {RBL_COM1_BAUD, RBL_COM2_BAUD};
GPIO_TypeDef *COM_TX_PORT[COMn] = {RBL_COM1_TX_GPIO_PORT, RBL_COM2_TX_GPIO_PORT};
GPIO_TypeDef *COM_RX_PORT[COMn] = {RBL_COM1_RX_GPIO_PORT, RBL_COM2_RX_GPIO_PORT};
const uint32_t COM_USART_CLK[COMn] = {RBL_COM1_CLK, RBL_COM2_CLK};
const uint32_t COM_TX_PORT_CLK[COMn] = {RBL_COM1_TX_GPIO_CLK, RBL_COM2_TX_GPIO_CLK};
const uint32_t COM_RX_PORT_CLK[COMn] = {RBL_COM1_RX_GPIO_CLK, RBL_COM2_RX_GPIO_CLK};
const uint16_t COM_TX_PIN[COMn] = {RBL_COM1_TX_PIN, RBL_COM2_TX_PIN};
const uint16_t COM_RX_PIN[COMn] = {RBL_COM1_RX_PIN, RBL_COM2_RX_PIN};
const uint32_t COM_DMA_CLK[COMn] = {RBL_COM1_DMA_CLK, RBL_COM2_DMA_CLK};
DMA_Channel_TypeDef *COM_TX_DMA_CHAN[COMn] = {RBL_COM1_TX_DMA_CHANNEL, RBL_COM2_TX_DMA_CHANNEL};
const uint32_t COM_DR_BASE[COMn] = {RBL_COM1_DR_BASE, RBL_COM2_DR_BASE};
const uint16_t COM_TX_BUFFER_SIZE[COMn] = {RBL_COM1_TX_SIZE, RBL_COM2_TX_SIZE};
const uint16_t COM_RX_BUFFER_SIZE[COMn] = {RBL_COM1_RX_SIZE, RBL_COM2_RX_SIZE};

u8 g_UartTxBuffer[RBL_COM1_TX_SIZE];
u8 g_UartRxBuffer[RBL_COM1_RX_SIZE];
u8 g_DebugRxBuffer[RBL_COM2_RX_SIZE];

ST_UART_FLAG g_UartTxFlag;
ST_UART_FLAG g_UartRxFlag;
ST_UART_FLAG g_DebugRxFlag;

ST_PRINT_CTRL g_Print;
ST_EVENT_CONTROL PrintEvent = {0xFF, 0};
ST_EVENT_CONTROL DebugRxEvent = {0xFF, 0};
ST_EVENT_CONTROL UartTxEvent = {0xFF, 0};
ST_EVENT_CONTROL UartRxEvent = {0xFF, 0};

ST_EVENT_METHOD UartEvents[] =
{
  {&PrintEvent, NULL},
  {&DebugRxEvent, NULL},
  {&UartTxEvent, NULL},
  {&UartRxEvent, NULL},
};

extern const Manufacturer_Version phyVersion;
//extern const u16 CRC16_CCITT_Table[256];
extern const u16 CRC16_Table[256];
ST_UPDATE st_update;
static u8 packet_nnmber = 0;
bool proceess_packet(ST_update_packet_info * current_ptr, ST_UPDATE * flash_ptr);
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handle uart events in the main loop.
  * @param  pEvents: Pointer to all uart events.
  * @retval None.
  */
void hal_RunUartEvents(ST_EVENT_METHOD *pEvents)
{
  u8 eventNum = 0;
  u8 eventCnt = 0;
  
  eventNum = sizeof(UartEvents) / sizeof(ST_EVENT_METHOD);

  for (eventCnt = 0; eventCnt < eventNum; eventCnt++)
  {
    if (pEvents[eventCnt].control->startoption == WAIT)
    {
      if (pEvents[eventCnt].control->timeToExecute <= GetSysTime())
      {
        /* Handle timeout events*/
        pEvents[eventCnt].control->startoption = END;
        switch (eventCnt)
        {
          case 0://PrintEvent
          {
            hal_InitCOM(DEBUG_COM);
            PrintEvent.timeToExecute = GetSysTime() + PRINT_TOTAL_TIMEOUT;
            hal_UartDMATx(COM2, PRINT_TIMEOUT_ERROR, StrLen(PRINT_TIMEOUT_ERROR));
            break;
          }
          case 2://UartTxEvent
          {
            hal_InitCOM(COM1);
            //hal_BlindLED(TXD_LED);
            PrintEvent.timeToExecute = GetSysTime() + PRINT_TOTAL_TIMEOUT;
            hal_UartDMATx(COM2, UART_TXD_TIMEOUT_ERROR, StrLen(UART_TXD_TIMEOUT_ERROR));
            break;
          }
          case 3://UartRxEvent
          {
            LED_Off(RX_LED);
      
            g_UartRxFlag.index = 0;
            
            UartRxEvent.startoption = END;
            break;
          }
          default:
          {
            break;
          }
        }
     }
   }
   else if (pEvents[eventCnt].control->startoption == FINISH)
   {
      pEvents[eventCnt].control->startoption = END;
      switch (eventCnt)
      {
        case 1://DebugRxEvent
        {
          Cmd_Proc();
          break;
        }
        case 3://UartRxEvent
        {
          apl_ProcessUartCmd();
          break;
        }
        default:
        {
          break;
        }
      }
   }
 }//end of for
}//end of function

/**
  * @brief  Initialize UART global variable
  * @param  None.
  * @retval None.
  */
void hal_InitUartVariable(void)
{
  hal_InitPrintVariable();

  PrintEvent.startoption = END;
  DebugRxEvent.startoption = END;
  UartTxEvent.startoption = END;
  UartRxEvent.startoption = END;
}

/**
  * @brief  Initialize Print global variable
  * @param  None.
  * @retval None.
  */
void hal_InitPrintVariable(void)
{
  u8 err = 0;

  OS_MemClr((u8 *)g_Print.buf, sizeof(g_Print.buf));
  OS_MemClr((u8 *) g_Print.block, sizeof(g_Print.block));
  g_Print.head = 0;
  g_Print.tail = 0;
  do
  {
    g_Print.part = OSMemCreate(g_Print.buf, PRINT_BLOCK_NUMBER, PRINT_BLOCK_SIZE, &err);
    if (err != OS_ERR_NONE)
    {
      hal_InitMemoryVariable();
    }
  }while (err != OS_ERR_NONE);
}

/**
  * @brief  Initialize All UART
  * @param  None.
  * @retval None.
  */
void UART_Init(void)
{
  hal_InitUartVariable();
  hal_InitCOM(COM1);
  hal_InitCOM(COM2);
}

PROCESS(hal_urat_process, "uart_process");
PROCESS_THREAD(hal_urat_process, ev, data)
{
  PROCESS_BEGIN();
  
  while(1)
  {
    process_poll(&hal_urat_process);
    hal_RunUartEvents(UartEvents);
    PROCESS_WAIT_EVENT(); 
  }
  
  PROCESS_END();
}

/**
  * @brief  Initialize COM port, include RCC, GPIO, Uart and DMA, exclude NVIC.
  * @param  COM: Specifies the COM port to be configured.
  *   This parameter can be one of following parameters:
  *     @arg COM1
  *     @arg COM2
  * @retval None.
  */
void hal_InitCOM(COM_TypeDef COM)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);

#if SWAP_UART
    /* Enable UART clock */
    if (COM == COM1)
    {
      RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
    }
    else
    {
      RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
    }
#else
    /* Enable UART clock */
    if (COM == COM1)
    {
      RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
    }
    else
    {
      RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
    }
#endif
  
  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);
  
  /* Configure USART Rx as input pull-up */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

  /* COM1 configured as follow:
        - BaudRate = 9600 baud
        - Word Length = 9 Bits
        - One Stop Bit
        - Even parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
        
    COM2 configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 9 Bits
        - One Stop Bit
        - Even parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = COM_BAUD[COM];
  USART_InitStructure.USART_WordLength = USART_WordLength_9b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_Even;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* USART configuration */
  USART_Init(COM_USART[COM], &USART_InitStructure);
  
  /* Enable USART */
  USART_Cmd(COM_USART[COM], ENABLE);
  
  /* Enable DMA Clock */
  RCC_AHBPeriphClockCmd(COM_DMA_CLK[COM], ENABLE);

  USART_ClearFlag(COM_USART[COM], USART_FLAG_RXNE);
  
  /* Enable the USART RXNE Interrupt */
  USART_ITConfig(COM_USART[COM], USART_IT_RXNE, ENABLE);
    
  /* Enable USART DMA TX request */
  USART_DMACmd(COM_USART[COM], USART_DMAReq_Tx, ENABLE);
}

/**
  * @brief  Retargets the C library printf function to the DEBUG_COM.
  * @param  None.
  * @retval None.
  */
int printf(const char *format ,... )
{
  va_list arg;
  u16 receNum = 0;
  u16 receData = 0;
  u8 blockNum = 0;
  u8 blockCnt = 0;
  u8 buffer[PRINT_BUFFER_SIZE];
  u8 *pbuf = buffer;
  u8 err = 0;
  u32 time = 0;
  OS_MEM_DATA memData;
  
  va_start(arg, format);
  receNum = (u16)vsnprintf((char *)(buffer), sizeof(buffer), format, arg);
  va_end(arg);

  receData = receNum;
  if ((receNum > 0) && (receNum <= sizeof(buffer)))
  {
    if (receNum % PRINT_BLOCK_DATA)
    {
      blockNum = receNum / PRINT_BLOCK_DATA + 1;
    }
    else
    {
      blockNum = receNum / PRINT_BLOCK_DATA;
    }

    blockCnt = 0;
    time = GetSysTime();
    while ((blockCnt < blockNum) && (GetSysTime() - time < PRINT_BLOCK_TIMEOUT))
    {
      FEED_WDG;
      err = OSMemQuery(g_Print.part, &memData);
      if ((err == OS_ERR_NONE) && (memData.OSNFree > 0))
      {
        g_Print.block[g_Print.head].pblk = (u8 *)OSMemGet(g_Print.part, &err) + sizeof(void *);
        if (err == OS_ERR_NONE)
        {
          if (receNum > PRINT_BLOCK_DATA)
          {
            MemCpy(g_Print.block[g_Print.head].pblk, pbuf, PRINT_BLOCK_DATA);
            g_Print.block[g_Print.head].len = PRINT_BLOCK_DATA;
            receNum -= PRINT_BLOCK_DATA;
            pbuf += PRINT_BLOCK_DATA;
          }
          else
          {
            MemCpy(g_Print.block[g_Print.head].pblk, pbuf, receNum);
            g_Print.block[g_Print.head].len = receNum;
          }
          
          time = GetSysTime();
          blockCnt++;
          g_Print.head++;
          if (g_Print.head >= PRINT_BLOCK_NUMBER)
          {
            g_Print.head = 0;
          }
        }
      }
    }
    
    if (PrintEvent.startoption != WAIT)
    {
      PrintEvent.startoption = WAIT;
      PrintEvent.timeToExecute = GetSysTime() + PRINT_TOTAL_TIMEOUT;
      hal_UartDMATx(COM2, g_Print.block[g_Print.tail].pblk, g_Print.block[g_Print.tail].len);
    }
  }

  return receData;
}

/**
  * @brief  Start Uart send data in DMA mode.
  * @param  pBuf: Send data pointer.
  * @param  length: Send data length.
  * @retval None.
  */
void hal_UartDMATx(COM_TypeDef COM, u8 *pBuf, u16 length)
{
  DMA_InitTypeDef DMA_InitStructure;
  
  if (length > 0)
  {
    if(length > COM_TX_BUFFER_SIZE[COM])
    {
      length = COM_TX_BUFFER_SIZE[COM];
    }
    
    /* USART_Tx_DMA_Channel (triggered by USART Tx event) Config */
    DMA_DeInit(COM_TX_DMA_CHAN[COM]);
    DMA_InitStructure.DMA_PeripheralBaseAddr = COM_DR_BASE[COM];
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32) pBuf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = length;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(COM_TX_DMA_CHAN[COM], &DMA_InitStructure);
    
    /* Enable DMA Channel Transfer Complete interrupt interrupt */
    DMA_ITConfig(COM_TX_DMA_CHAN[COM], DMA_IT_TC | DMA_IT_TE, ENABLE);
  
    /* Enable USART DMA TX Channel */
    DMA_Cmd(COM_TX_DMA_CHAN[COM], ENABLE);

    if (COM == COM1)
    {
      UartTxEvent.startoption = WAIT;
      UartTxEvent.timeToExecute = GetSysTime() + (u16)(UART_BAUD_COFF * length) + 100;
    }
    else
    {
      PrintEvent.startoption = WAIT;
      PrintEvent.timeToExecute = GetSysTime() + (u16)(PRINT_BAUD_COFF * length) + 100;
    }
  }
}


u8 GetChecksum(u8 *pbuffer, u16 length)
{
  u8 sum = 0;

  while (length--)
  {
    sum += *pbuffer++;
  }
  return sum;
}

/**
  * @brief  Process uart command.
  * @param  None.
  * @retval  None.
  */

/*
typedef enum
{
  START_POINT              = 0,
  LEN_POINT                = 1,
  CTRL_CODE_POINT          = 3,
  INFO_AREA_POINT          = 4,
  SOURCE_POINT             = 10,
  DEST_POINT               = 16,
  APP_FUN_CODE_POINT       = 22,
  DATA_MARK_POINT          = 23,
  COM_PROTOCOL_TYPE_POINT  = 25,
  COM_DELAY_POINT          = 26,
  SLAVE_NODE_POINT         = 27,
  FILE_LEN_POINT           = 28,
  FILE_START_POINT         = 29,
}em_3762_part_len_1;
*/

void apl_ProcessUartCmd(void)
{
  printf("usart receive ok\r\n");
  
  for (u8 i = 0; i < g_UartRxFlag.fLen; i++)
  {
    printf("%.2x ", g_UartRxBuffer[i]);
  }
  printf("\r\n");
  
  u16 packet_len = g_UartRxBuffer[LEN_POINT] + g_UartRxBuffer[LEN_POINT+1]*256;
  
  
  if (g_UartRxBuffer[CTRL_CODE_POINT] & 0x80 != 0x80) //上行帧，错误
  {
    return;
  }
  
  if ( (g_UartRxBuffer[INFO_AREA_POINT] & 0x04) == 0x04) //有通信模块标识,有地址域 
  {
      if (g_UartRxBuffer[APP_FUN_CODE_POINT] == 0x13)//AFN13
      {
          if ((g_UartRxBuffer[DATA_MARK_POINT] == 0x01) && (g_UartRxBuffer[DATA_MARK_POINT + 1] == 0x00) // F1: 01 00 
              && (g_UartRxBuffer[COM_PROTOCOL_TYPE_POINT] == 0x02)) //通信类型协议 645
          {
              MemCpy(g_UartTxBuffer, &g_UartRxBuffer[FILE_START_POINT], g_UartRxBuffer[FILE_LEN_POINT]);
              MemCpy(g_UartTxBuffer + g_UartRxBuffer[FILE_LEN_POINT],&g_UartRxBuffer[INFO_AREA_POINT], 18);
              printf("start RF send\r\n");
              
              for (u16 i = 0; i < g_UartRxBuffer[FILE_LEN_POINT] + 18; i++)
              {
                  printf("%.2x ", g_UartTxBuffer[i]);
              }
              printf("\r\n");
              
              SX1276LoRa_Send_Packet(g_UartTxBuffer, g_UartRxBuffer[FILE_LEN_POINT] + 18);
          }
          else if ((g_UartRxBuffer[DATA_MARK_POINT] == 0x08) && (g_UartRxBuffer[DATA_MARK_POINT + 1] == 0x02) // F1: 01 00 
              && (g_UartRxBuffer[COM_PROTOCOL_TYPE_POINT] == 0x03)) //通信类型协议 保留
          {
              g_UartTxBuffer[0]  = 0x68;
              g_UartTxBuffer[1]  = 0x99;
              g_UartTxBuffer[2]  = 0x99;
              g_UartTxBuffer[3]  = 0x99;
              g_UartTxBuffer[4]  = 0x99;
              g_UartTxBuffer[5]  = 0x99;
              g_UartTxBuffer[6]  = 0x99;
              g_UartTxBuffer[7]  = 0x68;
              g_UartTxBuffer[8]  = 0x14;//c
              g_UartTxBuffer[9]  = 0x05;//l
              g_UartTxBuffer[10] = 0x01;
              g_UartTxBuffer[11] = 0x00;
              g_UartTxBuffer[12] = 0x00;
              g_UartTxBuffer[13] = 0x69;
              g_UartTxBuffer[14] = g_UartRxBuffer[FILE_START_POINT + 2];
              for (u8 i = 0; i < 5; i++)
              {
                g_UartTxBuffer[10 + i] += 0x33;
              }
              g_UartTxBuffer[15] = GetChecksum(g_UartTxBuffer, 15);
              g_UartTxBuffer[16] = 0x16;
              SX1276LoRa_Send_Packet(g_UartTxBuffer, g_UartTxBuffer[9] + 12);
              
              printf(" send to  RF \r\n");
              for (u16 i = 0; i < g_UartTxBuffer[9] + 12; i++)
              {
                  printf("%.2x ", g_UartTxBuffer[i]);
              }
              printf("\r\n");
          }
      }
      else  if (g_UartRxBuffer[APP_FUN_CODE_POINT] == 0x03)//AFN3
      {
        if ((g_UartRxBuffer[DATA_MARK_POINT] == 0x01) && (g_UartRxBuffer[DATA_MARK_POINT + 1] == 0x00)) // F1: 01 00 
        {
          //读从节点版本号
          g_UartTxBuffer[0] = 0x68;
          MemCpy(&g_UartTxBuffer[1], &g_UartRxBuffer[DEST_POINT], 6);
          g_UartTxBuffer[7]  = 0x68;
          g_UartTxBuffer[8]  = 0x11;
          g_UartTxBuffer[9]  = 0x04;
          g_UartTxBuffer[10] = 0x02;             
          g_UartTxBuffer[11] = 0x00;
          g_UartTxBuffer[12] = 0x00;
          g_UartTxBuffer[13] = 0x6A;
          for (u8 i = 0; i < 4; i++)
          {
            g_UartTxBuffer[10 + i] += 0x33;
          }
          g_UartTxBuffer[14] = GetChecksum(g_UartTxBuffer, 14);
          g_UartTxBuffer[15] = 0x16;
           MemCpy(&g_UartTxBuffer[16], &g_UartRxBuffer[INFO_AREA_POINT], 18);
          SX1276LoRa_Send_Packet(g_UartTxBuffer, 16 + 18);
          
        }
      }
     
  }
  else //没有地址域
  {
     if ( g_UartRxBuffer[INFO_AREA_POINT + 6] == 0x12 ) //AFN12 停止路由
     {
          MemCpy(g_UartTxBuffer, g_UartRxBuffer, packet_len);
          g_UartTxBuffer[INFO_AREA_POINT + 6] = 0x00; // AFN 0x00
          g_UartTxBuffer[INFO_AREA_POINT + 7] = 0x01; // dt1
          g_UartTxBuffer[INFO_AREA_POINT + 8] = 0x00; // dt2
          g_UartTxBuffer[INFO_AREA_POINT + 9] = GetChecksum(&g_UartTxBuffer[CTRL_CODE_POINT],  10);
          g_UartTxBuffer[INFO_AREA_POINT + 10] = 0x16;
          printf(" send to master station \r\n");
          
          for (u16 i = 0; i < INFO_AREA_POINT + 11; i++)
          {
              printf("%.2x ", g_UartTxBuffer[i]);
          }
          printf("\r\n");
           
          Uart_Send(COM1, g_UartTxBuffer, INFO_AREA_POINT + 11);
     }
     else if ( (g_UartRxBuffer[INFO_AREA_POINT + 6] == 0x15) && (g_UartRxBuffer[INFO_AREA_POINT + 7] == 0x01) && (g_UartRxBuffer[INFO_AREA_POINT + 8] ==0x00) )  //AFN15 文件升级功能
     {
         //升级
         //这里要求输入的指针为数据单元的开始
         packet_nnmber = g_UartRxBuffer[INFO_AREA_POINT + 5];
         process_post(&apl_update_process, PROCESS_EVENT_MSG, &g_UartRxBuffer[13]);
     
     }
     else if ( (g_UartRxBuffer[INFO_AREA_POINT + 6] == 0x03)  && (g_UartRxBuffer[INFO_AREA_POINT + 7] == 0x01) && (g_UartRxBuffer[INFO_AREA_POINT + 8] ==0x00) ) 
     {
       //读集中器版本号
       #if 0
        g_UartTxBuffer[0]  = 0x68;
        g_UartTxBuffer[1]  = 24;
        g_UartTxBuffer[2]  = 0;   //L
        g_UartTxBuffer[3]  = 0x81; //C
        g_UartTxBuffer[4]  = g_UartRxBuffer[INFO_AREA_POINT];
        g_UartTxBuffer[5]  = g_UartRxBuffer[INFO_AREA_POINT+1];
        g_UartTxBuffer[6]  = g_UartRxBuffer[INFO_AREA_POINT+2];
        g_UartTxBuffer[7]  = g_UartRxBuffer[INFO_AREA_POINT+3];
        g_UartTxBuffer[8]  = g_UartRxBuffer[INFO_AREA_POINT+4];
        g_UartTxBuffer[9]  = g_UartRxBuffer[INFO_AREA_POINT+5];
        g_UartTxBuffer[10] = 0X03;
        g_UartTxBuffer[11] = 0x01;
        g_UartTxBuffer[12] = 0x00;
        
        g_UartTxBuffer[13] = Ver.VenderID[0];
        g_UartTxBuffer[14] = Ver.VenderID[1];
        g_UartTxBuffer[15] = 0;
        g_UartTxBuffer[16] = 0;
        g_UartTxBuffer[17] = Ver.Day;
        g_UartTxBuffer[18] = Ver.Mon;
        g_UartTxBuffer[19] = Ver.Year;
        g_UartTxBuffer[20] = 0;
        g_UartTxBuffer[21] = Ver.VerIDofSoft;
        
        g_UartTxBuffer[22] = GetChecksum(&g_UartTxBuffer[CTRL_CODE_POINT], 19);
        g_UartTxBuffer[23] = 0x16;
       #endif
     }
  }
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles COM1 DMA TX interrupt.
  * @param  None.
  * @retval None.
  */
void RBL_COM1_TX_IRQHandler(void)
{
  if (DMA_GetITStatus(RBL_COM1_TX_DMA_COMPLETE) != RESET)
  {
    /* Clear Interrupt Flag */
    DMA_ClearITPendingBit(RBL_COM1_TX_DMA_COMPLETE);

    UartTxEvent.startoption = END;
    
    LED_Off(TX_LED);
  }

  if (DMA_GetITStatus(RBL_COM1_TX_DMA_ERROR) != RESET)
  {
    DMA_ClearITPendingBit(RBL_COM1_TX_DMA_ERROR);
  }
}

/**
  * @brief  This function handles COM1 DMA RX Idle interrupt.
  * @param  None.
  * @retval None.
  */
void RBL_COM1_RX_IRQHandler(void)
{
  if (USART_GetITStatus(RBL_COM1, USART_IT_RXNE) != RESET)
  {
    if (g_UartRxFlag.index >= RBL_COM1_RX_SIZE)
    {
      g_UartRxFlag.index = 0;
    }
    g_UartRxBuffer[g_UartRxFlag.index] = USART_ReceiveData(RBL_COM1);

    UartRxEvent.startoption = WAIT;
    UartRxEvent.timeToExecute = GetSysTime() + 200;
   
    LED_On(RX_LED);
   
   switch(g_UartRxFlag.index)
   {
    case 0:
    {
      if(g_UartRxBuffer[g_UartRxFlag.index] != 0x68)
      {
        g_UartRxFlag.index = 0;
        UartRxEvent.startoption = END;
      }
      else
      {
        g_UartRxFlag.index++;
      }
    }
    break;
    
    
    case 3:
    {
       g_UartRxFlag.fLen = (u16)g_UartRxBuffer[1] + g_UartRxBuffer[2]*256;
       g_UartRxFlag.index++;
    }
    break;
    
   default:
   {
        g_UartRxFlag.index++;
        
        if ( g_UartRxFlag.index == g_UartRxFlag.fLen )
        {
          if ( (g_UartRxBuffer[g_UartRxFlag.index - 1] == 0x16) && (g_UartRxBuffer[g_UartRxFlag.index -2] == GetChecksum(g_UartRxBuffer +3, g_UartRxFlag.fLen-5)))
          {
               UartRxEvent.startoption = FINISH;
               g_UartRxFlag.index = 0;
          }
          else
          {
             g_UartRxFlag.index = 0;
             UartRxEvent.startoption = END;
          }
          
        }
   }
   break;
   }
     
  }        
}

/**
  * @brief  This function handles UART4 DMA TX interrupt.
  * @param  None.
  * @retval None.
  */
void RBL_COM2_TX_IRQHandler(void)
{
  u8 err = 0;

  if (DMA_GetITStatus(RBL_COM2_TX_DMA_COMPLETE) != RESET)
  {
    DMA_ClearITPendingBit(RBL_COM2_TX_DMA_COMPLETE);

    PrintEvent.startoption = END;

    err = OSMemPut(g_Print.part, g_Print.block[g_Print.tail].pblk - sizeof(void *));
    if (err == OS_ERR_NONE)
    {
      g_Print.tail++;
      if (g_Print.tail >= PRINT_BLOCK_NUMBER)
      {
        g_Print.tail = 0;
      }

      if (g_Print.tail != g_Print.head)
      {
        PrintEvent.startoption = WAIT;
        PrintEvent.timeToExecute = GetSysTime() + PRINT_TOTAL_TIMEOUT;
        hal_UartDMATx(COM2, g_Print.block[g_Print.tail].pblk, g_Print.block[g_Print.tail].len);
      }
    }
  }

  if (DMA_GetITStatus(RBL_COM2_TX_DMA_ERROR) != RESET)
  {
    DMA_ClearITPendingBit(RBL_COM2_TX_DMA_ERROR);
  }
}

/**
  * @brief  This function handles COM2 interrupt RX request.
  * @param  None.
  * @retval None.
  */
void RBL_COM2_RX_IRQHandler(void)
{
  if (USART_GetITStatus(RBL_COM2, USART_IT_RXNE) != RESET)
  {
    u8 receivedByte = USART_ReceiveData(RBL_COM2);
    
    if (g_DebugRxFlag.index >= RBL_COM2_RX_SIZE)
    {
      g_DebugRxFlag.index = 0;
    }

    if (receivedByte == '\b')//Backspace
    {
      if (g_DebugRxFlag.index > 0)
      {
        g_DebugRxFlag.index--;
      }
    }
    else
    {
      g_DebugRxBuffer[g_DebugRxFlag.index++] = receivedByte;
      //if ((receivedByte == '\n'))
      if ((receivedByte == '\r') || (receivedByte == '\n'))//Enter
      {
        DebugRxEvent.startoption = FINISH;
        g_DebugRxFlag.fLen = g_DebugRxFlag.index;
        g_DebugRxFlag.index = 0;
      }
    }
  }
}

/******************************************************************************/
/*            Debug Command Function                        */
/******************************************************************************/

/**
  * @brief Send data from COM1.
  * @param  None.
  * @retval  None.
  */
void Uart_Send(COM_TypeDef COM, u8* buff, u16 len)
{  
  hal_UartDMATx(COM, buff, len);  
}








/*****************************************************************************
 *
设置本包接收完成标识
 *
 *
 *
 *
*****************************************************************************/

void set_update_packetState(u16 packetNo)
{
  st_update.packetsState[packetNo/8] |= (1<<(packetNo%8));
}

/*****************************************************************************
 *
检测本包是否已经接收过

 *
 *
 *
 *
*****************************************************************************/

u8 check_update_packect_state(u16 packetNo)
{
  if (st_update.packetsState[packetNo/8] & (1<<(packetNo%8)) )
  {
    return 1;
  }
  else
  {
    return 0;
  }
  
}

/*****************************************************************************
 *
检测所有的包是否接收完成
 *
 *
 *
 *
*****************************************************************************/
bool check_update_state(u16 total_packets)
{
  for (u16 j = 0;  j < total_packets; j++)
  {
      if (check_update_packect_state(j) == 0)
      {
        return FALSE;
      }
  }
  return TRUE;
}



/*****************************************************************************
 *
检测升级文件CRC是否正确
 *
 *
 *
 *
 *
*****************************************************************************/
u8 update_software_check(u32 totalBytes)
{
  u16 temp;
  u16 crc16 = 0xFFFF;
  u32 count = totalBytes -2;
  u8 flash_temp;
  u32 current_flash_addr = FLASH_APPLICATION_BACK_ADDRESS;

  while (count--)
  {
    GDflash_read(current_flash_addr, &flash_temp, 1);
    current_flash_addr++;
    crc16 = (crc16 >> 8 ) ^ CRC16_Table[(crc16 ^ flash_temp) & 0xFF];
  }
  crc16 ^= 0xFFFF;
  
  printf("compute result = %.2x\r\n",crc16);

  GDflash_read(current_flash_addr, &flash_temp, 1);
  current_flash_addr++;
  temp =  (u16)flash_temp*256;  //高字节在前，低字节在后
  
  GDflash_read(current_flash_addr, &flash_temp, 1);
  current_flash_addr++;
  temp +=  flash_temp;
  
  printf("read result = %.2x\r\n",temp);
  
  if (crc16 == temp)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}


/*****************************************************************************
 *
将收到的数据按照包的方式，依次存入正确的位置，每个包的长度为64字节，除了最后一个包可能是变长
 *此处依然存在包计数从0开始
 *
 *
 *
 *
*****************************************************************************/
void FLASH_Write_update_page(u16 packetNo, u8 * Data, u8 length)
{
  u32 packet_start_addr = packetNo * UPDATE_DEFAULT_PACKET_SIZE;
  u8 temp[128];

  __disable_irq();
  GDflash_write(packet_start_addr, Data, length);
  __enable_irq();
  
  Delay_Nop(1000);
  
  GDflash_read(packet_start_addr, temp, length);
  
  for (u8 i = 0; i < length; i++)
  {
    if (Data[i] != temp[i])
    {
      printf("write flash %d error\r\n", packetNo);
      break;
    }
  }
}

/**********************************************************************************************************************************************
升级flash参数区排列：版本号1byte   总字节数(2byte)  总段数2byte  升级状态/1B(status) 升级包状态表/128byte(packets_states)   CRC校验2byte
#define UPDATE_VERSION_POS                        0   版本号
#define UPDATE_TOTAL_BYTE_POS                     2   总字节数
#define UPDATE_TOTAL_PACKETS_POS                  4   总段数               
#define UPDATE_STATE_POS                          6   升级状态
#define UPDATE_PACKET_STATES_POS                  7   升级包状态表
#define UPDATE_CRC_POS                            135 CRC
************************************************************************************************************************************************/
bool read_update_flash(ST_UPDATE *st_update_Structure)
{  
    bool retVal;
    
    u8 * update_flash_start_ptr = (u8*)FLASH_UPDATE_PARAMS_ADDRESS;
    st_update_Structure->version       = update_flash_start_ptr[UPDATE_VERSION_POS]*256 + update_flash_start_ptr[UPDATE_VERSION_POS + 1];
    st_update_Structure->totoalBytes   = update_flash_start_ptr[UPDATE_TOTAL_BYTE_POS]*256 + update_flash_start_ptr[UPDATE_TOTAL_BYTE_POS + 1];
    st_update_Structure->total_packets = update_flash_start_ptr[UPDATE_TOTAL_PACKETS_POS]*256 + update_flash_start_ptr[UPDATE_TOTAL_PACKETS_POS + 1];
    st_update_Structure->status        = update_flash_start_ptr[UPDATE_STATE_POS];
    st_update_Structure->crcValue      = update_flash_start_ptr[UPDATE_CRC_POS] *256 + update_flash_start_ptr[UPDATE_CRC_POS + 1];
    memcpy(st_update_Structure->packetsState, &update_flash_start_ptr[UPDATE_PACKET_STATES_POS], UPDATE_PACKETS_STATUS_LEN);

   retVal = ( GetCRC16((u8*)st_update_Structure, TOTAL_UPDATE_BYTE - 2) == st_update_Structure->crcValue)? \
   TRUE:FALSE;

   return retVal;
}


void write_update_flash(ST_UPDATE *st_update_Structure )
{ 
     u8 tempBuf[TOTAL_UPDATE_BYTE];
  
     tempBuf[UPDATE_VERSION_POS]           = (u8)(st_update_Structure->version/256);
     tempBuf[UPDATE_VERSION_POS + 1]       = (u8)(st_update_Structure->version%256);
     tempBuf[UPDATE_TOTAL_BYTE_POS]        = (u8)((st_update_Structure->totoalBytes >>8)&0xFF);
     tempBuf[UPDATE_TOTAL_BYTE_POS + 1]    = (u8)(st_update_Structure->totoalBytes & 0xFF);
     tempBuf[UPDATE_TOTAL_PACKETS_POS]     = (u8)((st_update_Structure->total_packets >>8)&0xFF);
     tempBuf[UPDATE_TOTAL_PACKETS_POS + 1] = (u8)(st_update_Structure->total_packets & 0xFF);
     tempBuf[UPDATE_STATE_POS]             =  st_update_Structure->status;
     memcpy(&tempBuf[UPDATE_PACKET_STATES_POS], st_update_Structure->packetsState, UPDATE_PACKETS_STATUS_LEN);
     
     st_update_Structure->crcValue = GetCRC16(tempBuf, TOTAL_UPDATE_BYTE - 2);
     tempBuf[UPDATE_CRC_POS]               =  (u8)((st_update_Structure->crcValue >>8)&0xFF);
     tempBuf[UPDATE_CRC_POS + 1]           =  (u8)(st_update_Structure->crcValue & 0xFF);

    if (STM32_FlashPageErase(FLASH_UPDATE_PARAMS_ADDRESS) == FLH_SUCCESS)
    {
       STM32_FlashWrite( FLASH_UPDATE_PARAMS_ADDRESS,  tempBuf, TOTAL_UPDATE_BYTE);
    }  
}

/*****************************************************************************
 *

 *
 *
 *
 *
 *
*****************************************************************************/
void reset_update_params(void)
{
    memset(&st_update, 0 , sizeof(st_update));
    write_update_flash(&st_update);   
}


void write_finish_debug(void)
{
   ST_UPDATE tempupdate;
   
   tempupdate.version = 5;
   tempupdate.current_packet_No = 287;
   tempupdate.totoalBytes  = 53;
   tempupdate.total_packets = 289;
   tempupdate.status        = UPDATE_FINISH;
   write_update_flash(&tempupdate );
}

/*****************************************************************************
 *
验证程序CRC是否正确
 *
 *
 *
 *
 *
*****************************************************************************/
void init_update(void)
{
  GDflash_init();
  read_update_flash(&st_update);

  switch (st_update.status)
  {
    case UPDATE_RUNNING:
      if (st_update.totoalBytes == 0) /*  处于升级但是没有收到数据 ,错误 */
      {
        reset_update_params();
      }
    break;

    case UPDATE_FINISH:
      //这里在以后会出现这种情况，升级数据接收成功，但是没有接收到升级指令，就不升级，复位后升级状态表示finish
      //本次程序不会出现这种情况
    break;

    case UPDATE_END:
     memset(&st_update, 0 , sizeof(st_update));
    break;
    
    case UPDATE_FAILED:
      printf("update failed\r\n");
      reset_update_params();
    break;

    case UPDATE_SUCCESS:
      printf("update successful\r\n");
      reset_update_params();
    break;

    default:
      reset_update_params();
    break; 
  }
}


/*****************************************************************************
 *
这里要求输入的指针为数据单元开始
 *
 *
 *
 *
 *
***************************************************************************/
void get_packet_info(u8 * buf, ST_update_packet_info * packet_info)
{
    packet_info->file_indication     =  buf[0];
    packet_info->file_property       =  buf[1];
    packet_info->file_instr          =  buf[2];
    packet_info->total_packets       =  buf[3] + buf[4]*256;
    packet_info->current_packet_No   =  buf[5] + buf[6]*256 + buf[7] + buf[8];
    packet_info->packet_length       =  buf[9] + buf[10]*256;
    packet_info->data                =  buf + 11;
}


//允许接收后，判断，存储
bool proceess_packet(ST_update_packet_info * current_ptr, ST_UPDATE * flash_ptr)
{
   
  u32 totoalBytes;
     //重复帧不处理,仅应答
    if (check_update_packect_state( current_ptr->current_packet_No) == 1)
    {
        printf("the same packet\r\n");
        return FALSE;
    }
    printf("write packet %d in flash\r\n", current_ptr->current_packet_No);
     //1、先将数据写入flash
    FLASH_Write_update_page(current_ptr->current_packet_No, current_ptr->data, current_ptr->packet_length);
    //2、更新内存中的已收到数据包标志
    
    if (current_ptr->current_packet_No == (current_ptr->total_packets - 1))
    {
       totoalBytes = (current_ptr->total_packets - 1)*UPDATE_DEFAULT_PACKET_SIZE + current_ptr->packet_length;
      
       flash_ptr->totoalBytes = current_ptr->packet_length;
       
        printf("total updte bytes = %d\r\n", totoalBytes);
    }
    set_update_packetState(current_ptr->current_packet_No);

    //3、检测是否接收完成
    if (check_update_state(flash_ptr->total_packets) == TRUE)
    {
        printf("all packet received\r\n");
        //4、完整读出flash验证 
        if (update_software_check(totoalBytes) == 0)
        {   //5、验证正确,升级成功
            flash_ptr->status = UPDATE_FINISH;
            //6、更新升级参数
            write_update_flash(flash_ptr);
            //测试用，到时候修改
            printf("update data OK\r\n");
            
            return TRUE;
        }
        else
        {
            //7、错误，复位升级数据
            reset_update_params();
            printf("update crc err\r\n");
        }
    }
    else
    {
        //8、升级未完成，保存升级数据
        write_update_flash(flash_ptr);
    }  
    
    return FALSE;
}



PROCESS(apl_update_process, "update_process ");
PROCESS_THREAD(apl_update_process, ev, data)
{
    static u8 * buf = NULL;
    static ST_update_packet_info st_update_packet;
    static bool update_finish_state = FALSE;
    static struct etimer update_timer; 
    //static u8 g_updateBuffer[150] = {0}; //缓存，用来存储数据
    //static bool update_buf_full = FALSE;
    
    PROCESS_BEGIN();
    while (1)
    {
        PROCESS_WAIT_EVENT(); 

        if (ev == PROCESS_EVENT_MSG)
        {   
            buf = (u8*)data;  //传进来的是数据指针，对于此来说，是从文件标识开始的                        
            get_packet_info(buf, &st_update_packet);

         
            if (st_update_packet.current_packet_No >= st_update_packet.total_packets )
            {
                printf("packet no error\r\n");
                continue; //升级包序号错误
            }

            if (st_update_packet.current_packet_No == 0)//开始升级
            {
                printf("update start\r\n");
                memset(&st_update, 0 , sizeof(st_update));
                
                GDflash_erase(0,  BLOCK_ERASE_64K);
                GDflash_erase(65536,  BLOCK_ERASE_64K);
                etimer_set(&update_timer, 1000);
                PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_TIMER) && ((struct etimer *)data == &update_timer));
                
               
                //总字节数在收到最后一帧的时候计算
                st_update.status        = UPDATE_RUNNING;
                st_update.total_packets = st_update_packet.total_packets;
                printf("update_totalPackets = %d\r\n", st_update.total_packets);
            }
            else
            {
              if (st_update.status != UPDATE_RUNNING)
              {
                  //当升级状态为finish的时候，再本程序设计中是不存在的，
                  //当为finish的时候，会自动复位，APP程序中，会把finish状态檫除
                  //但是在以后的升级中，可能会存在，在升级完成后，必须要接收到升级指令，才升级，所以程序可能存在finish的升级状态
                  reset_update_params();
                  continue;
              }
            }
            
            update_finish_state = proceess_packet(&st_update_packet, &st_update);
            
            g_UartTxBuffer[0] = 0x68;
            g_UartTxBuffer[1] = 19;
            g_UartTxBuffer[2] = 0;
            g_UartTxBuffer[3] = 0xca;
            
            g_UartTxBuffer[4] = 0x00;
            g_UartTxBuffer[5] = 0x00;
            g_UartTxBuffer[6] = 0x00;
            g_UartTxBuffer[7] = 0x00;
            g_UartTxBuffer[8] = 0x00;
            g_UartTxBuffer[9] = packet_nnmber;

            g_UartTxBuffer[10] = 0x15;
            g_UartTxBuffer[11] = 0x01;
            g_UartTxBuffer[12] = 0x00;
         
            g_UartTxBuffer[13] = (u8)(st_update_packet.current_packet_No & 0xFF);
            g_UartTxBuffer[14] = (u8)(st_update_packet.current_packet_No>>8 & 0xFF);
            g_UartTxBuffer[15] = (u8)(st_update_packet.current_packet_No>>16 & 0xFF);
            g_UartTxBuffer[16] = (u8)(st_update_packet.current_packet_No>>23 & 0xFF);
            
            g_UartTxBuffer[17] = GetChecksum(&g_UartTxBuffer[CTRL_CODE_POINT], 14);
            g_UartTxBuffer[18] = 0x16;
            
            Uart_Send(COM1, g_UartTxBuffer,  19);
            
            

            printf("ack to master update packet\r\n");
            for (u8 i = 0; i< 19; i++)
            {
                printf("%.2x ", g_UartTxBuffer[i]);
            }
            printf("\r\n");
            
            if (update_finish_state)
            {
              update_finish_state = FALSE;
              etimer_set(&update_timer, 500);
              PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_TIMER) && ((struct etimer *)data == &update_timer));
              SysReset();
            }
            else if ( (update_finish_state == FALSE) && (st_update_packet.current_packet_No == (st_update_packet.total_packets - 1)))
            {
                update_finish_state = FALSE;
                reset_update_params();
                  printf("update false \r\n");
                
            }
          }
    }
    
    PROCESS_END();
}




/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/
