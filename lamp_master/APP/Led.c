/*����������������������������������������������������
* �� �� ����Led.c  
* �ļ�˵����LedԴ�ļ�
*
* ��ǰ�汾��V8.0
* ��    �ߣ�ZL
* ��ʼ���ڣ�2013-12-30
*������������������������������������������������������*/
#include "Led.h"

GPIO_TypeDef* GPIO_PORT[LEDn] = {RXLED_GPIO_PORT, TXLED_GPIO_PORT,NETLED_GPIO_PORT};
uint16_t GPIO_PIN[LEDn] = {RXLED_PIN, TXLED_PIN,NETLED_PIN};
uint32_t GPIO_CLK[LEDn] = {RXLED_GPIO_CLK, TXLED_GPIO_CLK,NETLED_GPIO_CLK};


/************* �������� *************/

/*����������������������������������������������������
* �� �� ����LED_Init
* ���������led    ��
* ���������None
* �� �� ֵ��None
* ����˵������ʼ��LED��
*����������������������������������������������������*/
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

/*����������������������������������������������������
* �� �� ����LED_On
* ���������led    ��
* ���������None
* �� �� ֵ��None
* ����˵��������Led��
*����������������������������������������������������*/
void LED_On(eLed led)
{
  GPIO_PORT[led]->BRR = GPIO_PIN[led];    //���λΪ 0
}

/*����������������������������������������������������
* �� �� ����LED_Off
* ���������led    ��
* ���������None
* �� �� ֵ��None
* ����˵�����ر�Led��
*����������������������������������������������������*/
void LED_Off(eLed led)
{
  GPIO_PORT[led]->BSRR = GPIO_PIN[led];     //����λΪ 1
}

/*����������������������������������������������������
* �� �� ����LED_Toggle
* ���������led    ��
* ���������None
* �� �� ֵ��None
* ����˵������ƽ��ת
*����������������������������������������������������*/
void LED_Toggle(eLed led)
{  
  GPIO_PORT[led]->ODR ^= GPIO_PIN[led];
}


