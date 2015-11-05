/*——————————————————————————
* 文 件 名：MAC.h  
* 作    者：邹亮
* 日    期：2014-04-06
* 文件说明：MAC层头文件
*——————————————————————————*/
#ifndef _MAC_H
#define _MAC_H

#include "Comm.h"

#pragma pack(1)

#ifdef __MACDEBUG
#define MACDBG(CODE) PRINT(CODE)
#else
#define MACDBG(CODE)
#endif

#define MacProtoVer      1          //MAC层协议版本号
#define MaxMSDUSize      300          //MAC帧最大载荷长度
#define MacBuffLen      320          //MAC层最大缓冲区长度
#define MaxNB        6          //CSMA-CA算法中的退让次数最大值
#define MaxBE        10          //CSMA-CA算法中的退让指数最大值

#define BroadNID      0xffff        //广播NID
#define BroadAimAddr    0xffff        //广播目标地址

#define WaitSendMaxTime    500          //MAC层最大等待发送时间，5秒
#define SendingMaxTime    400          //MAC层最大发送持续时间，4秒
#define ReadMaxTime      300          //MAC层最大读取反馈时间，3秒

#define NotMatch      0          //不匹配
#define Match        1          //匹配

#define MacSendFailure    0          //MAC层发送失败
#define MacSendSuccess    1          //MAC层发送成功

//返回信息
typedef enum
{
  RET_MAC_SUCCESS,        //返回成功
  RET_MAC_PHASE_ERR,        //传入相位错误  
  RET_MAC_ATTRLEN_ERR,      //属性长度错误
  RET_MAC_ATTR_NOEXIST,      //属性名不存在
}eMacRet;

//发送选项
typedef enum
{
  OPTION_DEFAULT  = 0x00,      //默认  
  OPTION_ACK_READ = 0x01      //发送反馈标识  1 需要读取确认  0 不需要读取确认
}eMacTxOption;

//帧类型
typedef enum
{  
  MAC_FRAME_BEACON,        //信标帧
  MAC_FRAME_DATA,          //数据帧
  MAC_FRAME_ACK,          //应答帧
  MAC_FRAME_STOPRELAY,      //停止转发帧
  MAC_FRAME_RESERVE        //预留
}eMacFrameType;

//发送状态
typedef enum
{
  SEND_STATUS_IDLE,        //发送未完成
  SEND_STATUS_FINISH,        //发送完成
  SEND_STATUS_START,        //发送启动
  SEND_STATUS_TX_ING        //发送中
}eMacSendStatus;

//发送子状态
typedef enum
{
  SEND_SUB_STATUS_TX,        //立即发送
  SEND_SUB_STATUS_CCA,      //CCA检测
  SEND_SUB_STATUS_WAIT      //等待时隙
}eMacSendSubStatus;

/***************MAC层属性***************/

typedef struct
{        
  u8  mac_NB;            //CSMA-CA退让次数
  u8  mac_BE;            //CSMA-CA退让指数  
  u16 mac_NID;          //NID
  u16  mac_Short;          //短地址 
  u8  mac_Addr[6];        //长地址 
}sMacAttr;
/***************MAC层属性***************/

//接收数据
typedef struct 
{  
  u16 mac_recv_buf_len;      //接收帧数据长度
  u8   mac_recv_buf[MacBuffLen];  //接收帧数据缓存区
  u8   mac_recv_rssi;        //接收帧信号值
  u8   mac_recv_data_ready;    //接收到帧数据标识
}sMacRecv;

//发送数据
typedef struct
{
  u8   stoprelay_handle;      //发送停止转发包句柄
  u8   dataframe_handle;      //发送数据包句柄
  u8  follow_send_stoprelay;    //后续发送停止转发包
  u8  follow_send_dataframe;    //后续发送数据包
  u16  wait_read_time;        //等待读取超时时间
  u16  wait_send_time;        //等待启动发送时间  
  u16  wait_send_ack_time;      //等待发送完成时间    
  u8   send_status;        //发送状态
  u8  send_sub_status;      //发送子状态  
  u8   send_result;        //发送结果
  u8   send_result_read;      //发送结果反馈  1 需要等待读取反馈结果 0 不需要等待读取反馈
  u8   send_mode;          //发送模式
  u16  send_CCA_delay;        //发送CCA延时  
  u16  send_buf_len;        //发送数据缓冲区长度
  u8  send_buf[MacBuffLen];    //发送数据缓冲区        
  u8  stoprelay_len;        //停止转发缓冲区长度
  u8  stoprelay_buf[4];      //停止转发缓冲区
}sMacSend;

//停止转发帧
typedef struct
{
  u8  resv:6;            //预留
  u8  frame_type:2;        //帧类型
  u16 nid;            //NID  
  u8  data;            //数据域    
}sMacStopRelayFrame;

/***************MAC层主函数***************/
void Mac_Init(void);
void Mac_Proc(void);
void Mac_CCA_Proc(void);
void Mac_CCA_Confirm(u8 status);
/***************MAC层主函数***************/

/***************MAC层数据服务接口***************/
u8 Mac_StopRelay_Request(u8 handle,u8 life_perild,u8 flag);
u8 Mac_Data_Request(u8 handle,u16 msdulen,u8* msdu,u8 txoptions);
/***************MAC层数据服务接口***************/

/***************MAC层数据指示接口***************/
extern u8 Mcp_Data_Indication(u16 len,u8* buff,u8 rssi);
/***************MAC层数据指示接口***************/

#endif
