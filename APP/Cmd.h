/*——————————————————————————
* 文 件 名：Cmd.h  
* 作    者：邹亮
* 日    期：2013-12-30
* 文件说明：控制台头文件
*——————————————————————————*/
#ifndef _CMD_H
#define _CMD_H

#include "MCP.h"
#include "comm.h"

typedef const struct  
{
  const u8 *cmd;
  void (*action)(void);
  const u8 *info;
} sCmd;

void Cmd_Proc(void);
void Cmd_Exe(void); 
void GetU8Para(u8 *para,u8 index);
void GetU16Para(u16 *para,u8 index);
u8 GetStrPara(u8 index,u8* buffer);

void EraseFlash(void);
void ReadFlash(void);
void AckNode(void);
void AddSource(void);
void EraseMeter(void);
void EraseNIDList(void);
void EraseRoute(void);
void PrintFind(void);
void PrintMeter(void);
void PrintNIDList(void);
void PrintRoute(void);
void PrintSource(void);
void PrintSatTable(void);
void GetNwkStatus(void);
void ReadMeter(void);
void CollMeter(void);
void SendBeacon(void);
void QuitBeacon(void);
void SetTime(void);
void GetTime(void);
void SetNID(void);
void GetNID(void);
void RouteStart(void);
void RoutePause(void);
void RouteResum(void);
void ParaInit(void);
void Version(void);
void ListCmd(void);
void send_packet(void);
#endif 

