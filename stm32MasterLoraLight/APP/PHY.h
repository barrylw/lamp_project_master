/**
  ******************************************************************************
  * @file    phy.h 
  * @author  William Liang
  * @version V1.0.0
  * @date    09/23/2013
  * @brief   This file contains the headers of the physical layer.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _PHY_H_
#define _PHY_H_

/* Includes ------------------------------------------------------------------*/
#include "Comm.h"

#ifdef __PHYDEBUG
#define PHYDBG(CODE) PRINT(CODE)
#else
#define PHYDBG(CODE)
#endif

/* Exported constants --------------------------------------------------------*/

#define RSSI_THRESHOLD                 96u
#define aMaxPHYPayloadSize             252u
#define PHYLAYER_STANDARD_ID           1u
#define CSMA_CCA_TIMEOUT               2u //ms
#define TDMA_CCA_TIMEOUT               8u //ms

/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum
{
  HIGHEST_LEVEL = 0, //16dBm
  HIGH_LEVEL = 1,    //10dBm
  LOW_LEVEL = 2,     //4dBm
  LOWEST_LEVEL = 3,  //-2dBm
}TXPOWER_LEVEL;

typedef enum
{
  PHY_BUSY = 0,
  PHY_IDLE,
  PHY_BUSY_RX,
  PHY_BUSY_TX,
  PHY_RX_ON,
  PHY_TX_ON,
  PHY_TRX_OFF,
  PHY_FORCE_TRX_OFF,
  PHY_SUCCESS,
  PHY_INVALID_PARAMETER,
  PHY_UNSUPPORTED_ATTRIBUTE,
}PHY_STATUS;

typedef enum
{
  PHYA_TX_COMPLETE,
}PHY_ATTR;

typedef enum
{
  PHYSendSuccess,
  PHYSendFailure,
  PHYSending,
}PHY_SEND_RESULT;

typedef enum
{
  RET_PHY_SUCCESS,                //返回成功
  RET_PHY_ATTRLEN_ERR,            //属性长度错误
  RET_PHY_ATTR_NOEXIST,           //属性名不存在
}ePhyRet;

typedef enum
{
  PhyRxNoData,
  PhyRxDataReady,
}ePhyRxResult;

/* Exported functions ------------------------------------------------------- */


void Phy_Init(void);
void Phy_Proc(void);

u8 Phy_GetAttr(u8 attr,u8 attrlen,u8* val);
u8 Phy_SetAttr(u8 attr,u8 attrlen,u8* val);

u8 Phy_RxData();
u8 Phy_GetData(u16* psdulen, u8* psdu, u8* rssi);

void Phy_CCA_Request(void);
u8 Phy_Data_Request(u16 psduLen, u8 *psdu, u8 channel);

extern void Mac_CCA_Confirm(u8 status);

#endif /* __PHY_H__ */

/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/
