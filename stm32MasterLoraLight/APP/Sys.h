#ifndef _SYS_H
#define _SYS_H
/*����������������������������������������������������
* �� �� ����Sys.h
* �ļ�˵����ϵͳ���ͷ�ļ�
*
* ��ǰ�汾��V8.0
* ��    �ߣ�ZL
* ��ʼ���ڣ�2014-01-14
*������������������������������������������������������*/

#include "Basedefine.h"
#include "Rtc.h"

//ȫ�ִ�ӡ
#define PRINT(CODE)   do\
{\
  sTime time;\
  time = TimetoBCD(RTC_GetCounter());\
  printf("%02d:%02d:%02d->", time.hour, time.minute, time.second);\
  CODE\
} while (0)

#define __DISPLAY  

#ifdef __DISPLAY
#define DISP(CODE) PRINT(CODE)
#define LIST(CODE) do{CODE}while(0)
#else
#define DISP(CODE)
#define LIST(CODE)
#endif

//���Դ�ӡ
#define DEBUG_MODE  

#ifdef DEBUG_MODE
#define __PRODEBUG  //��Լ����Դ�ӡ
#define __MACDEBUG  //MAC����Դ�ӡ
#define __PHYDEBUG  //PHY����Դ�ӡ
#define __FLHDEBUG  //�ڲ�FLASH���Դ�ӡ
#endif

                        
/*************** �������� ***************/
/*����������������������������������������������������
* �� �� ����SysReset
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����ϵͳ��λ
*����������������������������������������������������*/
void SysReset(void);

#endif