/*——————————————————————————
* 文 件 名：Main.c  
* 作    者：邹亮
* 日    期：2013-12-30
* 文件说明：主程序主文件
*——————————————————————————*/
#include "Main.h"
#include "FRAM.h"
#include "PHY.h"
#include "hal_radio.h"
#include "etimer.h"
#include "hal_gdflash.h"
//版本

    
u8 tedtbuf[150];
extern ST_EVENT_CONTROL sRFTransmitEvent;
/*——————————————————————————
* 函 数 名：main  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：主函数
*——————————————————————————*/
int main()
{

  
  for (u8 i = 0; i < 150; i++)
  {
    tedtbuf[i] = i+1;
  }
  
  Comm_Init(ENABLE,1);
 
  
  Phy_Init();
  init_update();
   
  
  printf("start app\r\n");
  process_init();
  process_start(&etimer_process, NULL);
  process_start(&hal_urat_process, NULL);
  process_start(&hal_RF_process, NULL);
  process_start(&hal_RF_reset, NULL);
  process_start(&apl_update_process, NULL);
  
  //process_start(&hal_long_send, NULL);
  //autostart_start(autostart_processes); 
  //start_continuous_mode();


  while (1)
  {
    /*执行完所有needspoll为1的进程及处理完所有队列，详情见3.2*/
    do
    {
       FEED_WDG;
    }
    while (process_run() > 0);
  }
}







