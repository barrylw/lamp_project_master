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
#include "cfs/cfs-coffee.h"
#include "cfs-coffee-arch.h"

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



/************************************
coffee文件系统测试进程
1、format 格式化文件系统
2、以读写方式打开CoffeeTest文件，文件不存在，新建文件
3、将buf1内容写入文件，再读出，
4、删除文件
5、再打开文件CoffeeTest，应该是新建的文件，旧文件已经删除
6、将buf2内容写入文件，再读出，

************************************/
PROCESS(cfs_test_process, "cfs test");
AUTOSTART_PROCESSES(&cfs_test_process);

PROCESS_THREAD(cfs_test_process, ev, data)
{
  PROCESS_BEGIN();
  printf("***cfs test process start***\r\n");

  if(cfs_coffee_format() == - 1)
  {
    printf("coffee format error.");
    return - 1;
  }

  int fd = cfs_open("CoffeeTest", CFS_WRITE | CFS_READ);
  if(fd == - 1)
  {
    printf("First time open error.");
    return - 1;
  }

  char buf1[]   = "Hello, World!\r\n";
  char buf2[32] = "aaaaa\r\n";
  char buf[100];
  
  printf("The orignal buf1 and buf2 is \r\n: ");
  printf(buf1);
  printf(buf2);

  cfs_write(fd, buf1, sizeof(buf1));
  cfs_write(fd, buf2, sizeof(buf2));
  
  cfs_seek(fd, 0, CFS_SEEK_SET);
  cfs_read( fd, buf, sizeof(buf1) + sizeof(buf2) );
  printf(buf);
  cfs_remove("CoffeeTest");
  
  
  fd = cfs_open("CoffeeTest", CFS_WRITE | CFS_READ);
  cfs_write(fd, buf2, sizeof(buf2));
  cfs_seek(fd, 0, CFS_SEEK_SET);
  cfs_read( fd, buf, sizeof(buf2));
  printf(buf); 
   
  //cfs_seek(fd, 0, CFS_SEEK_SET);
  //cfs_write(fd, buf2, sizeof(buf2));
 
  cfs_close(fd);

  PROCESS_END();
}


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
  FRAM_init();
  
  printf("start app\r\n");
  process_init();
  process_start(&etimer_process, NULL);
  process_start(&hal_urat_process, NULL);
  process_start(&hal_RF_process, NULL);
  process_start(&cfs_test_process, NULL);
  
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







