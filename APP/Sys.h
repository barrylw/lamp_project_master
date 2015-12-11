#ifndef _SYS_H
#define _SYS_H
/*——————————————————————————
* 文 件 名：Sys.h
* 文件说明：系统相关头文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2014-01-14
*———————————————————————————*/

#include "Basedefine.h"
#include "Rtc.h"

//全局打印
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

//调试打印
#define DEBUG_MODE  

#ifdef DEBUG_MODE
#define __PRODEBUG  //规约库调试打印
#define __MACDEBUG  //MAC层调试打印
#define __PHYDEBUG  //PHY层调试打印
#define __FLHDEBUG  //内部FLASH调试打印
#endif

                        
/*************** 变量定义 ***************/
/*——————————————————————————
* 函 数 名：SysReset
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：系统复位
*——————————————————————————*/
void SysReset(void);

#endif