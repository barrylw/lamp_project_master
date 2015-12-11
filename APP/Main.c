/*����������������������������������������������������
* �� �� ����Main.c  
* ��    �ߣ�����
* ��    �ڣ�2013-12-30
* �ļ�˵�������������ļ�
*����������������������������������������������������*/
#include "Main.h"
#include "FRAM.h"
#include "PHY.h"
#include "hal_radio.h"
#include "etimer.h"
#include "hal_gdflash.h"
//�汾

    
u8 tedtbuf[150];
extern ST_EVENT_CONTROL sRFTransmitEvent;
/*����������������������������������������������������
* �� �� ����main  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����������
*����������������������������������������������������*/
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
    /*ִ��������needspollΪ1�Ľ��̼����������ж��У������3.2*/
    do
    {
       FEED_WDG;
    }
    while (process_run() > 0);
  }
}







