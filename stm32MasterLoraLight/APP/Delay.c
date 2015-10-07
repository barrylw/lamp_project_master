/*——————————————————————————
* 文 件 名：Delay.c  
* 文件说明：延时函数源文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2013-12-30
*———————————————————————————*/
#include "Delay.h"
#include "Rtc.h"

/*——————————————————————————
* 函 数 名：Delay_Nop
* 输入参数：time:需要延时的时间
* 输出参数：None
* 返 回 值：None
* 功能说明：延时函数
*——————————————————————————*/
void Delay_Nop(u32 DelayNop_D)
{
  do
  {
    DelayNop_D--;  
    if(DelayNop_D == 1)
      break;
  }while(DelayNop_D > 0);
}

/*——————————————————————————
* 函 数 名：Delay_Nop
* 输入参数：time:需要延时的时间
* 输出参数：None
* 返 回 值：None
* 功能说明：延时函数
*——————————————————————————*/
void Delayms(u16 ms)
{
    u32 time = 0;

    time = GetSysTime();
    while (GetSysTime() - time < ms)
    {
        FEED_WDG;
    }
}