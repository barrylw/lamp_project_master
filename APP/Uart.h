
/**
  ******************************************************************************
  * @file    hal_uart.h 
  * @author  William Liang
  * @version V1.0.0
  * @date    07/22/2013
  * @brief   This file contains the headers of the uart handlers.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _UART_H_
#define _UART_H_

/* Includes ------------------------------------------------------------------*/

#include "Basedefine.h"
#include "Event.h"
#include "Memory.h"
#include "process.h"
#include "etimer.h"
/** @addtogroup Low Level Uart Driver
  * @{
  */
/* Exported constants --------------------------------------------------------*/
#define COMn           2

#define DEBUG_COM    COM2

#define UART_BUFFER_SIZE 256
#define DEBUG_BUFFER_SIZE 30
#define PRINT_BUFFER_SIZE 200
#define PRINT_BLOCK_NUMBER 250
#define PRINT_BLOCK_SIZE 16
#define PRINT_BLOCK_DATA (PRINT_BLOCK_SIZE - sizeof(void *))
#define PRINT_TOTAL_TIMEOUT ((PRINT_BLOCK_NUMBER * PRINT_BLOCK_DATA * 11000) / RBL_COM2_BAUD) + 100 //ms
#define PRINT_BLOCK_TIMEOUT ((PRINT_BLOCK_DATA * 11000) / RBL_COM2_BAUD) + 100 //ms
#define PRINT_TIMEOUT_ERROR ("print timeout error!\r\n")
#define PRINT_LENGTH_ERROR ("print length error!\r\n")
#define UART_TXD_TIMEOUT_ERROR ("uart send timeout!\r\n")
#define UART_RXD_TIMEOUT_ERROR ("uart receive timeout!\r\n")
#define UART_BAUD_COFF     ((11.0 * 1000.0) / RBL_COM1_BAUD)
#define PRINT_BAUD_COFF    ((11.0 * 1000.0) / RBL_COM2_BAUD)

#define SWAP_UART      1u
#if SWAP_UART
/**
 * @brief Definition for COM1, connected to UART4
 */ 
#define RBL_COM1                         UART4
#define RBL_COM1_BAUD                    9600
#define RBL_COM1_CLK                     RCC_APB1Periph_UART4
#define RBL_COM1_TX_PIN                  GPIO_Pin_10
#define RBL_COM1_TX_GPIO_PORT            GPIOC
#define RBL_COM1_TX_GPIO_CLK             RCC_APB2Periph_GPIOC
#define RBL_COM1_RX_PIN                  GPIO_Pin_11
#define RBL_COM1_RX_GPIO_PORT            GPIOC
#define RBL_COM1_RX_GPIO_CLK             RCC_APB2Periph_GPIOC
#define RBL_COM1_IRQn                    UART4_IRQn
#define RBL_COM1_DMA_CLK                 RCC_AHBPeriph_DMA2
#define RBL_COM1_TX_DMA_CHANNEL          DMA2_Channel5
#define RBL_COM1_RX_DMA_CHANNEL          DMA2_Channel3
#define RBL_COM1_TX_DMA_FLAG             DMA2_FLAG_GL5 | DMA2_FLAG_TC5 | DMA2_FLAG_TE5 | DMA2_FLAG_HT5
#define RBL_COM1_RX_DMA_FLAG             DMA2_FLAG_GL3 | DMA2_FLAG_TC3 | DMA2_FLAG_TE3 | DMA2_FLAG_HT3
#define RBL_COM1_DR_BASE                 (UART4_BASE + 4)
#define RBL_COM1_TX_IRQHandler           DMA2_Channel4_5_IRQHandler
#define RBL_COM1_RX_IRQHandler           UART4_IRQHandler
#define RBL_COM1_TX_DMA_COMPLETE         DMA2_IT_TC5
#define RBL_COM1_TX_DMA_ERROR            DMA2_IT_TE5

/**
 * @brief Definition for COM2, connected to USART2
 */ 
#define RBL_COM2                         USART2
#define RBL_COM2_BAUD                    115200
#define RBL_COM2_CLK                     RCC_APB1Periph_USART2
#define RBL_COM2_TX_PIN                  GPIO_Pin_2
#define RBL_COM2_TX_GPIO_PORT            GPIOA
#define RBL_COM2_TX_GPIO_CLK             RCC_APB2Periph_GPIOA
#define RBL_COM2_RX_PIN                  GPIO_Pin_3
#define RBL_COM2_RX_GPIO_PORT            GPIOA
#define RBL_COM2_RX_GPIO_CLK             RCC_APB2Periph_GPIOA
#define RBL_COM2_IRQn                    USART2_IRQn
#define RBL_COM2_DMA_CLK                 RCC_AHBPeriph_DMA1
#define RBL_COM2_TX_DMA_CHANNEL          DMA1_Channel7
#define RBL_COM2_RX_DMA_CHANNEL          DMA1_Channel6
#define RBL_COM2_DR_BASE                 (USART2_BASE + 4)
#define RBL_COM2_TX_IRQHandler           DMA1_Channel7_IRQHandler
#define RBL_COM2_RX_IRQHandler           USART2_IRQHandler
#define RBL_COM2_TX_DMA_COMPLETE         DMA1_IT_TC7
#define RBL_COM2_TX_DMA_ERROR            DMA1_IT_TE7

#else
/**
 * @brief Definition for COM1, connected to USART1
 */ 
#define RBL_COM1                         USART1
#define RBL_COM1_BAUD                    115200
#define RBL_COM1_CLK                     RCC_APB2Periph_USART1
#define RBL_COM1_TX_PIN                  GPIO_Pin_9
#define RBL_COM1_TX_GPIO_PORT            GPIOA
#define RBL_COM1_TX_GPIO_CLK             RCC_APB2Periph_GPIOA
#define RBL_COM1_RX_PIN                  GPIO_Pin_10
#define RBL_COM1_RX_GPIO_PORT            GPIOA
#define RBL_COM1_RX_GPIO_CLK             RCC_APB2Periph_GPIOA
#define RBL_COM1_IRQn                    USART1_IRQn
#define RBL_COM1_DMA_CLK                 RCC_AHBPeriph_DMA1
#define RBL_COM1_TX_DMA_CHANNEL          DMA1_Channel4
#define RBL_COM1_TX_DMA_FLAG             DMA1_FLAG_GL4 | DMA1_FLAG_TC4 | DMA1_FLAG_TE4 | DMA1_FLAG_HT4
#define RBL_COM1_DR_BASE                 (USART1_BASE + 4)
#define RBL_COM1_TX_IRQHandler           DMA1_Channel4_IRQHandler
#define RBL_COM1_RX_IRQHandler           USART1_IRQHandler
#define RBL_COM1_TX_DMA_COMPLETE         DMA1_IT_TC4
#define RBL_COM1_TX_DMA_ERROR            DMA1_IT_TE4

/**
 * @brief Definition for COM2, connected to UART4
 */ 
#define RBL_COM2                         UART4
#define RBL_COM2_BAUD                    115200
#define RBL_COM2_CLK                     RCC_APB1Periph_UART4
#define RBL_COM2_TX_PIN                  GPIO_Pin_10
#define RBL_COM2_TX_GPIO_PORT            GPIOC
#define RBL_COM2_TX_GPIO_CLK             RCC_APB2Periph_GPIOC
#define RBL_COM2_RX_PIN                  GPIO_Pin_11
#define RBL_COM2_RX_GPIO_PORT            GPIOC
#define RBL_COM2_RX_GPIO_CLK             RCC_APB2Periph_GPIOC
#define RBL_COM2_IRQn                    UART4_IRQn
#define RBL_COM2_DMA_CLK                 RCC_AHBPeriph_DMA2
#define RBL_COM2_TX_DMA_CHANNEL          DMA2_Channel5
#define RBL_COM2_DR_BASE                 (UART4_BASE + 4)
#define RBL_COM2_TX_IRQHandler           DMA2_Channel4_5_IRQHandler
#define RBL_COM2_RX_IRQHandler           UART4_IRQHandler
#define RBL_COM2_TX_DMA_COMPLETE         DMA2_IT_TC5
#define RBL_COM2_TX_DMA_ERROR            DMA2_IT_TE5
#endif

#define RBL_COM1_TX_SIZE                 UART_BUFFER_SIZE
#define RBL_COM1_RX_SIZE                 UART_BUFFER_SIZE
#define RBL_COM1_RX_BUFFER               g_UartRxBuffer

#define RBL_COM2_TX_SIZE                 PRINT_BUFFER_SIZE
#define RBL_COM2_RX_SIZE                 DEBUG_BUFFER_SIZE
#define RBL_COM2_RX_BUFFER               g_DebugRxBuffer

/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum 
{
  COM1 = 0,
  COM2 = 1,
}COM_TypeDef;   

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

//#pragma pack(1)

typedef struct
{
  u16 index;
  u16 fLen;
}ST_UART_FLAG;

typedef enum
{ 
  BUFF_EMPTY = 0,
  BUFF_HAS_DATA,
  BUFF_FULL
}EN_BUFF_STATUS;

typedef struct
{
  u16 length;
  u8 buf[DEBUG_BUFFER_SIZE];
}ST_DEBUG_CMD;
    
typedef struct
{
  u8 *pblk;
  u8 len;
}ST_PRINT_BLOCK;

typedef struct
{
  u8 head;
  u8 tail;
  ST_PRINT_BLOCK block[PRINT_BLOCK_NUMBER];
  u8 buf[PRINT_BLOCK_NUMBER][PRINT_BLOCK_SIZE];
  OS_MEM *part;            /* Pointer to printf function of memory partitions       */
}ST_PRINT_CTRL;

typedef struct
{
  u8 start;
  u8 length[2];
  u8 ctrl_code;
  u8 info_area[6];
  u8 source_addr[6];
  u8 dest_addr[6];
  u8 app_fun_code;
  u8 data_mark[2];
  u8 com_protocol_type;
  u8 com_delay;
  u8 slave_node_count;
  u8 slave_file_len;
  u8 *file;
  u8 checksum;
  u8 end;
}st_3762_strcut;

  
  
typedef enum
{
    UPDATE_END = 0,
    UPDATE_RUNNING,
    UPDATE_FINISH,
    UPDATE_SUCCESS,
    UPDATE_FAILED,
    UPDATE_SALVE,
}EM_UPDATE_STATUS;


typedef struct
{
  u16 lastBytes;
  u16 total_packets;
  u16 current_packet_No;
  u16 crcValue;
  u16 version;
  u8  status;
  u8  packetsState[128];
}ST_UPDATE;

typedef struct
{
    u16  total_packets;
    u16  packet_length;
    u32  current_packet_No;
    u8   file_indication;
    u8   file_property;
    u8   file_instr;
    u8 * data;
}ST_update_packet_info;




#define TOTAL_UPDATE_BYTE                              137
#define UPDATE_PACKETS_STATUS_LEN                      128

#define UPDATE_VERSION_POS                             0
#define UPDATE_TOTAL_BYTE_POS                          2
#define UPDATE_TOTAL_PACKETS_POS                       4               
#define UPDATE_STATE_POS                               6
#define UPDATE_PACKET_STATES_POS                       7
#define UPDATE_CRC_POS                                 135

#define UPDATE_DEFAULT_PACKET_SIZE                     128


#define FLASH_API_ADDRESS                              0x08000000
#define FLASH_APPLICATION_ADDRESS                      0x08001800

#define FLASH_APPLICATION_BACK_ADDRESS                 0

#define FLASH_UPDATE_PARAMS_ADDRESS                    0x0803F800

/* Exported functions ------------------------------------------------------- */

void hal_RunUartEvents(ST_EVENT_METHOD *pEvents);
void hal_InitUartVariable(void);
void hal_InitPrintVariable(void);
void UART_Init(void);
void hal_InitCOM(COM_TypeDef COM);
void hal_UartDMATx(COM_TypeDef COM, u8 *pBuf, u16 length);

int printf(const char *format ,... );

void apl_ProcessUartCmd(void);

void Uart_Send(COM_TypeDef COM, u8* buff, u16 len);
void init_update(void);

PROCESS_NAME(hal_urat_process);
PROCESS_NAME(apl_update_process);

#endif /* __HAL_UART_H__ */

/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/
