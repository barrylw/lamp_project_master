#ifndef _COMM_H
#define _COMM_H
/*——————————————————————————
* 文 件 名：Comm.h  
* 文件说明：公共库平台头文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2013-12-30
*———————————————————————————*/

/*************常用函数库**************/
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

/*——————————————————————————
* 函 数 名：CommInit
* 输入参数：dogstate  开门狗使能
      dogtime    开门狗周期（s）      
      ledtype    灯类型
* 输出参数：None
* 返 回 值：None
* 功能说明：公共库初始化
*——————————————————————————*/
void Comm_Init(FunctionalState dogstate,u8 dogtime);

/*——————————————————————————
* 函 数 名：Comm_Proc
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：公共库任务调度
*——————————————————————————*/
void Comm_Proc();







#endif
