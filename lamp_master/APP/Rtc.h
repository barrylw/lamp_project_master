/**
  ******************************************************************************
  * @file    hal_rtc.h 
  * @author  William Liang
  * @version V1.0.0
  * @date    05/05/2014
  * @brief   This file contains the headers of the rtc handlers.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _RTC_H_
#define _RTC_H_

/* Includes ------------------------------------------------------------------*/
#include "Basedefine.h"

/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/

#define START_OF_2010 0
#define SECS_IN_DAY 86400 // 24*3600

#define RTC_PRESCALER (HSE_VALUE / 128 - 1)
#define RTC_VALID_REG BKP_DR1
#define RTC_VALID_FLAG 0xA5A5
#define GetRTCTime RTC_GetCounter


/* Exported macro ------------------------------------------------------------*/
extern u32 g_SystickCounter;

/* Exported functions ------------------------------------------------------- */
void RTC_Init(void);
sTime TimetoBCD(u32 utcTime);
void Time_Set(u8 year, u8 month, u8 day, u8 hour, u8 min, u8 sec);
void Time_Get(void);

/**
  * @brief  Get the Systick Timer Counter.
  * @param  None.
  * @retval g_SystickCounter: Systick Timer Counter Value.
  */
__STATIC_INLINE u32 GetSysTime(void)
{
  return g_SystickCounter;
}

/*——————————————————————————
* 函 数 名：GetSysTime
* 输入参数：None
* 输出参数：None
* 返 回 值：系统节拍数
* 功能说明：获取时间（单位：毫秒）
*——————————————————————————*/

#endif /* __HAL_TIMER_H__ */

/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/
