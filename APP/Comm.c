/*——————————————————————————
* 文 件 名：Comm.c  
* 文件说明：公共库平台源文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2013-12-30
*———————————————————————————*/
#include "Comm.h"

extern ST_EVENT_METHOD UartEvents[];

/*——————————————————————————
* 函 数 名：CommInit
* 输入参数：dogstate  开门狗使能
      dogtime    开门狗周期（s）      
      ledtype    灯类型
* 输出参数：None
* 返 回 值：None
* 功能说明：公共库初始化
*——————————————————————————*/
void Comm_Init(FunctionalState dogstate,u8 dogtime)
{
  RCC_Init();
  Nvic_Init();
  if(dogstate == ENABLE)
  {
    Iwdg_Init(dogtime);
  }
  UART_Init();
  //Usart_Init(COM1,BR_115200,Parity_Even);
  //Usart_Init(COM2,BR_115200,Parity_Even);
  RTC_Init();
  //Time_Init();
  //IFlash_Init();

  Timer_Init(TIM5);
  //Timer_Start(TIM5);

  LED_Init(RX_LED);
  Delayms(500);
  LED_Off(RX_LED);

  LED_Init(TX_LED);
  Delayms(500);
  LED_Off(TX_LED);

  LED_Init(NET_LED);  
  Delayms(500);
  LED_Off(NET_LED);  

  DISP(printf("SYS: 初始化完成\r\n"););
}

/*——————————————————————————
* 函 数 名：Comm_Proc
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：公共库任务调度
*——————————————————————————*/
void Comm_Proc()
{
  FEED_WDG;
  hal_RunUartEvents(UartEvents);
}

/************* 函数声明 *************/







