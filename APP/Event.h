/**
  ******************************************************************************
  * @file    event.h 
  * @author  William Liang
  * @version V1.0.0
  * @date    09/24/2013
  * @brief   This file contains the headers of event handle.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _EVENT_H_
#define _EVENT_H_

/* Includes ------------------------------------------------------------------*/
#include "Basedefine.h"

#pragma pack(1)

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct
{
  u8 status : 3;
  u8 startoption : 3;
  u8 retrytimes : 2;
  u32 timeToExecute;
}ST_EVENT_CONTROL;

typedef struct
{
  ST_EVENT_CONTROL *control;
  void (*handler)(void);
}ST_EVENT_METHOD;

typedef enum
{
  END = 0,
  START,
  WAIT,
  FINISH,
  READ_STANDBY,
  WRITE_STANDBY,
}EN_EVENT_STARTOPTION;

typedef struct
{
  u8 venderID[2];//const 厂商代码 
  u8 chipID[2];//const 芯片代码
  u8 date;//const 日
  u8 month;//const 月
  u8 year;//const 年
  u8 version[2];//const 版本
}Manufacturer_Version;

#endif /* __EVENT_H__ */

/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/
