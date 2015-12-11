/*——————————————————————————
* 文 件 名：Cmd.c  
* 作    者：邹亮
* 日    期：2013-12-30
* 文件说明：控制台主文件
*——————————————————————————*/
#include "Cmd.h"
#include "hal_radio.h"
#include "sx1276-LoRa.h"

extern u8 tedtbuf[150];
extern u16 resettime;//系统复位次数
extern ST_UART_FLAG g_DebugRxFlag;
extern u8 g_DebugRxBuffer[RBL_COM2_RX_SIZE];
void read_reg_all(void);
void read_all_flash();

sCmd CmdList[] = 
{
  {"ef",EraseFlash,"全擦FLASH"},
  {"rf",ReadFlash,"读物理FLASH{块号 长度}"},
  {"an",AckNode,"确认节点"},
  {"as",AddSource,"添加源路由{短地址 父短地址 路径级数 路径}"},
  {"em",EraseMeter,"清空电表档案"},
  {"en",EraseNIDList,"清空NID侦听列表"},
  {"er",EraseRoute,"清空路由表"},
  {"pf",PrintFind,"打印收集信息"},
  {"pm",PrintMeter,"打印电表档案"},  
  {"pn",PrintNIDList,"打印NID侦听列表"},  
  {"pr",PrintRoute,"打印路由表"},
  {"ps",PrintSource,"打印源路由"},  
  {"pt",PrintSatTable,"打印映射表{序号}"},
  {"ns",GetNwkStatus,"网络层状态"},
  {"rm",ReadMeter,"抄表测试"},
  {"cm",CollMeter,"收集测试"},
  {"sb",SendBeacon,"发送信标{相位 模式 级数}"},
  {"qb",QuitBeacon,"退出信标{相位}"},
  {"st",SetTime,"设置时间{年 月 日 时 分 秒}"},
  {"gt",GetTime,"获取时间"},
  {"sn",SetNID,"设置NID{NID}"},
  {"gn",GetNID,"获取NID"},
  {"rs",RouteStart,"路由重启"},
  {"rp",RoutePause,"路由暂停"},
  {"rr",RouteResum,"路由恢复"},
  {"pi",ParaInit,"参数初始化"},
  {"sr",SysReset,"系统复位"},
  {"ver",Version,"版本信息"},
  {"ls",ListCmd,"打印列表"},
  {"readft",read_reg_all},
  {"tx",send_packet},
  {"allflash",read_all_flash},
  //{"su",SendUart,"主串口发送{长度}"},

  //{"rid", ReadFlashID, "读取FLASH ID"},
  //{"rsr", ReadFlashSR, "读取FLASH状态寄存器"},
  //{"rdt", ReadFlashData, "读取FLASH数据{高位地址 低位地址 长度}"},
  //{"wsr", WriteFlashSR, "写入FLASH状态寄存器{状态值}"},
  //{"wdt", WriteFlashData, "写入FLASH数据{高位地址 低位地址 长度}"},
  //{"es", EraseFlashSector, "擦除扇区{扇区高位地址 低位地址}"},
  //{"ec", EraseFlashChip, "擦除整片"},
   /*
  {"rg",ReadReg,"读取全部RF寄存器"},
  {"wg",WriteReg,"写入全部RF寄存器默认值"},
  {"tx",Transmit,"发射数据包{信道 长度}"},
  {"ct",ContSend,"持续发射{开始/结束标识 信道}"},
  {"tp",SetTxPower,"设置发射功率{功率值}"},
  {"tm",TestMaster,"启动乒乓测试主节点{包长 包数 间隔}"},
  {"ts",TestSlaver,"启动乒乓测试从节点{开始/结束标识}"},
  {"tr",TestResult,"查询乒乓测试结果"},
  */
};

u8 CmdListLen = sizeof(CmdList) / sizeof(sCmd);

/*——————————————————————————
* 函 数 名：Cmd_Proc  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：控制台任务调度
*——————————————————————————*/
void Cmd_Proc(void) 
{
  if (g_DebugRxFlag.fLen > 0)
  {
    Cmd_Exe();
    LIST(printf("C>"););
  }
  else
  {
    LIST(printf("\r\ncommand length error!\r\n"););
  }
  
  OS_MemClr(g_DebugRxBuffer, sizeof(g_DebugRxBuffer));
  g_DebugRxFlag.fLen = 0;
}

/*——————————————————————————
* 函 数 名：Cmd_Exe  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：控制台执行
*——————————————————————————*/
void Cmd_Exe(void) 
{
  u32 time_t = 0;
  u8 commandLength = 0;
  u8 commandByte = 0;
  const u8 *command;
  bool match = TRUE;

  // go through each command
  for (u8 i = 0; i < CmdListLen; i++)
  {
    // get the command
    command = CmdList[i].cmd;
    
    // get the length of the cmd
    commandLength = 0;
    commandByte = CmdList[i].cmd[0];
    while (commandByte != 0)
    {
      commandLength++;
      commandByte = CmdList[i].cmd[commandLength];
    }

    match = TRUE;
    // check for differences in the command and what was entered
    for (u8 j = 0; j < commandLength; j++)
    {
      if (command[j] != g_DebugRxBuffer[j])
      {
        match = FALSE;
        break;
      }
    }
    // commands with 0 length are invalid
    if (commandLength < 1)
    {
      match = FALSE;
    }
    // if command matches command entered call the callback
    if (match)
    {
      time_t = GetSysTime();
      //LIST(printf("\r\n"););
      (CmdList[i].action)();
      //LIST(printf("it has taken %d ms\r\n", GetSysTime() - time_t););
      DISP(printf("CMD: 耗时%dms\r\n",GetSysTime() - time_t););
      break;
    }
  }
  
  if (!match) 
  {
    if((g_DebugRxBuffer[0] != '\r') && (g_DebugRxBuffer[0] != '\n'))
    {
      //LIST(printf("\r\ncommand not recognized\r\n"););
      DISP(printf("CMD: 不支持命令\r\n"););
    }
    else
    {
      LIST(printf("\r\n"););
    }
  }  
}
/*——————————————————————————
* 函 数 名：CharToU8  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：ASII码字符数据转换成u8型
*——————————————————————————*/
u8 CharToU8(u8 data)
{
  if ('0' <= data && data <= '9')
  {
    return data - '0';
  }
  else if ('a' <= data && data <= 'f')
  {
    return data - 'a' + 10;
  }
  else if ('A' <= data && data <= 'F')
  {
    return data - 'A' + 10;
  }
  else
  {
    return 0;
  }
}

/*——————————————————————————
* 函 数 名：GetU8Para  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：控制台获取数据
*——————————————————————————*/
void GetU8Para(u8 *para,u8 index)
{
  u8  i = 0;

  for(u16 j=0;j<g_DebugRxFlag.fLen;j++)
  {
    if(g_DebugRxBuffer[j] == ' ')
    {
      i++;
      if(i == index)
      {
        if ((g_DebugRxBuffer[j + 2] == ' ') \
          || (g_DebugRxBuffer[j + 2] == '\r') \
          || (g_DebugRxBuffer[j + 2] == '\n'))
        {
          (*para) = CharToU8(g_DebugRxBuffer[j+1]); 
        }
        else if ((g_DebugRxBuffer[j + 3] == ' ') \
          || (g_DebugRxBuffer[j + 3] == '\r') \
          || (g_DebugRxBuffer[j + 3] == '\n'))
        {
          u8 temp1,temp2;
          temp1 = CharToU8(g_DebugRxBuffer[j + 1]);
          temp1 *= 10;
          temp2 = CharToU8(g_DebugRxBuffer[j + 2]);
          temp2 += temp1;
          *para = temp2;
        }
        else
        {
          u8 temp1, temp2, temp3;
          temp1 = CharToU8(g_DebugRxBuffer[j + 1]);
          temp1 *= 100;
          temp2 = CharToU8(g_DebugRxBuffer[j + 2]);
          temp2 *= 10;
          temp3 = CharToU8(g_DebugRxBuffer[j + 3]);
          temp3 += temp1 + temp2;
          (*para) = temp3;
        }
        return;
      } 
    }
  }

  *para = 0;

  return;
}

/*——————————————————————————
* 函 数 名：GetU16Para  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：控制台获取数据
*——————————————————————————*/
void GetU16Para(u16 *para,u8 index)
{  
  u8  i = 0;
  u8  length;
  u8  placeVal;
  u8  startPos = 0;
  u16 j = 0;
  u16 value = 0;
  u16 digitMod = 1;  

  for(j=0;j<g_DebugRxFlag.fLen;j++)
  {
    if(g_DebugRxBuffer[j] == ' ')
    {
      i++;

      if(i == index)
      {
        u8 *finger = &g_DebugRxBuffer[j+1];
        u8 *header = &g_DebugRxBuffer[j+1];  

        while((*finger) != ' ')
        {
          finger++;

          if((*finger) == '\r' || (*finger) == '\n')
          {
            break;
          }

          if((finger-header) > 5)
          {
            break;
          }
        }
        length = finger-header;

        if((*header) == '-')
        {
          startPos = 1;
        }

        for(i=length;i>startPos;i--)
        {
          finger--;
          placeVal = (*finger) - 48;
          value = value + (digitMod * placeVal);
          digitMod = digitMod * 10;
        }

        if(startPos == 1)
        {
          value = 65536 - value;
        }

        length=0;

        break;
      }
    }
  }

  *para = value;

  return;
}

/*——————————————————————————
* 函 数 名：GetStrPara  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：控制台获取数据
*——————————————————————————*/
u8 GetStrPara(u8 index,u8* buffer)
{
  u8  i = 0;
  u8  len = 0; 
  u8* buf = buffer;
  u16 j = 0;

  for(j=0;j<g_DebugRxFlag.fLen;j++)
  {
    if(g_DebugRxBuffer[j] == ' ')
    {
      i++;

      if(i == index)
      {
        while(g_DebugRxBuffer[j+1] != ' ' & (g_DebugRxBuffer[j+1] != '\r') & (g_DebugRxBuffer[j+1] != '\n'))
        {
          *buf = g_DebugRxBuffer[j+1];
          buf++;
          j++;
          len++;
        }

        return len;  
      } 
    }
  }

  return len;
}

/*——————————————————————————
* 函 数 名：EraseFlash  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：全擦FLASH
*——————————————————————————*/
void EraseFlash(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：ReadFlash  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：读物理FLASH
*——————————————————————————*/
void ReadFlash(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：AckNode  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：确认节点
*——————————————————————————*/
void AckNode(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：AddSource  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：添加源路由
*——————————————————————————*/
void AddSource(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：EraseMeter  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：清空电表档案
*——————————————————————————*/
void EraseMeter(void)
{
  //Aps_MeterDoc_AllSetInvalid();  

  //Aps_Init();  

  return;
}

/*——————————————————————————
* 函 数 名：EraseNIDList  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：清空NID侦听列表
*——————————————————————————*/
void EraseNIDList(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：EraseRoute  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：清空路由表
*——————————————————————————*/
void EraseRoute(void)
{
  
  return;
}

/*——————————————————————————
* 函 数 名：PrintFind 
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：打印收集信息
*——————————————————————————*/
void PrintFind(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：PrintMeter  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：打印电表档案
*——————————————————————————*/
void PrintMeter(void)
{
  //Aps_Meter_Print();

  return;
}

/*——————————————————————————
* 函 数 名：PrintNIDList 
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：打印NID侦听列表
*——————————————————————————*/
void PrintNIDList(void)
{


  return;
}

/*——————————————————————————
* 函 数 名：PrintRoute  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：打印路由表
*——————————————————————————*/
void PrintRoute(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：PrintSource  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：打印源路由
*——————————————————————————*/
void PrintSource(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：PrintSatTable  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：打印映射表
*——————————————————————————*/
void PrintSatTable(void)
{  
  

  return;
}

/*——————————————————————————
* 函 数 名：GetNwkStatus  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：获得网络层工作状态
*——————————————————————————*/
void GetNwkStatus(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：ReadMeter  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：抄表
*——————————————————————————*/
void ReadMeter(void)
{  
  

  return;
}

/*——————————————————————————
* 函 数 名：CollMeter  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：收集
*——————————————————————————*/
void CollMeter(void)
{    
  
  return;
}

/*——————————————————————————
* 函 数 名：SendBeacon  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：发送信标
*——————————————————————————*/
void SendBeacon(void)
{    
  

  return;
}

/*——————————————————————————
* 函 数 名：QuitBeacon  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：退出信标
*——————————————————————————*/
void QuitBeacon(void)
{      
  

  return;
}

/*——————————————————————————
* 函 数 名：SetTime  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：设置系统时钟
*——————————————————————————*/
void SetTime(void)
{
  u8 year = 10;
  u8 month = 1;
  u8 day = 1;
  u8 hour = 0;
  u8 minute = 0;
  u8 second = 0;
  
  GetU8Para(&year, 1);
  GetU8Para(&month, 2);
  GetU8Para(&day, 3);
  GetU8Para(&hour, 4);
  GetU8Para(&minute, 5);
  GetU8Para(&second, 6);
  
  if (year == 0 || year > 99)
  {
    year = 10;
  }

  if (month == 0 || month > 12)
  {
    month = 1;
  }

  if (day == 0 || day > 31)
  {
    day = 1;
  }

  if (hour >= 24)
  {
    hour = 0;
  }

  if (minute >= 60)
  {
    minute = 0;
  }

  if (second >= 60)
  {
    second = 0;
  }

  Time_Set(year, month, day, hour, minute, second);
}

/*——————————————————————————
* 函 数 名：GetTime  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：获取系统时钟
*——————————————————————————*/
void GetTime(void)
{
  Time_Get();
}

/*——————————————————————————
* 函 数 名：SetNID  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：设置NID
*——————————————————————————*/
void SetNID(void)
{
  

  return;
}

/*——————————————————————————
* 函 数 名：GetNID  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：获取NID
*——————————————————————————*/
void GetNID(void)
{
  
  return;
}

/*——————————————————————————
* 函 数 名：RouteStart  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：路由重启
*——————————————————————————*/
void RouteStart(void)
{  
  //Aps_RunStatus_SetRequest(ROUTER_CTRL_RESTART);

  return;
}

/*——————————————————————————
* 函 数 名：RoutePause  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：路由暂停
*——————————————————————————*/
void RoutePause(void)
{  
  //Aps_RunStatus_SetRequest(ROUTER_CTRL_PAUSE);

  return;
}

/*——————————————————————————
* 函 数 名：RouteResum  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：路由恢复
*——————————————————————————*/
void RouteResum(void)
{  
  //Aps_RunStatus_SetRequest(ROUTER_CTRL_RESUM);

  return;
}

/*——————————————————————————
* 函 数 名：ParaInit  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：参数初始化
*——————————————————————————*/
void ParaInit(void)
{  
  

  return;
}

/*——————————————————————————
* 函 数 名：Version  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：打印版本
*——————————————————————————*/
void Version(void)
{  
  //Aps_Print_VerInfo();
  resettime = BKP_ReadBackupRegister(BKP_DR3);
  DISP(printf("SYS: 异常复位次数 %d\r\n",resettime););

  return;
}

/*——————————————————————————
* 函 数 名：ListCmd  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：打印控制台命令
*——————————————————————————*/
void ListCmd(void)
{  
  u8 i;

  for (i=0; i<CmdListLen; i++)
  {
    DISP(printf("CMD: %s\t%s\r\n",CmdList[i].cmd,CmdList[i].info););    
  }

  return;
}

void read_reg_all(void)
{
  u8 startReg = 0;
  u8 endReg = 0;
  u8 tempValue;
  
  GetU8Para(&startReg, 1);
  GetU8Para(&endReg, 2);
  
  for (u8 i = startReg; i<= endReg; i++)
  {
    if ((i % 10 == 0) && (i > 0))
    {
      printf("\r\n");
    }
    SX1276Read(i, &tempValue);
    printf("%x  ", tempValue);
  }
  printf("\r\n");
}

void send_packet(void)
{
  u8 length;
  GetU8Para(&length, 1);
  #ifndef USE_LORA_MODE
  SX1276Fsk_Send_Packet(tedtbuf, length); 
#else
  SX1276LoRa_Send_Packet(tedtbuf, length);
  #endif
}

void read_all_flash(void)
{
  u32 addr = 4096;
  u8  temp;
  
   for (u32 i = 0; i < 128; i++)
   {
     GDflash_read(i+ addr, &temp, 1);
     printf("%.2x ", temp);
   }
    printf("\r\n");
}
