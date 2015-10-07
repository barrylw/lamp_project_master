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

//版本
const sVersion Ver = 
{  
  {'Z','L'},
  0x03,0x01,
  0x2014, 0x07, 0x01,
  0x00
};
    
u8 tedtbuf[150];
extern ST_EVENT_CONTROL sRFTransmitEvent;
/*——————————————————————————
* 函 数 名：main  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：主函数
*——————————————————————————*/
#if 0
void main(void)
{
 
  Comm_Init(DISABLE,1);
  Phy_Init();
  //Mac_Init();
  
 
  for (int i =0; i<150 ; i++)
  {
    tedtbuf[i] = i+1;
  }

  //DISP(printf("VER: V%x.%x %04x-%02x-%02x\r\n",Ver.VerIDofMcp,Ver.VerIDofSoft,Ver.Year,Ver.Mon,Ver.Day););
  //SX1276LoRa_Send_Packet(tedtbuf,20);
  while(1)
  {
    Comm_Proc();
    Phy_Proc();
    //Mac_Proc();
  }
}
#endif

int main()
{

  
  for (u8 i = 0; i < 150; i++)
  {
    tedtbuf[i] = i+1;
  }
  
  Comm_Init(ENABLE,1);
  Phy_Init();
 
  printf("start app\r\n");
  process_init();
  process_start(&etimer_process, NULL);
  process_start(&hal_urat_process, NULL);
  process_start(&hal_RF_process, NULL);
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







