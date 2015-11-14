#ifndef _BASEDEFINE_H
#define _BASEDEFINE_H
/*——————————————————————————
* 文 件 名：Basedefine.h
* 文件说明：基础库头文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2013-12-30
*———————————————————————————*/

/***************标准库函数************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
/************stm32固件库头文件******/
#include "stm32f10x.h"

//真假值
typedef enum
{
  FALSE = 0,        //假
  TRUE  = !FALSE    //真
}bool;


//模式
typedef enum 
{
  MODE_BPSK =  4,      //BPSK模式
  MODE_QPSK =  5,      //QPSK模式
  MODE_ENHANCE = 7,    //ENHANCE模式
  MODE_ROBUST = 8      //鲁棒模式
}eMode;

//设备类型
typedef enum
{
  TYPE_MASTER,      //主节点
  TYPE_METER_SINGLE,    //单相载波表节点
  TYPE_METER_THREE,    //三相载波表节点
  TYPE_COLLECT_I,      //I型采集器节点
  TYPE_COLLECT_II,    //II型采集器节点
  TYPE_RELAY,        //中继节点
  TYPE_METER_485,      //485表非载波节点
  TYPE_UNKNOWN      //未知设备类型
}eType;

//路由状态
typedef enum
{
  ROUTE_PTOP,        //点对点
  ROUTE_SOURCE,      //源路由
  ROUTE_BLIND,      //盲中继
  ROUTE_UNKNOWN      //未知路由
}eRouteStatus;

//路由方式
typedef enum
{
  MODE_PTOP,        //点对点
  MODE_SOURCE,      //源路由
  MODE_BLIND        //盲中继  
}eRouteMode;

//网络标识
typedef enum
{
  NET_SELF,        //本网
  NET_ALL          //全网  
}eNet;

//抄读标志
typedef enum
{
  READ_PREPARE,      //待抄读
  READ_SUCCESS_PART,    //部分抄读成功
  READ_SUCCESS_ALL,    //全部抄读成功  
  READ_FAIL        //全部抄读失败
}eReadFlag;

//地址模式
typedef enum
{
  ADDRTYPE_LONG,      //长地址模式
  ADDRTYPE_SHORT      //短地址模式
}eAddrType;

//时间
typedef struct
{
  u8 year;
  u8 month;
  u8 day;
  u8 hour;
  u8 minute;
  u8 second;
}sTime;


//喂狗
#define FEED_WDG  do{IWDG_ReloadCounter();}while(0)




#endif

