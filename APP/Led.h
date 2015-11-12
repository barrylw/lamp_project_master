/*——————————————————————————
* 文 件 名：Led.h  
* 文件说明：Led头文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2013-12-30
*———————————————————————————*/
#ifndef _LED_H
#define _LED_H

#include "Basedefine.h"

/*************** 宏定义 ***************/
#define LEDn                             3

//串口接收灯
#define RXLED_PIN                         GPIO_Pin_12
#define RXLED_GPIO_PORT                   GPIOC
#define RXLED_GPIO_CLK                    RCC_APB2Periph_GPIOC 

//串口发送灯
#define TXLED_PIN                         GPIO_Pin_2
#define TXLED_GPIO_PORT                   GPIOD
#define TXLED_GPIO_CLK                    RCC_APB2Periph_GPIOD  

//网络灯
#define NETLED_PIN                        GPIO_Pin_3
#define NETLED_GPIO_PORT                  GPIOB
#define NETLED_GPIO_CLK                   RCC_APB2Periph_GPIOB


/*************** 结构定义 ***************/
typedef enum 
{
  RX_LED = 0,        //串口接收灯
  TX_LED = 1,        //串口发送灯
  NET_LED = 2,      //网络灯  
}eLed;

/*************** 函数实现 ***************/

/*——————————————————————————
* 函 数 名：LED_Init
* 输入参数：led    灯
* 输出参数：None
* 返 回 值：None
* 功能说明：初始化LED灯
*——————————————————————————*/
void LED_Init(eLed led);

/*——————————————————————————
* 函 数 名：LED_On
* 输入参数：led    灯
* 输出参数：None
* 返 回 值：None
* 功能说明：点亮Led灯
*——————————————————————————*/
void LED_On (eLed led);

/*——————————————————————————
* 函 数 名：LED_Off
* 输入参数：led    灯
* 输出参数：None
* 返 回 值：None
* 功能说明：关闭Led灯
*——————————————————————————*/
void LED_Off(eLed led);

/*——————————————————————————
* 函 数 名：LED_Toggle
* 输入参数：led    灯
* 输出参数：None
* 返 回 值：None
* 功能说明：电平反转
*——————————————————————————*/
void LED_Toggle(eLed led);

#endif

/******************* (C) COPYRIGHT 2011 Leaguer Microelectronics *****END OF FILE****/
