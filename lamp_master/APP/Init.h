#ifndef _INIT_H
#define _INIT_H
/*——————————————————————————
* 文 件 名：Init.h  
* 文件说明: 初始化源文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2013-12-30
*———————————————————————————*/

#include "Basedefine.h"


/*************函数声明 **************/
/*——————————————————————————
* 函 数 名：RCC_Init
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：RCC时钟初始化
*——————————————————————————*/
void RCC_Init(void);

/*——————————————————————————
* 函 数 名：Nvic_Init
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：中断向量初始化
*——————————————————————————*/
void Nvic_Init(void);

/*——————————————————————————
* 函 数 名：Iwdg_Init
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：看门狗程序初始化
*——————————————————————————*/
void Iwdg_Init(u8 timeout);




#endif