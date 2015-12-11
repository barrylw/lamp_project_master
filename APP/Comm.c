/*����������������������������������������������������
* �� �� ����Comm.c  
* �ļ�˵����������ƽ̨Դ�ļ�
*
* ��ǰ�汾��V8.0
* ��    �ߣ�ZL
* ��ʼ���ڣ�2013-12-30
*������������������������������������������������������*/
#include "Comm.h"

extern ST_EVENT_METHOD UartEvents[];

/*����������������������������������������������������
* �� �� ����CommInit
* ���������dogstate  ���Ź�ʹ��
      dogtime    ���Ź����ڣ�s��      
      ledtype    ������
* ���������None
* �� �� ֵ��None
* ����˵�����������ʼ��
*����������������������������������������������������*/
void Comm_Init(FunctionalState dogstate,u8 dogtime)
{
  RCC_Init();
  Nvic_Init();
  if(dogstate == ENABLE)
  {
    Iwdg_Init(dogtime);
  }
  UART_Init();
  //Usart_Init(COM1,BR_115200,Parity_Even);
  //Usart_Init(COM2,BR_115200,Parity_Even);
  RTC_Init();
  //Time_Init();
  //IFlash_Init();

  Timer_Init(TIM5);
  //Timer_Start(TIM5);

  LED_Init(RX_LED);
  Delayms(500);
  LED_Off(RX_LED);

  LED_Init(TX_LED);
  Delayms(500);
  LED_Off(TX_LED);

  LED_Init(NET_LED);  
  Delayms(500);
  LED_Off(NET_LED);  

  DISP(printf("SYS: ��ʼ�����\r\n"););
}

/*����������������������������������������������������
* �� �� ����Comm_Proc
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�����������������
*����������������������������������������������������*/
void Comm_Proc()
{
  FEED_WDG;
  hal_RunUartEvents(UartEvents);
}

/************* �������� *************/







