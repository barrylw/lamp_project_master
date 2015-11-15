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
#include "sx1276-LoRa.h"
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
            g_UartRxFlag.index = 0;
            hal_InitCOM(COM1);
            //hal_BlindLED(RXD_LED);
            PrintEvent.timeToExecute = GetSysTime() + PRINT_TOTAL_TIMEOUT;
            hal_UartDMATx(COM2, UART_RXD_TIMEOUT_ERROR, StrLen(UART_RXD_TIMEOUT_ERROR));
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
  }
}

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
void apl_ProcessUartCmd(void)
{
  DISP(printf("HAL:Uart Receive Packet:\r\n"););

  #if 0
  PrintBuff(g_UartRxBuffer,g_UartRxFlag.fLen);

  MCP_Process(g_UartRxBuffer,g_UartRxFlag.fLen);
  #endif

  if (GetChecksum(g_UartRxBuffer, g_UartRxFlag.fLen - 2) == g_UartRxBuffer[g_UartRxFlag.fLen - 2])
  {
    SX1276LoRa_Send_Packet(g_UartRxBuffer,g_UartRxFlag.fLen);
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
    UartRxEvent.timeToExecute = GetSysTime() + (u32)UART_BAUD_COFF + 100;


    
    #if 0
    switch (g_UartRxFlag.index)
    {
      case 0:
    {
      if(g_UartRxBuffer[g_UartRxFlag.index] != MCP_FSTART_1)
      {
        g_UartRxFlag.index = 0;
      }
      else
      {
        g_UartRxFlag.index++;
      }
    }
        break;
      case 1:
      {
        if((g_UartRxBuffer[g_UartRxFlag.index] == MCP_FSTART_2) && (g_UartRxBuffer[0] == MCP_FSTART_1))
        {
            g_UartRxFlag.index++;
        }
        else
        {
      g_UartRxFlag.index = 0;          
        }
        break;
      }      
      case 2:
      case 3:
      case 4:      
      {
        g_UartRxFlag.index++;
        break;
      }
      default:
      {
        if (g_UartRxFlag.index >= g_UartRxBuffer[4] + 7)
        {
          if (g_UartRxBuffer[g_UartRxFlag.index] == MCP_FEND)
          {
            UartRxEvent.startoption = FINISH;
            g_UartRxFlag.fLen = g_UartRxFlag.index + 1;
          }
          g_UartRxFlag.index = 0;
        }
        else
        {
          g_UartRxFlag.index++;
        }
        break;
      }
    }
    #endif

    switch (g_UartRxFlag.index)
    {
      case 0:
      case 7:
      {
        if(g_UartRxBuffer[g_UartRxFlag.index] != 0x68)
        {
          g_UartRxFlag.index = 0;
        }
        else
        {
          g_UartRxFlag.index++;
        }
        break;
      }
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 8:
      case 9:
      {
        g_UartRxFlag.index++;
        break;
      }
      default:
      {
        if (g_UartRxFlag.index >= g_UartRxBuffer[9] + 11)
        {
          if (g_UartRxBuffer[g_UartRxFlag.index] == 0x16)
          {
            UartRxEvent.startoption = FINISH;
            g_UartRxFlag.fLen = g_UartRxFlag.index + 1;
          }
          g_UartRxFlag.index = 0;
        }
        else
        {
          g_UartRxFlag.index++;
        }
        break;
      }
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

/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/
