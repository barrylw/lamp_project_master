#ifndef _BASEDEFINE_H
#define _BASEDEFINE_H
/*����������������������������������������������������
* �� �� ����Basedefine.h
* �ļ�˵����������ͷ�ļ�
*
* ��ǰ�汾��V8.0
* ��    �ߣ�ZL
* ��ʼ���ڣ�2013-12-30
*������������������������������������������������������*/

/***************��׼�⺯��************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
/************stm32�̼���ͷ�ļ�******/
#include "stm32f10x.h"

//���ֵ
typedef enum
{
  FALSE = 0,        //��
  TRUE  = !FALSE    //��
}bool;


//ģʽ
typedef enum 
{
  MODE_BPSK =  4,      //BPSKģʽ
  MODE_QPSK =  5,      //QPSKģʽ
  MODE_ENHANCE = 7,    //ENHANCEģʽ
  MODE_ROBUST = 8      //³��ģʽ
}eMode;

//�豸����
typedef enum
{
  TYPE_MASTER,      //���ڵ�
  TYPE_METER_SINGLE,    //�����ز���ڵ�
  TYPE_METER_THREE,    //�����ز���ڵ�
  TYPE_COLLECT_I,      //I�Ͳɼ����ڵ�
  TYPE_COLLECT_II,    //II�Ͳɼ����ڵ�
  TYPE_RELAY,        //�м̽ڵ�
  TYPE_METER_485,      //485����ز��ڵ�
  TYPE_UNKNOWN      //δ֪�豸����
}eType;

//·��״̬
typedef enum
{
  ROUTE_PTOP,        //��Ե�
  ROUTE_SOURCE,      //Դ·��
  ROUTE_BLIND,      //ä�м�
  ROUTE_UNKNOWN      //δ֪·��
}eRouteStatus;

//·�ɷ�ʽ
typedef enum
{
  MODE_PTOP,        //��Ե�
  MODE_SOURCE,      //Դ·��
  MODE_BLIND        //ä�м�  
}eRouteMode;

//�����ʶ
typedef enum
{
  NET_SELF,        //����
  NET_ALL          //ȫ��  
}eNet;

//������־
typedef enum
{
  READ_PREPARE,      //������
  READ_SUCCESS_PART,    //���ֳ����ɹ�
  READ_SUCCESS_ALL,    //ȫ�������ɹ�  
  READ_FAIL        //ȫ������ʧ��
}eReadFlag;

//��ַģʽ
typedef enum
{
  ADDRTYPE_LONG,      //����ַģʽ
  ADDRTYPE_SHORT      //�̵�ַģʽ
}eAddrType;

//ʱ��
typedef struct
{
  u8 year;
  u8 month;
  u8 day;
  u8 hour;
  u8 minute;
  u8 second;
}sTime;


//ι��
#define FEED_WDG  do{IWDG_ReloadCounter();}while(0)




#endif

