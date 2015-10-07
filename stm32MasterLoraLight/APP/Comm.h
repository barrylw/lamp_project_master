#ifndef _COMM_H
#define _COMM_H
/*����������������������������������������������������
* �� �� ����Comm.h  
* �ļ�˵����������ƽ̨ͷ�ļ�
*
* ��ǰ�汾��V8.0
* ��    �ߣ�ZL
* ��ʼ���ڣ�2013-12-30
*������������������������������������������������������*/

/*************���ú�����**************/
#include "Event.h"
#include "Mem.h"
#include "General.h"
#include "Str.h"
#include "hal_Random.h"
#include "Led.h"
#include "Uart.h"
#include "Rtc.h"
#include "Sys.h"
#include "Delay.h"
#include "Init.h"
#include "Flash.h"
#include "Tim.h"
#include "Radio.h"

/*����������������������������������������������������
* �� �� ����CommInit
* ���������dogstate  ���Ź�ʹ��
      dogtime    ���Ź����ڣ�s��      
      ledtype    ������
* ���������None
* �� �� ֵ��None
* ����˵�����������ʼ��
*����������������������������������������������������*/
void Comm_Init(FunctionalState dogstate,u8 dogtime);

/*����������������������������������������������������
* �� �� ����Comm_Proc
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�����������������
*����������������������������������������������������*/
void Comm_Proc();


void * w_memcpy( void *out, const void *in, unsigned int n);
void *w_memset(void *out, int value, unsigned int n);

#define memcpy(dest, src, count)   w_memcpy(dest, src, count)
#define memset(dest, value, count) w_memset(dest, value, count)




#endif
