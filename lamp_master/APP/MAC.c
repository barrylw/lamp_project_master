/*——————————————————————————
* 文 件 名：MAC.c  
* 作    者：邹亮
* 日    期：2014-04-06
* 文件说明：MAC层主文件
*——————————————————————————*/
#include "MAC.h"
#include "PHY.h"


/***************MAC层变量***************/
sMacAttr   MacAttr;      //MAC层属性
sMacRecv   MacRecv;      //MAC层接收数据
sMacSend   MacSend;      //MAC层发送数据
/***************MAC层变量***************/


/***************MAC层外部变量***************/

/***************MAC层外部变量***************/


/***************MAC层函数***************/
static void Mac_Vari_Init(void);
static void Mac_Tick(void);
static u8   Mac_Send(u8 handle,u8 frametype,u8 txoptions);
/***************MAC层函数***************/


/***************MAC层初始化***************/

/*——————————————————————————
* 函 数 名：Mac_Init  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：MAC层初始化
*——————————————————————————*/
void Mac_Init(void)
{
  //初始化
  Mac_Vari_Init();  

  MACDBG(printf("MAC: 初始化完成\r\n"););

  return;  
}

/*——————————————————————————
* 函 数 名：Mac_Vari_Init  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：MAC层相关初始化
*——————————————————————————*/
static void Mac_Vari_Init(void)
{  
  u16 NID = 0;  
  
  MemSet((u8 *)&MacAttr,0,sizeof(sMacAttr));
  MemSet((u8 *)&MacRecv,0,sizeof(sMacRecv));  
  MemSet((u8 *)&MacSend,0,sizeof(sMacSend));        
  
  MacAttr.mac_NID = NID;  
  
  return;
}

/***************MAC层初始化***************/


/***************MAC层周期任务***************/

/*——————————————————————————
* 函 数 名：Mac_Proc  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：MAC层周期任务
*——————————————————————————*/
void Mac_Proc(void)
{  
  //MAC任务
  Mac_Tick();

  return;
}

/*——————————————————————————
* 函 数 名：Mac_Tick  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：MAC层任务
*——————————————————————————*/
static void Mac_Tick(void)
{
  if (MacSend.send_status == SEND_STATUS_IDLE)  
  {
    if(MacSend.follow_send_stoprelay == 1)
    {
      DISP(printf("MAC: 下一次发送停止转发\r\n"););
      MacSend.follow_send_stoprelay = 0;      
      Mac_Send(MacSend.stoprelay_handle,MAC_FRAME_STOPRELAY,MacSend.send_result_read);

    }
    else if(MacSend.follow_send_dataframe == 1)
    {
      DISP(printf("MAC: 下一次发送数据\r\n"););
      MacSend.follow_send_dataframe = 0;
      Mac_Send(MacSend.dataframe_handle,MAC_FRAME_DATA,MacSend.send_result_read);      
    }
  }

  if(MacRecv.mac_recv_data_ready == 1)      
  {  
    sMacRecv mac_recv_temp;

    MacRecv.mac_recv_data_ready = 0;  
    
    //将接收数据拷贝到临时缓冲区再上传 防止上传数据未处理完而被中断接收覆盖
    MemCpy(&mac_recv_temp,&MacRecv,sizeof(sMacRecv));
    
    MACDBG(printf("MAC: 收到数据\r\n"););
    PrintBuff(mac_recv_temp.mac_recv_buf,mac_recv_temp.mac_recv_buf_len);  
    
    //数据交给MCP上报
    Mcp_Data_Indication(mac_recv_temp.mac_recv_buf_len,mac_recv_temp.mac_recv_buf,mac_recv_temp.mac_recv_rssi);  
  }

  return;
}

/***************MAC层周期任务***************/


/***************MAC层中断任务***************/

/*——————————————————————————
* 函 数 名：Mac_CCA_Proc  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：MAC层CCA中断任务
*——————————————————————————*/
void Mac_CCA_Proc(void)
{  
  u8 tmp;
  
  //数据发送启动
  if (MacSend.send_status == SEND_STATUS_START)
  {
    MacSend.wait_send_time++;
    if(MacSend.wait_send_time >= WaitSendMaxTime)
    {    
      //超过等待发送总时间 发送失败
      //MACDBG(printf("MAC: %d WT\r\n",phase););
      MacSend.send_result = MacSendFailure; 
      if (MacSend.send_result_read)
      {  
        MacSend.wait_read_time = 0;
        MacSend.send_status = SEND_STATUS_FINISH;        
      }
      else
      {
        MacSend.send_status = SEND_STATUS_IDLE;
      }      
    }

    //发送子状态
    switch (MacSend.send_sub_status)
    {
    case SEND_SUB_STATUS_TX:  
      //不处理
      break;

    case SEND_SUB_STATUS_CCA:      
      //不处理            
      break;

    case SEND_SUB_STATUS_WAIT:
      //等待时隙
      if(MacSend.send_CCA_delay != 0)    
      {
        MacSend.send_CCA_delay--;
      }

      if(0 == MacSend.send_CCA_delay)
      {  
        //等待时隙到 再次发起CCA请求  
        MacSend.send_sub_status = SEND_SUB_STATUS_CCA;
        Phy_CCA_Request();        
      }      
      break;
      
    default:
      break;
    }    
  }

  //数据发送中
  if(MacSend.send_status == SEND_STATUS_TX_ING)
  {
    MacSend.wait_send_ack_time++;
    if(MacSend.wait_send_ack_time >= SendingMaxTime)
    {    
      //超过发送应答总时间 发送失败
      //MACDBG(printf("MAC: %d AT\r\n",phase););
      MacSend.send_result = MacSendFailure; 
      if (MacSend.send_result_read)
      {
        MacSend.wait_read_time = 0;
        MacSend.send_status = SEND_STATUS_FINISH;        
      }
      else
      {
        MacSend.send_status = SEND_STATUS_IDLE;
      }
    }
    else
    {      
      Phy_GetAttr(PHYA_TX_COMPLETE,1,&tmp);
      //MACDBG(printf("MAC: PhySendConfirm %d\r\n", tmp););
      //读取物理层发送确认标志
      if(PHYSending != tmp)
      {
        //物理层已发送完成读取发送结果
        MacSend.send_result = tmp;  
        if (MacSend.send_result_read)
        {
          MacSend.wait_read_time = 0;
          MacSend.send_status = SEND_STATUS_FINISH;          
        }
        else
        {
          MacSend.send_status = SEND_STATUS_IDLE;
        }
        //MACDBG(printf("MAC: %d SendStatus %d SendResult %d\r\n",phase,mac_send[phase].send_status, mac_send[phase].send_result););
      }
    }
  }  
  
  //发送完成
  if (MacSend.send_status == SEND_STATUS_FINISH)
  {  
    //等待读取反馈
    MacSend.wait_read_time++;
    if(MacSend.wait_read_time >= ReadMaxTime)
    {
      //超过读取反馈总时间 变更为空闲
      //MACDBG(printf("MAC: %d RT\r\n",phase););      
      MacSend.send_status = SEND_STATUS_IDLE;
    }
  }

  //接收物理层数据处理
  if(Phy_RxData())
  {    
    MacRecv.mac_recv_data_ready = 0;
    
    //读取接收到的数据
    Phy_GetData(&MacRecv.mac_recv_buf_len,MacRecv.mac_recv_buf,&MacRecv.mac_recv_rssi);
    
    if(0 == MacRecv.mac_recv_buf_len)
    {
      return;
    }      

    if(MacRecv.mac_recv_buf_len > MacBuffLen)
    {
      return;
    } 
    else
    {
      sMacStopRelayFrame* macstoprelayframe = (sMacStopRelayFrame *)MacRecv.mac_recv_buf;
      if(macstoprelayframe->frame_type == MAC_FRAME_STOPRELAY)
      {  
        //停止转发包直接丢弃 此处只过滤停止转发包 其他数据全部上报
        return;
      }  
    }

    //满足盲中继发停止转发条件立马发送停止转发
    //Mac_Check_StopRelay();    
    
    //置数据有效标志
    MacRecv.mac_recv_data_ready = 1;      
  }

  return;
}

/*——————————————————————————
* 函 数 名：Mac_CCA_Confirm  
* 输入参数：None
* 输出参数：None
* 返 回 值：None
* 功能说明：MAC层接收CCA确认
*——————————————————————————*/
void Mac_CCA_Confirm(u8 status)
{
  if (status == 0)
  {
    //检测空闲立即发送
    MacSend.send_sub_status = SEND_SUB_STATUS_TX;    
  }
  else
  {
    //检测遇忙产生退避时隙    
    MacSend.send_CCA_delay = GetRand(1,MacAttr.mac_BE);        
    //MACDBG(printf("MAC: %d SD %d\r\n",phase,mac_send[phase].send_CCA_delay););
    MacSend.send_sub_status = SEND_SUB_STATUS_WAIT;

    MacAttr.mac_NB++;
    if(MacAttr.mac_NB >= MaxNB)
    {
      //MACDBG(printf("MAC: %d NB\r\n",phase););
      //超过最大次数强制发送
      MacSend.send_sub_status = SEND_SUB_STATUS_TX;      
    }    
  }

  if (MacSend.send_sub_status == SEND_SUB_STATUS_TX)
  {
    u8 phychannel = 1;
    Phy_Data_Request(MacSend.send_buf_len,MacSend.send_buf,phychannel); 

    MacSend.wait_send_ack_time = 0;
    MacSend.send_status = SEND_STATUS_TX_ING;    
    //MACDBG(printf("MAC: %d TX\r\n",phase););
  }
}

/*——————————————————————————
* 函 数 名：Mac_Send  
* 输入参数：handle  句柄
      frametype  帧类型
      txoptions  发送选项  b0 协议版本 b1 读取反馈标志
* 输出参数：None
* 返 回 值：None
* 功能说明：MAC层发送操作
*——————————————————————————*/
static u8 Mac_Send(u8 handle,u8 frametype,u8 txoptions)
{
  MacAttr.mac_NB = 0;
  MacAttr.mac_BE = MaxBE;

  MacSend.send_result = MacSendFailure;
  MacSend.send_result_read = txoptions;  

  if (frametype == MAC_FRAME_STOPRELAY)
  {
    MacSend.send_sub_status = SEND_SUB_STATUS_TX;

    u8 phychannel = 1;
    Phy_Data_Request(MacSend.send_buf_len,MacSend.send_buf,phychannel); 

    MacSend.wait_send_ack_time = 0;
    MacSend.send_status = SEND_STATUS_TX_ING;    
    //MACDBG(printf("MAC: %d TX\r\n",phase););    
  }
  else
  {
    MacSend.send_sub_status = SEND_SUB_STATUS_CCA;

    Phy_CCA_Request();

    MacSend.wait_send_time = 0; 
    MacSend.send_status = SEND_STATUS_START;    
  }  

  return RET_MAC_SUCCESS;
}

/***************MAC层中断任务***************/


/***************MAC层数据服务接口***************/

/*——————————————————————————
* 函 数 名：Mac_StopRelay_Request  
* 输入参数：handle    句柄
      life_perild  生命周期
      flag    全网标识 1 全网      
* 输出参数：None
* 返 回 值：0  成功
      2  数据长度错误
* 功能说明：MAC层停止转发包发送请求
*——————————————————————————*/
u8 Mac_StopRelay_Request(u8 handle,u8 life_perild,u8 flag)
{  
  sMacStopRelayFrame* macstoprelayframe = (sMacStopRelayFrame *)MacSend.stoprelay_buf;

  if(life_perild > 0x3)
  {
    life_perild = 3;
  }

  macstoprelayframe->frame_type = MAC_FRAME_STOPRELAY;
  macstoprelayframe->resv = 0;

  if (flag)
  {
    //全网
    macstoprelayframe->nid = BroadNID;
  }
  else
  {
    macstoprelayframe->nid = MacAttr.mac_NID;
  }

  macstoprelayframe->data = life_perild;  

  MacSend.stoprelay_len = sizeof(sMacStopRelayFrame);    

  MacSend.stoprelay_handle = handle;

  //CCA中断发送停止转发帧 取消打印
  if(MacSend.send_status != SEND_STATUS_IDLE) 
  {
    //DISP(printf("MAC: Phase %d In Sending...\r\n",phase););

    MacSend.follow_send_stoprelay = 1;
    MacSend.send_result_read = OPTION_DEFAULT;//不需要确认
  }
  else
  {
    //DISP(printf("MAC: Stop Relay Request Phase %d\r\n",phase););
    //PrintBuffer(MacSend[phase].stoprelay_buf,MacSend[phase].stoprelay_len);
    
    Mac_Send(handle,MAC_FRAME_STOPRELAY,OPTION_DEFAULT);//不需要确认    
  }

  return RET_MAC_SUCCESS;
}

/*——————————————————————————
* 函 数 名：Mac_Data_Request  
* 输入参数：handle    句柄      
      msdulen    发送数据长度
      msdu    发送数据缓冲区
      txoptions  发送选项  b0 协议版本  1 新版本 0 旧版本
* 输出参数：None
* 返 回 值：0  成功
      2  数据长度错误
* 功能说明：MAC层数据发送请求
*——————————————————————————*/
u8 Mac_Data_Request(u8 handle,u16 msdulen,u8* msdu,u8 txoptions)
{  
  if(msdulen > MaxMSDUSize)
  {    
    return 2;
  }

  MacSend.send_buf_len = msdulen;
  MemCpy(MacSend.send_buf,msdu,msdulen);
  MacSend.dataframe_handle = handle;  

  if(MacSend.send_status != SEND_STATUS_IDLE)
  {
    DISP(printf("MAC: 正在发送中...\r\n"););

    MacSend.follow_send_dataframe = 1;    
    MacSend.send_result_read = OPTION_DEFAULT;//不需要确认
  }
  else
  {    
    DISP(printf("MAC: 数据请求\r\n"););
    PrintBuff(MacSend.send_buf,MacSend.send_buf_len);

    Mac_Send(handle,MAC_FRAME_DATA,OPTION_DEFAULT);
  }

  return RET_MAC_SUCCESS;
  
}

/***************MAC层数据服务接口***************/

