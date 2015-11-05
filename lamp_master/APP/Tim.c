/*����������������������������������������������������
* �� �� ����Tim.c
* �ļ�˵����ϵͳ���Դ�ļ�
*
* ��ǰ�汾��V8.0
* ��    �ߣ�ZL
* ��ʼ���ڣ�2014-01-14
*������������������������������������������������������*/
#include "Tim.h"


/*************** �������� ***************/
TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

extern void Mac_CCA_Proc(void);

/*************** ����ʵ�� ***************/
/*����������������������������������������������������
* �� �� ����Timer_Init  
* ���������
TIMx  ��ʱ��
* ���������None
* �� �� ֵ��None
* ����˵���������Timer��ʼ��
*����������������������������������������������������*/
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
/*����������������������������������������������������
* �� �� ����Timer_Start  
* ���������TIMx  ��ʱ��
* ���������None
* �� �� ֵ��None
* ����˵���������������ʱ��
*����������������������������������������������������*/
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

/*����������������������������������������������������
* �� �� ����TIM5_IRQHandler  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵���������TIM5�ж����
*����������������������������������������������������*/
void TIM5_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) 
  {
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    Mac_CCA_Proc();      
  }

  return;
}