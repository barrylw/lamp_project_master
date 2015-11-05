/*——————————————————————————
* 文 件 名：Tim.c
* 文件说明：系统相关源文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2014-01-14
*———————————————————————————*/
#include "Tim.h"


/*************** 变量定义 ***************/
TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

extern void Mac_CCA_Proc(void);

/*************** 函数实现 ***************/
/*——————————————————————————
* 函 数 名：Timer_Init  
* 输入参数：
TIMx  定时器
* 输出参数：None
* 返 回 值：None
* 功能说明：物理层Timer初始化
*——————————————————————————*/
void Timer_Init(TIM_TypeDef* TIMx)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseStructure.TIM_Period = 0;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
}
/*——————————————————————————
* 函 数 名：Timer_Start  
* 输入参数：TIMx  定时器
* 输出参数：None
* 返 回 值：None
* 功能说明：物理层启动定时器
*——————————————————————————*/
void Timer_Start(TIM_TypeDef* TIMx)
{
  TIM_TimeBaseStructure.TIM_Period = 4;
  TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/1000; // 10 ms
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
  TIM_ClearFlag(TIMx, TIM_IT_Update);
  TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIMx, ENABLE);

  return;
}

/*——————————————————————————
* 函 数 名：TIM5_IRQHandler  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：物理层TIM5中断入口
*——————————————————————————*/
void TIM5_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) 
  {
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    Mac_CCA_Proc();      
  }

  return;
}