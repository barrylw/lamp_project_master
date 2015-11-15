/**
  ******************************************************************************
  * @file    hal_radio.h 
  * @author  William Liang
  * @version V1.0.0
  * @date    09/10/2013
  * @brief   This file contains the headers of the radio frequency handlers.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HAL_sRF_H__
#define __HAL_sRF_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "phy.h"
#include "Basedefine.h"
#include "sys/process.h"
#include "etimer.h"

/* Exported constants --------------------------------------------------------*/

/**
  * @brief  SPI sRF Frequency Chip Interface pins
  */
#define sRF_SPI                                  SPI2
#define sRF_SPI_CLK                              RCC_APB1Periph_SPI2
#define sRF_CS_PIN                               GPIO_Pin_12                  /* PB.12 */
#define sRF_CS_GPIO_PORT                         GPIOB                       /* GPIOB */
#define sRF_CS_GPIO_CLK                          RCC_APB2Periph_GPIOA
#define sRF_SPI_SCK_PIN                          GPIO_Pin_13                  /* PB.13 */
#define sRF_SPI_SCK_GPIO_PORT                    GPIOB                       /* GPIOB */
#define sRF_SPI_SCK_GPIO_CLK                     RCC_APB2Periph_GPIOA
#define sRF_SPI_MISO_PIN                         GPIO_Pin_14                  /* PB.14 */
#define sRF_SPI_MISO_GPIO_PORT                   GPIOB                      /* GPIOB */
#define sRF_SPI_MISO_GPIO_CLK                    RCC_APB2Periph_GPIOA
#define sRF_SPI_MOSI_PIN                         GPIO_Pin_15                 /* PB.15 */
#define sRF_SPI_MOSI_GPIO_PORT                   GPIOB                       /* GPIOB */
#define sRF_SPI_MOSI_GPIO_CLK                    RCC_APB2Periph_GPIOA
   
#define sRF_SPI_DR_Base                          (SPI2_BASE + 0x0C)
#define sRF_SPI_DMA                              DMA1
#define sRF_SPI_DMA_CLK                          RCC_AHBPeriph_DMA1
#define sRF_SPI_DMA_RX_Channel                   DMA1_Channel4
#define sRF_SPI_DMA_TX_Channel                   DMA1_Channel5
#define sRF_SPI_DMA_RX_FLAG                      DMA1_FLAG_TC4
#define sRF_SPI_DMA_TX_FLAG                      DMA1_FLAG_TC5


/* BEGIN: Added by Barry, 2014/3/4 */
#define sRF_RESET_PORT                          GPIOA 
#define sRF_RESET_SCK                           RCC_APB2Periph_GPIOA
#define sRF_RESET_PIN                           GPIO_Pin_15
    

#define sRF_DIOx_PORT                           GPIOC
#define sRF_DIOx_SCK                            RCC_APB2Periph_GPIOC
#define sRF_DIO0_PIN                            GPIO_Pin_0
#define sRF_DIO1_PIN                            GPIO_Pin_1
#define sRF_DIO2_PIN                            GPIO_Pin_2
#define sRF_DIO3_PIN                            GPIO_Pin_3
#define sRF_DIO4_PIN                            GPIO_Pin_4
//#define sRF_DIO5_PIN                            GPIO_Pin_5

#define sRF_DIOx_PORT_SOURCE                    GPIO_PortSourceGPIOC
    
#define sRF_DIO0_PIN_SOURCE                     GPIO_PinSource0
#define sRF_DIO1_PIN_SOURCE                     GPIO_PinSource1
#define sRF_DIO2_PIN_SOURCE                     GPIO_PinSource2
#define sRF_DIO3_PIN_SOURCE                     GPIO_PinSource3
#define sRF_DIO4_PIN_SOURCE                     EXTI_PinSource4

#define  DIO0_IRQ                               EXTI_Line0
#define  DIO1_IRQ                               EXTI_Line1
#define  DIO2_IRQ                               EXTI_Line2
#define  DIO3_IRQ                               EXTI_Line3
#define  DIO4_IRQ                               EXTI_Line4
#define  DIOall_IRQ                             (DIO0_IRQ | DIO1_IRQ | DIO2_IRQ | DIO3_IRQ | DIO4_IRQ)
#define  hal_DIOx_ITConfig(n,NewState)          hal_sRF_ITConfig(DIO##n##_IRQ,NewState)



#define sRF_FIFO_ARRD                           0
/* END:   Added by Barry, 2014/3/4 */
  
  /* Select sRF: Chip Select pin low */
#define sRF_CS_LOW()                             GPIO_ResetBits(sRF_CS_GPIO_PORT, sRF_CS_PIN)
  /* Deselect sRF: Chip Select pin high */
#define sRF_CS_HIGH()                            GPIO_SetBits(sRF_CS_GPIO_PORT, sRF_CS_PIN)
  
  /* Disable PA */
#define sRF_PA_DISABLE()                         GPIO_ResetBits(sRF_PA_CTRL_PORT, sRF_PA_CTRL_PIN)
  /* Enable PA */
#define sRF_PA_ENABLE()                          GPIO_SetBits(sRF_PA_CTRL_PORT, sRF_PA_CTRL_PIN)


#define sRF_DUMMY_BYTE                           0xAA
#define sRF_CRYSTALCAP_EEPROM_ADDR               0xFFC1
#define sRF_CHANNEL_NUMBER                       66u
#define sRF_PREAMBLE_LENGTH                      80u //bytes
#define sRF_SYNCWORD_LENGTH                      2u //bytes
#define sRF_FREQ_HOP_TIMEOUT                     30u//ms
#define sRF_CHECK_SYNCWORD_TIMEOUT               85u //(((sRF_PREAMBLE_LENGTH + sRF_SYNCWORD_LENGTH) * 8) / 10 + 20) //ms
#define sRF_FIRST_TX_TIMEOUT                     136u //(((sRF_PREAMBLE_LENGTH + sRF_SYNCWORD_LENGTH + FIFO_SIZE) * 8) / 10 + 20) //ms
#define sRF_FIFO_TRX_TIMEOUT                     71u //((FIFO_SIZE * 8) / 10 + 20) //ms
#define sRF_FIFO_DMA_TIMEOUT                     20u //ms
#define sRF_PACKET_SIZE                          (aMaxPHYPayloadSize + 6) //258u
#define sRF_RSSI_SAMPLE_NUMBER                   9u
#define sRF_RSSI_SAMPLE_INTERVAL                 2u //ms
#define sRF_TEST_INTERVAL                        1000u //ms

#define TOTAL_REGISTER_NUMBER                    0x70
#define FIFO_SIZE                                255u

#define RF_TIMEOUT                               3000

#define RXTX( txEnable )                            SX1276WriteRxTx( txEnable )
#define TICK_RATE_MS( ms )                          ( ms )
    
#define hal_GetSystickCounter                   GetSysTime
#define true                                    TRUE
#define false                                   FALSE

/* BEGIN: Added by Barry, 2014/3/1 */

typedef enum
{
  RADIO_RESET_OFF,
  RADIO_RESET_ON,
}tRadioResetState;
/* END:   Added by Barry, 2014/3/1 */

#endif

/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/



/* Exported functions ------------------------------------------------------- */
void hal_RunRFEvents(ST_EVENT_METHOD *pEvents);
void hal_InitRFVariable(void);
//void hal_InitRF(void);
void hal_sRF_InitSPI(void);
void hal_sRF_ITConfig(uint32_t irqLine, FunctionalState NewState);
u8 hal_sRF_ReadRegister(u8 reg);
void hal_sRF_WriteRegister(u8 reg, u8 val);
void hal_sRF_DMA_Read(u8 startReg, u8 *pBuffer, u8 length);
void hal_sRF_DMA_Write(u8 *pBuffer, u8 length);
void hal_sRF_Config(u8 startReg, u8 *pBuffer, u8 length);
void hal_sRF_Read(u8 startReg, u8 *pBuffer, u8 length);
void hal_sRF_Receive(void);
void hal_sRF_Transmit(u8 *pBuffer, u8 length, u8 channel);
void hal_sRF_ReadRssi(void);
u8 hal_sRF_GetLinkQuality(void);
u8 hal_sRF_RssiToLinkQuality(u8 rssi);
void hal_sRF_FrequencyHopping(void);
void hal_sRF_WhiteningBuffer(u8 *pBuffer, u16 length);
bool hal_sRF_CheckPacketValid(u8 *pBuffer, u16 length);
void hal_sRF_Sync_Handle(void);
void hal_sRF_TRX_Handle(void);

u8 spiReadWriteByte(u8 data);
void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size );
void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size );
void SX1276Write( uint8_t addr, uint8_t data );
void SX1276Read( uint8_t addr, uint8_t *data );
void SX1276WriteFifo( uint8_t *buffer, uint8_t size );
void SX1276ReadFifo( uint8_t *buffer, uint8_t size );
void SX1276SetReset( uint8_t state );

void SX1276WriteRxTx( bool txEnable );
void hal_sRF_SPI_Config(void);
void sRFTransmitHandle(void);
void GPIO_int_Config(void);
void TimeOutHandle(void);
void hal_sRF_writeFIFO_DMA(u8 * pBuffer, u8 length);
void hal_sRF_readFIFO_DMA(u8 * pBuffer, u8 length);
void hal_sRF_Transmit(u8 *pBuffer, u8 length, u8 channel);
void hal_sRF_FSK_ITConfig( uint32_t irqLine, FunctionalState NewState);
#define hal_fsk_eit_failing(n,NewState)         hal_sRF_FSK_ITConfig(DIO##n##_IRQ,NewState)
/*!
 * DIO state read functions mapping
 */
#define DIO(n)                            GPIO_ReadInputDataBit( sRF_DIOx_PORT, sRF_DIO##n##_PIN )
 

#define DIO0          DIO(0)                              
#define DIO1          DIO(1)                             
#define DIO2          DIO(2)                              
#define DIO3          DIO(3)                              
#define DIO4          DIO(4)                              
#define DIO5          DIO(5)        
#define DEBUG

PROCESS_NAME(hal_RF_process);
PROCESS_NAME(hal_long_send);

/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/
