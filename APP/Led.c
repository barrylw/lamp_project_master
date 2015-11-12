/*——————————————————————————
* 文 件 名：Led.c  
* 文件说明：Led源文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2013-12-30
*———————————————————————————*/
#include "Led.h"

GPIO_TypeDef* GPIO_PORT[LEDn] = {RXLED_GPIO_PORT, TXLED_GPIO_PORT,NETLED_GPIO_PORT};
uint16_t GPIO_PIN[LEDn] = {RXLED_PIN, TXLED_PIN,NETLED_PIN};
uint32_t GPIO_CLK[LEDn] = {RXLED_GPIO_CLK, TXLED_GPIO_CLK,NETLED_GPIO_CLK};


/************* 函数声明 *************/

/*——————————————————————————
* 函 数 名：LED_Init
* 输入参数：led    灯
* 输出参数：None
* 返 回 值：None
* 功能说明：初始化LED灯
*——————————————————————————*/
void LED_Init(eLed led)
{
  GPIO_InitTypeDef  GPIO_InitStructure;  

  /* Enable the GPIO_LED Clock */
  RCC_APB2PeriphClockCmd(GPIO_CLK[led], ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN[led];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

  GPIO_Init(GPIO_PORT[led], &GPIO_InitStructure);

  LED_On(led);  
}

/*——————————————————————————
* 函 数 名：LED_On
* 输入参数：led    灯
* 输出参数：None
* 返 回 值：None
* 功能说明：点亮Led灯
*——————————————————————————*/
void LED_On(eLed led)
{
  GPIO_PORT[led]->BRR = GPIO_PIN[led];    //清除位为 0
}

/*——————————————————————————
* 函 数 名：LED_Off
* 输入参数：led    灯
* 输出参数：None
* 返 回 值：None
* 功能说明：关闭Led灯
*——————————————————————————*/
void LED_Off(eLed led)
{
  GPIO_PORT[led]->BSRR = GPIO_PIN[led];     //设置位为 1
}

/*——————————————————————————
* 函 数 名：LED_Toggle
* 输入参数：led    灯
* 输出参数：None
* 返 回 值：None
* 功能说明：电平反转
*——————————————————————————*/
void LED_Toggle(eLed led)
{  
  GPIO_PORT[led]->ODR ^= GPIO_PIN[led];
}


