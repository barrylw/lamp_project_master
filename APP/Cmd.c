/*����������������������������������������������������
* �� �� ����Cmd.c  
* ��    �ߣ�����
* ��    �ڣ�2013-12-30
* �ļ�˵��������̨���ļ�
*����������������������������������������������������*/
#include "Cmd.h"
#include "hal_radio.h"
#include "sx1276-LoRa.h"

extern u8 tedtbuf[150];
extern u16 resettime;//ϵͳ��λ����
extern ST_UART_FLAG g_DebugRxFlag;
extern u8 g_DebugRxBuffer[RBL_COM2_RX_SIZE];
void read_reg_all(void);
void read_all_flash();

sCmd CmdList[] = 
{
  {"ef",EraseFlash,"ȫ��FLASH"},
  {"rf",ReadFlash,"������FLASH{��� ����}"},
  {"an",AckNode,"ȷ�Ͻڵ�"},
  {"as",AddSource,"���Դ·��{�̵�ַ ���̵�ַ ·������ ·��}"},
  {"em",EraseMeter,"��յ����"},
  {"en",EraseNIDList,"���NID�����б�"},
  {"er",EraseRoute,"���·�ɱ�"},
  {"pf",PrintFind,"��ӡ�ռ���Ϣ"},
  {"pm",PrintMeter,"��ӡ�����"},  
  {"pn",PrintNIDList,"��ӡNID�����б�"},  
  {"pr",PrintRoute,"��ӡ·�ɱ�"},
  {"ps",PrintSource,"��ӡԴ·��"},  
  {"pt",PrintSatTable,"��ӡӳ���{���}"},
  {"ns",GetNwkStatus,"�����״̬"},
  {"rm",ReadMeter,"�������"},
  {"cm",CollMeter,"�ռ�����"},
  {"sb",SendBeacon,"�����ű�{��λ ģʽ ����}"},
  {"qb",QuitBeacon,"�˳��ű�{��λ}"},
  {"st",SetTime,"����ʱ��{�� �� �� ʱ �� ��}"},
  {"gt",GetTime,"��ȡʱ��"},
  {"sn",SetNID,"����NID{NID}"},
  {"gn",GetNID,"��ȡNID"},
  {"rs",RouteStart,"·������"},
  {"rp",RoutePause,"·����ͣ"},
  {"rr",RouteResum,"·�ɻָ�"},
  {"pi",ParaInit,"������ʼ��"},
  {"sr",SysReset,"ϵͳ��λ"},
  {"ver",Version,"�汾��Ϣ"},
  {"ls",ListCmd,"��ӡ�б�"},
  {"readft",read_reg_all},
  {"tx",send_packet},
  {"allflash",read_all_flash},
  //{"su",SendUart,"�����ڷ���{����}"},

  //{"rid", ReadFlashID, "��ȡFLASH ID"},
  //{"rsr", ReadFlashSR, "��ȡFLASH״̬�Ĵ���"},
  //{"rdt", ReadFlashData, "��ȡFLASH����{��λ��ַ ��λ��ַ ����}"},
  //{"wsr", WriteFlashSR, "д��FLASH״̬�Ĵ���{״ֵ̬}"},
  //{"wdt", WriteFlashData, "д��FLASH����{��λ��ַ ��λ��ַ ����}"},
  //{"es", EraseFlashSector, "��������{������λ��ַ ��λ��ַ}"},
  //{"ec", EraseFlashChip, "������Ƭ"},
   /*
  {"rg",ReadReg,"��ȡȫ��RF�Ĵ���"},
  {"wg",WriteReg,"д��ȫ��RF�Ĵ���Ĭ��ֵ"},
  {"tx",Transmit,"�������ݰ�{�ŵ� ����}"},
  {"ct",ContSend,"��������{��ʼ/������ʶ �ŵ�}"},
  {"tp",SetTxPower,"���÷��书��{����ֵ}"},
  {"tm",TestMaster,"����ƹ�Ҳ������ڵ�{���� ���� ���}"},
  {"ts",TestSlaver,"����ƹ�Ҳ��Դӽڵ�{��ʼ/������ʶ}"},
  {"tr",TestResult,"��ѯƹ�Ҳ��Խ��"},
  */
};

u8 CmdListLen = sizeof(CmdList) / sizeof(sCmd);

/*����������������������������������������������������
* �� �� ����Cmd_Proc  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵��������̨�������
*����������������������������������������������������*/
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

/*����������������������������������������������������
* �� �� ����Cmd_Exe  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵��������ִ̨��
*����������������������������������������������������*/
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
      DISP(printf("CMD: ��ʱ%dms\r\n",GetSysTime() - time_t););
      break;
    }
  }
  
  if (!match) 
  {
    if((g_DebugRxBuffer[0] != '\r') && (g_DebugRxBuffer[0] != '\n'))
    {
      //LIST(printf("\r\ncommand not recognized\r\n"););
      DISP(printf("CMD: ��֧������\r\n"););
    }
    else
    {
      LIST(printf("\r\n"););
    }
  }  
}
/*����������������������������������������������������
* �� �� ����CharToU8  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����ASII���ַ�����ת����u8��
*����������������������������������������������������*/
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

/*����������������������������������������������������
* �� �� ����GetU8Para  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵��������̨��ȡ����
*����������������������������������������������������*/
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

/*����������������������������������������������������
* �� �� ����GetU16Para  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵��������̨��ȡ����
*����������������������������������������������������*/
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

/*����������������������������������������������������
* �� �� ����GetStrPara  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵��������̨��ȡ����
*����������������������������������������������������*/
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

/*����������������������������������������������������
* �� �� ����EraseFlash  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����ȫ��FLASH
*����������������������������������������������������*/
void EraseFlash(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����ReadFlash  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����������FLASH
*����������������������������������������������������*/
void ReadFlash(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����AckNode  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����ȷ�Ͻڵ�
*����������������������������������������������������*/
void AckNode(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����AddSource  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�������Դ·��
*����������������������������������������������������*/
void AddSource(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����EraseMeter  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������յ����
*����������������������������������������������������*/
void EraseMeter(void)
{
  //Aps_MeterDoc_AllSetInvalid();  

  //Aps_Init();  

  return;
}

/*����������������������������������������������������
* �� �� ����EraseNIDList  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�������NID�����б�
*����������������������������������������������������*/
void EraseNIDList(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����EraseRoute  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�������·�ɱ�
*����������������������������������������������������*/
void EraseRoute(void)
{
  
  return;
}

/*����������������������������������������������������
* �� �� ����PrintFind 
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ӡ�ռ���Ϣ
*����������������������������������������������������*/
void PrintFind(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����PrintMeter  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ӡ�����
*����������������������������������������������������*/
void PrintMeter(void)
{
  //Aps_Meter_Print();

  return;
}

/*����������������������������������������������������
* �� �� ����PrintNIDList 
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ӡNID�����б�
*����������������������������������������������������*/
void PrintNIDList(void)
{


  return;
}

/*����������������������������������������������������
* �� �� ����PrintRoute  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ӡ·�ɱ�
*����������������������������������������������������*/
void PrintRoute(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����PrintSource  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ӡԴ·��
*����������������������������������������������������*/
void PrintSource(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����PrintSatTable  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ӡӳ���
*����������������������������������������������������*/
void PrintSatTable(void)
{  
  

  return;
}

/*����������������������������������������������������
* �� �� ����GetNwkStatus  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�����������㹤��״̬
*����������������������������������������������������*/
void GetNwkStatus(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����ReadMeter  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵��������
*����������������������������������������������������*/
void ReadMeter(void)
{  
  

  return;
}

/*����������������������������������������������������
* �� �� ����CollMeter  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�����ռ�
*����������������������������������������������������*/
void CollMeter(void)
{    
  
  return;
}

/*����������������������������������������������������
* �� �� ����SendBeacon  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵���������ű�
*����������������������������������������������������*/
void SendBeacon(void)
{    
  

  return;
}

/*����������������������������������������������������
* �� �� ����QuitBeacon  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵�����˳��ű�
*����������������������������������������������������*/
void QuitBeacon(void)
{      
  

  return;
}

/*����������������������������������������������������
* �� �� ����SetTime  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵��������ϵͳʱ��
*����������������������������������������������������*/
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

/*����������������������������������������������������
* �� �� ����GetTime  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ȡϵͳʱ��
*����������������������������������������������������*/
void GetTime(void)
{
  Time_Get();
}

/*����������������������������������������������������
* �� �� ����SetNID  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵��������NID
*����������������������������������������������������*/
void SetNID(void)
{
  

  return;
}

/*����������������������������������������������������
* �� �� ����GetNID  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ȡNID
*����������������������������������������������������*/
void GetNID(void)
{
  
  return;
}

/*����������������������������������������������������
* �� �� ����RouteStart  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����·������
*����������������������������������������������������*/
void RouteStart(void)
{  
  //Aps_RunStatus_SetRequest(ROUTER_CTRL_RESTART);

  return;
}

/*����������������������������������������������������
* �� �� ����RoutePause  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����·����ͣ
*����������������������������������������������������*/
void RoutePause(void)
{  
  //Aps_RunStatus_SetRequest(ROUTER_CTRL_PAUSE);

  return;
}

/*����������������������������������������������������
* �� �� ����RouteResum  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����·�ɻָ�
*����������������������������������������������������*/
void RouteResum(void)
{  
  //Aps_RunStatus_SetRequest(ROUTER_CTRL_RESUM);

  return;
}

/*����������������������������������������������������
* �� �� ����ParaInit  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����������ʼ��
*����������������������������������������������������*/
void ParaInit(void)
{  
  

  return;
}

/*����������������������������������������������������
* �� �� ����Version  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ӡ�汾
*����������������������������������������������������*/
void Version(void)
{  
  //Aps_Print_VerInfo();
  resettime = BKP_ReadBackupRegister(BKP_DR3);
  DISP(printf("SYS: �쳣��λ���� %d\r\n",resettime););

  return;
}

/*����������������������������������������������������
* �� �� ����ListCmd  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵������ӡ����̨����
*����������������������������������������������������*/
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
