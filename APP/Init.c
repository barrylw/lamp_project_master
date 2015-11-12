/*����������������������������������������������������
* �� �� ����Init.c  
* �ļ�˵��: ��ʼ��Դ�ļ�
*
* ��ǰ�汾��V8.0
* ��    �ߣ�ZL
* ��ʼ���ڣ�2013-12-30
*������������������������������������������������������*/

#include "Init.h"


/*************** ����ʵ�� ***************/

/*����������������������������������������������������
* �� �� ����RCC_Init
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����RCCʱ�ӳ�ʼ��
*����������������������������������������������������*/
void RCC_Init(void)
{
  /* Initialize the Embedded Flash Interface, the PLL and update the  SystemCoreClock variable */
  //SystemInit();
  //SystemCoreClockUpdate();

  /* Enable Clock Security System(CSS): this will generate an NMI exception
  when HSE clock fails */
  RCC_ClockSecuritySystemCmd(ENABLE);

  /* Setup SysTick Timer for 1 msec interrupts  */
  if(SysTick_Config(SystemCoreClock / 1000))
  { 
    while(1);
  }  
}

/*����������������������������������������������������
* �� �� ����RCC_IRQHandler
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����RCC�жϷ�����
*����������������������������������������������������*/
void RCC_IRQHandler(void)
{
    if(RCC_GetITStatus(RCC_IT_HSERDY) != RESET)
    { 
        /* Clear HSERDY interrupt pending bit */
        RCC_ClearITPendingBit(RCC_IT_HSERDY);

        /* Check if the HSE clock is still available */
        if (RCC_GetFlagStatus(RCC_FLAG_HSERDY) != RESET)
        { 
            /* Enable PLL: once the PLL is ready the PLLRDY interrupt is generated */ 
            RCC_PLLCmd(ENABLE);
        }
    }
  
    if(RCC_GetITStatus(RCC_IT_PLLRDY) != RESET)
    { 
        /* Clear PLLRDY interrupt pending bit */
        RCC_ClearITPendingBit(RCC_IT_PLLRDY);

        /* Check if the PLL is still locked */
        if (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) != RESET)
        { 
            /* Select PLL as system clock source */
            RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        }
    }
}

/*����������������������������������������������������
* �� �� ����NVIC_Init
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�����ж�������ʼ��
*����������������������������������������������������*/
void Nvic_Init(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  

  /* Enable and configure RCC global IRQ channel */
  NVIC_InitStructure.NVIC_IRQChannel = RCC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable EXTI Line0 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  
  /* Enable USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable UART4 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable DMA1 Channel2 (SPI1 RX) Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable DMA1 Channel3 (SPI1 TX) Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable DMA1 Channel4 (USART1 TX) Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable DMA2 Channel5 (UART4 TX) Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
}

/*����������������������������������������������������
* �� �� ����Iwdg_Init
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�����������Ź������ʼ��
*����������������������������������������������������*/
void Iwdg_Init(u8 timeout)
{  
  if(timeout < 1)
  {
    timeout = 1;
  }  
  else if(timeout > 50)
  {
    timeout = 50;
  }
  /* Check if the system has resumed from IWDG reset */
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
  {
    RCC_ClearFlag();
  }

  /* IWDG timeout equal to 280 ms (the timeout may varies due to LSI frequency
  dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_256);

  /* Set counter reload value to 1250, 1s timeout */
  IWDG_SetReload(1250 * timeout);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();  
}

