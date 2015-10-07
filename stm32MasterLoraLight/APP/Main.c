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

//�汾
const sVersion Ver = 
{  
  {'Z','L'},
  0x03,0x01,
  0x2014, 0x07, 0x01,
  0x00
};
    
u8 tedtbuf[150];
extern ST_EVENT_CONTROL sRFTransmitEvent;
/*����������������������������������������������������
* �� �� ����main  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����������
*����������������������������������������������������*/
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
    /*ִ��������needspollΪ1�Ľ��̼����������ж��У������3.2*/
    do
    {
       FEED_WDG;
    }
    while (process_run() > 0);
  }
}







