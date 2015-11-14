/*����������������������������������������������������
* �� �� ����Led.h  
* �ļ�˵����Ledͷ�ļ�
*
* ��ǰ�汾��V8.0
* ��    �ߣ�ZL
* ��ʼ���ڣ�2013-12-30
*������������������������������������������������������*/
#ifndef _LED_H
#define _LED_H

#include "Basedefine.h"

/*************** �궨�� ***************/
#define LEDn                             3

//���ڽ��յ�
#define RXLED_PIN                         GPIO_Pin_12
#define RXLED_GPIO_PORT                   GPIOC
#define RXLED_GPIO_CLK                    RCC_APB2Periph_GPIOC 

//���ڷ��͵�
#define TXLED_PIN                         GPIO_Pin_2
#define TXLED_GPIO_PORT                   GPIOD
#define TXLED_GPIO_CLK                    RCC_APB2Periph_GPIOD  

//�����
#define NETLED_PIN                        GPIO_Pin_3
#define NETLED_GPIO_PORT                  GPIOB
#define NETLED_GPIO_CLK                   RCC_APB2Periph_GPIOB


/*************** �ṹ���� ***************/
typedef enum 
{
  RX_LED = 0,        //���ڽ��յ�
  TX_LED = 1,        //���ڷ��͵�
  NET_LED = 2,      //�����  
}eLed;

/*************** ����ʵ�� ***************/

/*����������������������������������������������������
* �� �� ����LED_Init
* ���������led    ��
* ���������None
* �� �� ֵ��None
* ����˵������ʼ��LED��
*����������������������������������������������������*/
void LED_Init(eLed led);

/*����������������������������������������������������
* �� �� ����LED_On
* ���������led    ��
* ���������None
* �� �� ֵ��None
* ����˵��������Led��
*����������������������������������������������������*/
void LED_On (eLed led);

/*����������������������������������������������������
* �� �� ����LED_Off
* ���������led    ��
* ���������None
* �� �� ֵ��None
* ����˵�����ر�Led��
*����������������������������������������������������*/
void LED_Off(eLed led);

/*����������������������������������������������������
* �� �� ����LED_Toggle
* ���������led    ��
* ���������None
* �� �� ֵ��None
* ����˵������ƽ��ת
*����������������������������������������������������*/
void LED_Toggle(eLed led);

#endif

/******************* (C) COPYRIGHT 2011 Leaguer Microelectronics *****END OF FILE****/
