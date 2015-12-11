#ifndef _TIM_H
#define _TIM_H
/*——————————————————————————
* 文 件 名：Tim.h
* 文件说明：系统相关头文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2014-01-14
*———————————————————————————*/

#include "Basedefine.h"


#define PrescalerValue      64

void Timer_Init(TIM_TypeDef* TIMx);

void Timer_Start(TIM_TypeDef* TIMx);

void TIM5_IRQHandler(void);

#endif