#ifndef _TIM_H
#define _TIM_H
/*����������������������������������������������������
* �� �� ����Tim.h
* �ļ�˵����ϵͳ���ͷ�ļ�
*
* ��ǰ�汾��V8.0
* ��    �ߣ�ZL
* ��ʼ���ڣ�2014-01-14
*������������������������������������������������������*/

#include "Basedefine.h"


#define PrescalerValue      64

void Timer_Init(TIM_TypeDef* TIMx);

void Timer_Start(TIM_TypeDef* TIMx);

void TIM5_IRQHandler(void);

#endif