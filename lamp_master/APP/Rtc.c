/**
  ******************************************************************************
  * @file    hal_timer.c
  * @author  William Liang
  * @version V1.0.0
  * @date    07/18/2013
  * @brief   This file contains the initialization and handle of the timer.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Rtc.h"
#include "Uart.h"

/** @addtogroup Timer
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static const u8 daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
u32 g_SystickCounter = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the RTC.
  * @param  None.
  * @retval None.
  */
void RTC_Init(void)
{
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);
  
  /* Select HSE as RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);

  if (BKP_ReadBackupRegister(RTC_VALID_REG) != RTC_VALID_FLAG)
  {
    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
    
    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();
    
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    
    /* Set RTC prescaler: set RTC period to 1sec */
    RTC_SetPrescaler(RTC_PRESCALER); /* RTC period = RTCCLK/RTC_PR = (8 MHz /128)/(RTC_PRESCALER+1) */
    
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Set RTC Valid Flag in the BKP */
    BKP_WriteBackupRegister(RTC_VALID_REG, RTC_VALID_FLAG);

    /* Initialize current time to 2010/1/1 00:00:00 */
    Time_Set(10, 1, 1, 0, 0, 0);
  }
  else
  {
    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();
    
    Time_Get();
  }
}

/**
  * @brief  convert current time format from the total seconds since 2010 to the time struct.
  * @param  utcTime: input variable, total seconds since 2010.
  * @retval time: the struct of current time.
  */
sTime TimetoBCD(u32 utcTime)
{
  sTime time;
  
  u32 secondsSince2010 = (utcTime - START_OF_2010); 
  u16 daysSince2010 = (secondsSince2010 >> 2) / (u16)(SECS_IN_DAY >> 2);
  u32 secondInDay = secondsSince2010 - ((u32)daysSince2010) * SECS_IN_DAY;
  u8 yearsSince2000 = 0;
  u16 daysRemaining = daysSince2010;
  u16 accum = 10;//We start from 2010
  u8 phase;

  for (phase = 0; phase < 2; phase++)
  {
    while (1)
    {
      u16 tick;
      
      if (phase == 0)
      {
        tick = ((accum & 0x03) == 0 ? 366 : 365);
      }
      else
      {
        tick = daysInMonth[accum];
        if ((accum == 1) && ((yearsSince2000 & 0x03) == 0))
        {
          tick++;
        }
      }
      
      if (tick <= daysRemaining)
      {
        daysRemaining -= tick;
        accum++;
      }
      else
      {
        if (phase == 0)
        {
          yearsSince2000 = accum;
          accum = 0;
        }
        break;
      }
    }
  }
  
  time.year = yearsSince2000;
  time.month = accum + 1;
  time.day = daysRemaining + 1;
  time.hour = secondInDay / 3600;
  time.minute = (secondInDay - time.hour * 3600) / 60;
  time.second = (secondInDay - time.hour * 3600) % 60;
  return time;
}

/**
  * @brief  Set the current time.
  * @param  year: input variable, the year of current time.
  * @param  month: input variable, the month of current time.
  * @param  day: input variable, the day of current time.
  * @param  hour: input variable, the hour of current time.
  * @param  min: input variable, the minute of current time.
  * @param  sec: input variable, the second of current time.
  * @retval None.
  */
void Time_Set(u8 year, u8 month, u8 day, u8 hour, u8 min, u8 sec)
{
  u32 secs = START_OF_2010 + (u32) hour * 3600 + (u32) min * 60 + (u32) sec;
  u16 days = day - 1;
  u8 i;
 
  printf("设置系统时间 %02d-%02d-%02d %02d:%02d:%02d\r\n", year, month, day, hour, min, sec);
  
  for (i = 10; i < year; i++)
  {
    days += 365;
    if ((i & 0x03) == 0)
    {
      days++;
    }
  }

  for (i = 0; i < month - 1; i++)
  {
    days += daysInMonth[i];
  }
  
  if ((month > 2) && ((year & 0x03) == 0))
  {
    days++;
  }

  secs += ((u32) days) * SECS_IN_DAY;
  
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  /* Change the current time */
  RTC_SetCounter(secs);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}

/**
  * @brief  Print the current time.
  * @param  None.
  * @retval None.
  */
void Time_Get(void)
{
  sTime CurrentTime;

  CurrentTime = TimetoBCD(RTC_GetCounter());
  
  printf("获取系统时间 %02d-%02d-%02d %02d:%02d:%02d %d %d\r\n",
              CurrentTime.year,
              CurrentTime.month,
              CurrentTime.day,
              CurrentTime.hour,
              CurrentTime.minute,
              CurrentTime.second,
              RTC_GetCounter(),
              GetSysTime());
}

/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/
