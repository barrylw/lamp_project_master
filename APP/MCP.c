#include "MCP.h"

//u8  gMCP_FT = FT_COMMAND;  //当前的帧类型

stSEND_INFO_OF_MCP_DF gInfoOfMultFrame ={0};  //数据帧信息
static u8 gMcpFN = 0;  //主动上报的帧序号
extern const sVersion Ver;//版本信息

/************************************************************************
函数功能：发送MCP协议的数据应答帧
参数说明：参数名称        输入/输出      说明
apsFn          in        APS层的帧序号
appProtocol      in        APS层中返回的应用情形及应用协议
1、      addrMode        in        地址类型
2、      psourAddr        in        源地址
3、      signalQ        in        信号质量
4、      recvPhase        in        接收相位
5、      routerMode      in        路由方式
6、      routerLevel      in        中继级数
7、      puserData        in        用户数据（包括应用协议）
8、      userdatalen      in        长度
返回值：
************************************************************************/
u8 Mcp_Data_Indication(u16 len,u8* buff,u8 rssi)
{    
  u8  pdata[MCP_MAXSENDDATA_LEN] ={0};
  u16 nsize = 0;
  u8  countFrame = 0;
  u8  nByte = 0;    
  stFCTRL_OF_MCP stfctr ={0};
  LPstDHEAD_OF_MCP_DFRAME pDH = (LPstDHEAD_OF_MCP_DFRAME)pdata;
  LPstRecvParam precvParam = (LPstRecvParam)(pdata+1);


  countFrame = len/(sizeof(pdata)-sizeof(stMCP)-2);
  nByte = len%(sizeof(pdata)-sizeof(stMCP)-2);

  if (nByte != 0)
  {
    countFrame += 1;
  }

  //判断是否需要分帧
  if (countFrame == 1)
  {
    pDH->u_MF = MF_SINGLE;
    pDH->u_DSN = 0;
    pDH->u_AT = AID_SHORT;
    pDH->uResevedBit= 0;

    precvParam->u_RT = 0; 
    precvParam->u_RSV_B0_b345 = 0;
    precvParam->u_PH = 0;

    precvParam->u_RSSI = rssi;
    precvParam->u_RL = 0;

    precvParam->u_RSV_B2_b4567 = 0;
    precvParam->u_MOD = 0;

    precvParam->u_RSV_B3 = 0;

    nsize=sizeof(stDHEAD_OF_MCP_DFRAME)+sizeof(stRecvParam);
     
    
    //应用协议
    pdata[nsize++] = 0;

    //源端口 
    pdata[nsize++] = 0;
      
    //源MAC地址：地址类型转换
    if (pDH->u_AT == AID_LONG)
    { 
      //APS层回的载波源节点地址是长地址
      pdata[nsize++] = 0;  
      pdata[nsize++] = 0;
      pdata[nsize++] = 0;
      pdata[nsize++] = 0;
      pdata[nsize++] = 0;
      pdata[nsize++] = 0;
    }
    else
    {
      pdata[nsize++] = 0;
      pdata[nsize++] = 0;      
    }    
    
    MemCpy(pdata+nsize,buff,len);
    nsize += len;    

    stfctr.u_FN = gMcpFN++;   
    if (gMcpFN == 16)
    {
      gMcpFN = 0;
    }
    stfctr.u_RSV_B0_b4 = 0;
    stfctr.u_CNF = CNF_NO;
    stfctr.u_FT = FT_DATA;
    stfctr.u_VerID = Ver.VerIDofMcp;
    stfctr.u_RSV_B1_b4567 = 0;

    MCP_Send(stfctr.u_VerID,stfctr.u_FN,stfctr.u_CNF,stfctr.u_FT,pdata,nsize);
  }
  
  return 1;

}

/************************************************************************
函数功能：MCP协议的命令帧处理函数
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pDBuffer          in        MCP协议帧的数据域
3、      dl            in        MCP协议帧的数据域长度
返回值：无
************************************************************************/
void Process_CmdFrame(stFCTRL_OF_MCP mFctr,u8 *pDBuffer,u16 dl)
{  
  LPstCMD_DATA_OF_MCP pcmdD=(LPstCMD_DATA_OF_MCP)pDBuffer;

  if (dl<2)
  {  
    MCPDBG(printf("ERROR in Process_CmdFrame:帧数据太短!\r\n"););
    if (mFctr.u_CNF == CNF_YES)
    {
      MCP_Send_Confirm(mFctr,CONFIRM_NCK,ERR_DATAOFCMD);
    }
    return;
  }

  switch(pcmdD->u_FC)
  {
  case FC_1:
    {
      switch(pcmdD->u_CMD)
      {
      case CMD_1:
        {  
          //modem复位
          MCPDBG(printf("MCP::复位命令.\r\n"););
          MCP_Process_FC1_CMD1(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_127:
        {
          //文件传输   
          MCPDBG(printf("MCP::文件传输.\r\n"););
          MCP_Process_FC1_CMD127(mFctr,pcmdD->pData,dl-2);
        }
        break;
      default:
        {      
          MCPDBG(printf("MCP::未知命令,FC=%d CMD=%d.\r\n",pcmdD->u_FC,pcmdD->u_CMD););
          if (mFctr.u_CNF==CNF_YES)
          {  
            MCP_Send_Confirm(mFctr,CONFIRM_NCK,ERR_INVALID_CMD);
          }
        }
        break;
      }
    }
    break; 
  case FC_2:
    {
      switch(pcmdD->u_CMD)
      {
      case CMD_0:
        {
          //读版本 
          MCPDBG(printf("MCP::读版本信息.\r\n"););
          MCP_Process_FC2_CMD0(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_1:
        {
          //设置串口速率   
          MCPDBG(printf("MCP::设置串口波特率.\r\n"););
          MCP_Process_FC2_CMD1(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_2:
        {
          //读串口速率   
          MCPDBG(printf("MCP::读串口波特率.\r\n"););
          MCP_Process_FC2_CMD2(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_10:
        {
          //设置Mac地址  
          MCPDBG(printf("MCP::设置主节点MAC地址.\r\n"););
          MCP_Process_FC2_CMD10(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_11:
        {
          //读Mac地址   
          MCPDBG(printf("MCP::读主节点MAC地址.\r\n"););
          MCP_Process_FC2_CMD11(mFctr,pcmdD->pData,dl-2);
        }
        break;         
      case CMD_20:
        {
          //设置NID
          MCPDBG(printf("MCP::设置NID.\r\n"););  
          MCP_Process_FC2_CMD20(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_21:
        {
          //读NID  
          MCPDBG(printf("MCP::读NID.\r\n"););
          MCP_Process_FC2_CMD21(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_22:
        {
          //添加从节点Mac地址  
          MCPDBG(printf("MCP::添加从节点Mac地址.\r\n"););
          MCP_Process_FC2_CMD22(mFctr,pcmdD->pData,dl-2);
        }
        break; 
      case CMD_23:
        {
          //获取从节点Mac地址  
          MCPDBG(printf("MCP::获取从节点Mac地址.\r\n"););
          MCP_Process_FC2_CMD23(mFctr,pcmdD->pData,dl-2);
        }
        break; 
      case CMD_24:
        {
          //删除从节点Mac地址  
          MCPDBG(printf("MCP::删除从节点Mac地址.\r\n"););
          MCP_Process_FC2_CMD24(mFctr,pcmdD->pData,dl-2);
        }
        break; 
      case CMD_25:
        {
          //清空从节点Mac地址  
          MCPDBG(printf("MCP::清空从节点Mac地址.\r\n"););
          MCP_Process_FC2_CMD25(mFctr,pcmdD->pData,dl-2);
        }
        break; 
      case CMD_30:
        {
          //设置时间   
          MCPDBG(printf("MCP::设置时间.\r\n"););
          MCP_Process_FC2_CMD30(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_31:
        {
          //读时间    
          MCPDBG(printf("MCP::读取时间.\r\n"););
          MCP_Process_FC2_CMD31(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_40:
        {
          //获取端口列表   
          MCPDBG(printf("MCP::获取端口列表.\r\n"););
          MCP_Process_FC2_CMD40(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_41:
        {
          //设置端口   
          MCPDBG(printf("MCP::设置端口.\r\n"););
          MCP_Process_FC2_CMD41(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_42:
        {
          //添加AID   
          MCPDBG(printf("MCP::添加AID.\r\n"););
          MCP_Process_FC2_CMD42(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_43:
        {
          //删除AID   
          MCPDBG(printf("MCP::删除AID.\r\n"););
          MCP_Process_FC2_CMD43(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_44:
        {
          //清空AID   
          MCPDBG(printf("MCP::清空AID.\r\n"););
          MCP_Process_FC2_CMD44(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_45:
        {
          //获取端口信息 
          MCPDBG(printf("MCP::获取端口信息.\r\n"););
          MCP_Process_FC2_CMD45(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_100:
        {
          //设置通信参数 
          MCPDBG(printf("MCP::设置通信参数.\r\n"););
          MCP_Process_FC2_CMD100(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_201:
        {
          //设置BE使能或失能 
          MCPDBG(printf("MCP::设置BE使能或失能.\r\n"););
          MCP_Process_FC2_CMD201(mFctr,pcmdD->pData,dl-2);
        }
        break;
      default:
        {                           
          MCPDBG(printf("MCP::未知命令,FC=%d CMD=%d.\r\n",pcmdD->u_FC,pcmdD->u_CMD););
          if (mFctr.u_CNF==CNF_YES)
          {  
            MCP_Send_Confirm(mFctr,CONFIRM_NCK,ERR_INVALID_CMD);
          }
        }
        break;
      }
    }
    break;
  case FC_3:
    { 
      switch(pcmdD->u_CMD)
      {
      case CMD_1:
        {
          //启动组网   
          MCPDBG(printf("MCP::启动组网.\r\n"););
          MCP_Process_FC3_CMD1(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_2:
        {
          //读取组网状态   
          MCPDBG(printf("MCP::读取组网状态.\r\n"););
          MCP_Process_FC3_CMD2(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_3:
        {
          //读取路由信息  
          MCPDBG(printf("MCP::读取路由信息.\r\n"););
          MCP_Process_FC3_CMD3(mFctr,pcmdD->pData,dl-2);
        }
        break;  
      case CMD_4:
        {
          //停止组网
          MCPDBG(printf("MCP::停止组网.\r\n"););
          MCP_Process_FC3_CMD4(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_5:
        {
          //恢复组网
          MCPDBG(printf("MCP::恢复组网.\r\n"););
          MCP_Process_FC3_CMD5(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_6:
        {
          //读取定时组网信息
          MCPDBG(printf("MCP::读取定时组网信息.\r\n"););
          MCP_Process_FC3_CMD6(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_7:
        {
          //设置定时组网信息 
          MCPDBG(printf("MCP::设置定时组网信息.\r\n"););
          MCP_Process_FC3_CMD7(mFctr,pcmdD->pData,dl-2);
        }
        break;    
      case CMD_8:
        {
          //设置定时组网信息 
          MCPDBG(printf("MCP::读未组网APE列表.\r\n"););
          MCP_Process_FC3_CMD8(mFctr,pcmdD->pData,dl-2);
        }
        break;    
      case CMD_9:
        {
          //读邻居网络 
          MCPDBG(printf("MCP::读未组网APE列表.\r\n"););
          MCP_Process_FC3_CMD9(mFctr,pcmdD->pData,dl-2);
        }
        break;  
      case CMD_12:
        {
          //入网请求  
          MCPDBG(printf("MCP::入网请求.\r\n"););
          MCP_Process_FC3_CMD12(mFctr,pcmdD->pData,dl-2);
        }
        break;
      default:
        { 
          if (mFctr.u_CNF==CNF_YES)
          {  
            MCP_Send_Confirm(mFctr,CONFIRM_NCK,ERR_INVALID_CMD);
          }
        }
        break;
      }
    }
    break;
  case FC_FF:
    {
      switch(pcmdD->u_CMD)
      {  
      case CMD_FF:
        { 
          MCP_Process_FCFF_CMDFF(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_FE:
        { 
          MCP_Process_FCFF_CMDFE(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_FD:
        {  
          MCP_Process_FCFF_CMDFD(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_FC:
        { 
          MCP_Process_FCFF_CMDFC(mFctr,pcmdD->pData,dl-2);
        }
        break;
      default:
        {  
          if (mFctr.u_CNF==CNF_YES)
          {  
            MCP_Send_Confirm(mFctr,CONFIRM_NCK,ERR_INVALID_CMD);
          }
        }
      }
    }
    break;
  default:
    { 
      if (mFctr.u_CNF==CNF_YES)
      {  
        MCP_Send_Confirm(mFctr,CONFIRM_NCK,ERR_INVALID_CMD);
      }
    }
    break;
  }
  return;
}

/************************************************************************
函数功能：MCP协议的数据帧处理函数     
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pDBuffer          in        MCP协议帧的数据域
3、      dl            in        MCP协议帧的数据域长度
返回值：无
************************************************************************/
void Process_DataFrame(stFCTRL_OF_MCP mFctr,u8 *pDBuffer,u16 dl)
{  
  u8 errFlag = 0;
  u8 errID = 0;  
  u16 offset = 0;
  LPstDHEAD_OF_MCP_DFRAME pDH;
  //LPstSendParam psendParam;  
  u16 useDataLen = 0;    

  //判断数据域-数据头
  if (dl < sizeof(stDHEAD_OF_MCP_DFRAME))
  {     
    MCPDBG(printf("MCP::请求发送的数据帧长度小于帧头长度!\r\n"););
    errFlag = CONFIRM_NCK;
    errID = ERR_DATAOFCMD;
    goto exit_lable;
  }

  offset = 0;
  pDH = (LPstDHEAD_OF_MCP_DFRAME)(pDBuffer+offset);
  offset++;

  if (pDH->u_MF==MF_SINGLE || (pDH->u_MF==MF_MULTI && pDH->u_DSN==DSN_FIRST))  //首帧、单帧
  {
    //首帧信息长度判断：帧头1+发送选项2+协议1+目标端口1+目标MAC地址6+用户数据n
    if (dl < (sizeof(stDHEAD_OF_MCP_DFRAME)+sizeof(stSendParam)+8))
    {     
      MCPDBG(printf("MCP::请求发送的数据帧的首帧信息长度太短!\r\n"););
      errFlag = CONFIRM_NCK;
      errID = ERR_DATAOFCMD;
      goto exit_lable;
    }

    //发送
    //psendParam=(LPstSendParam)(pDBuffer+offset); 
    offset+=sizeof(stSendParam); 
    

    //应用协议
    offset++;

    //目标端口    
    offset++;

    //目标节点MAC地址    
    offset += 2;

    //用户数据 
    useDataLen = dl-offset;

    //判断用户用户数据长度：不能大于APS层数据缓存
    if (useDataLen > MCP_MAXSENDDATA_LEN)
    {   
      MCPDBG(printf("MCP::需要载波发送的数据大于APS层的缓存!\r\n"););
      errFlag = CONFIRM_NCK;
      errID = ERR_OVER_BUFFER;
      goto exit_lable;
    }
    
    //数据发送         
    gInfoOfMultFrame.b_PreDSN = pDH->u_DSN; 
    gInfoOfMultFrame.b_MF     = pDH->u_MF;    
    gInfoOfMultFrame.len      = useDataLen;
    MemCpy(gInfoOfMultFrame.buff,pDBuffer+offset,useDataLen);

    if (gInfoOfMultFrame.b_MF == MF_SINGLE)
    {
      //数据请求
      Mac_Data_Request(0,gInfoOfMultFrame.len,gInfoOfMultFrame.buff,0);
      //返回确认帧
      errFlag = CONFIRM_ACK;
      errID = 0;      
      goto exit_lable;
    }
    else
    {    
      //非单帧，返回确认帧
      errFlag = CONFIRM_ACK;
      errID = 0;      
      goto exit_lable;
    }
    
  }
  else if (pDH->u_MF == MF_MULTI && pDH->u_DSN == DSN_END) //结束帧
  {  
    bool SendFlag = FALSE;
    useDataLen = dl-offset;    
    
    if (gInfoOfMultFrame.b_MF == MF_MULTI)
    {  
      if (gInfoOfMultFrame.len+useDataLen <= sizeof(gInfoOfMultFrame.buff))
      {  
        MemCpy(gInfoOfMultFrame.buff+gInfoOfMultFrame.len,pDBuffer+offset,useDataLen); 
        gInfoOfMultFrame.len += useDataLen;         
        
        Mac_Data_Request(0,gInfoOfMultFrame.len,gInfoOfMultFrame.buff,0);

        SendFlag = TRUE;        
      }
    }
    

    if (SendFlag == FALSE)
    {
      MCPDBG(printf("MCP::数据请求发送，多帧-结束帧发送失败.\r\n"););
      errFlag = CONFIRM_NCK;
      errID = ERR_SEND_FAILED;
      goto exit_lable;
    }
    else
    {
      errFlag = CONFIRM_ACK;
      errID = 0;      
      goto exit_lable;
    }
  }
  else //中间帧
  {
    bool SendFlag = FALSE;
    useDataLen = dl-offset;
    
    if (gInfoOfMultFrame.b_MF == MF_MULTI)
    {  
      if ((gInfoOfMultFrame.b_PreDSN+1 == pDH->u_DSN) ||(gInfoOfMultFrame.b_PreDSN==6 && pDH->u_DSN==1))
      {
        if (gInfoOfMultFrame.len+useDataLen <= sizeof(gInfoOfMultFrame.buff))
        {
          MemCpy(gInfoOfMultFrame.buff+gInfoOfMultFrame.len,pDBuffer+offset,useDataLen); 
          gInfoOfMultFrame.len += useDataLen; 

          //更新分帧序号
          gInfoOfMultFrame.b_PreDSN = pDH->u_DSN;
          SendFlag = TRUE;
        }
      }
    }
    

    if (SendFlag == FALSE)
    {
      MCPDBG(printf("MCP::数据请求发送，多帧-中间帧数据错误.\r\n"););
      errFlag = CONFIRM_NCK;
      errID = ERR_DATAOFCMD;
      goto exit_lable;
    }
    else
    {
      errFlag = CONFIRM_ACK;
      errID = 0;
      goto exit_lable;
    }
  }

exit_lable:  
  if (mFctr.u_CNF == CNF_YES)
  {
    MCP_Send_Confirm(mFctr,errFlag,errID);     
  }

  return;
}

/************************************************************************
函数功能：MCP协议的确认帧处理函数  
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pDBuffer          in        MCP协议帧的数据域
3、      dl            in        MCP协议帧的数据域长度
返回值：无
************************************************************************/
void Process_ConfirmFrame(stFCTRL_OF_MCP mFctr,u8 *pDBuffer,u16 dl)
{
  return;
}

/************************************************************************
函数功能：MCP协议的应答帧处理函数       
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pDBuffer          in        MCP协议帧的数据域
3、      dl            in        MCP协议帧的数据域长度
返回值：无
************************************************************************/
void Process_ReplyFrame(stFCTRL_OF_MCP mFctr,u8 *pDBuffer,u16 dl)
{
  return;
}

/************************************************************************
函数功能：MCP协议的帧处理函数       
参数说明：参数名称        输入/输出      说明
2、      pdata            in        串口USART1接收的数据
3、      nsize            in        串口USART1接收的数据的长度
返回值：无
************************************************************************/
void MCP_Process(u8 *pdata,u16 nsize)
{
  u8 cs=0;
  int offset=0;
  LPstMCP p_mcp;

  
  offset=0;
  while(offset<nsize-1)
  {
    if (MCP_FSTART_1==pdata[offset] && MCP_FSTART_2==pdata[offset+1])
    {
      break;
    }
    offset++;
  }

  if (nsize-offset<sizeof(stMCP))
  {         
    MCPDBG(printf("MCP::按MCP帧接收的数据-太短!\r\n"););
    return;
  }

  p_mcp=(LPstMCP)(pdata+offset);
  if (nsize-offset<sizeof(stMCP)+p_mcp->m_DL.u_dl)
  {    
    MCPDBG(printf("MCP::按MCP帧接收的数据-小于数据域长度!\r\n"););  
    return;
  }

  if (MCP_FEND != p_mcp->pBuffer[p_mcp->m_DL.u_dl + 1])
  {  
    MCPDBG(printf("MCP::按MCP帧接收的数据-帧尾不是0x%02x!\r\n",MCP_FEND););    
    return;
  }

  cs=GetSum(pdata+offset+2,p_mcp->m_DL.u_dl + 4);
  if (cs != p_mcp->pBuffer[p_mcp->m_DL.u_dl])
  {  
    MCPDBG(printf("MCP::按MCP帧接收的数据-帧校验错误!\r\n"););  
    return;
  }  
  
  
  switch(p_mcp->m_Fctrl.u_FT)
  {
  case FT_COMMAND:
    {
      //gMCP_FT = FT_COMMAND;
      Process_CmdFrame(p_mcp->m_Fctrl,p_mcp->pBuffer,p_mcp->m_DL.u_dl);
    }
    break; 
  case FT_DATA:
    { 
      //gMCP_FT = FT_DATA;
      Process_DataFrame(p_mcp->m_Fctrl,p_mcp->pBuffer,p_mcp->m_DL.u_dl);
    }
    break;
  case FT_CONFIRM:
    { 
      //gMCP_FT = FT_CONFIRM;
      Process_ConfirmFrame(p_mcp->m_Fctrl,p_mcp->pBuffer,p_mcp->m_DL.u_dl);
    }
    break;
  case FT_REPLY:
    { 
      //gMCP_FT = FT_REPLY;
      Process_ReplyFrame(p_mcp->m_Fctrl,p_mcp->pBuffer,p_mcp->m_DL.u_dl);
    }
    break;
  default:
    {
      if (p_mcp->m_Fctrl.u_CNF == CNF_YES)
      {  
        MCP_Send_Confirm(p_mcp->m_Fctrl,CONFIRM_NCK,ERR_INVALID_CMD);
      }
    }
    break;
  }
  return;
}

/************************************************************************
函数功能：MCP协议帧的物理发送
参数说明：参数名称        输入/输出      说明
1、      version        in        MCP协议帧的控制域的版本号
2、      framesn        in        MCP协议帧真序号
3、      cnf          in        确认标志
4、      ft          in        MCP协议帧类型（FT_CMD,FT_DATA,FT_CONFIRM,FT_REPLY）
5、      pData          in        MCP协议帧数据域
6、      dl          in        MCP协议帧数据域长度
返回值：
************************************************************************/
u16 MCP_Send(u8 version,u8 framesn,u8 cnf,unsigned ft,u8 *pData,u16 dl)
{
  u8 buffer[MCP_MAXSENDDATA_LEN]={0};
  u8 cs=0;
  LPstMCP pstmcp=(LPstMCP)(buffer+2);

  buffer[0]=MCP_FSTART_1;
  buffer[1]=MCP_FSTART_1;

  pstmcp->u_Fstart1=MCP_FSTART_1;  
  pstmcp->u_Fstart2=MCP_FSTART_2;

  pstmcp->m_Fctrl.u_VerID=version;
  pstmcp->m_Fctrl.u_FN=framesn;
  pstmcp->m_Fctrl.u_CNF=cnf;
  pstmcp->m_Fctrl.u_FT=ft;

  pstmcp->m_DL.u_dl=dl;
  pstmcp->m_DL.u_RSV_B1_b1_7=0;
  if (dl>0)
  {
    MemCpy(pstmcp->pBuffer,pData,dl);
  }

  cs=GetSum((u8*)(&(pstmcp->m_Fctrl)),dl+4);  
  pstmcp->pBuffer[dl]=cs;
  pstmcp->pBuffer[dl+1]=MCP_FEND;

  MCPDBG(printf("MCP::串口【%d】应答数据【%d】\r\n",COM1,2+sizeof(stMCP)+dl););
  PrintBuff(buffer,sizeof(stMCP)+2+dl);

  Uart_Send(COM1,buffer,2+sizeof(stMCP)+dl);
  return 2+sizeof(stMCP)+dl;
}

/************************************************************************
函数功能：发送MCP协议的确认帧
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      errflag          in        CONFIRM_NCK(否认)、CONFIRM_ACK(确认)
3、      errid            in        错误代码
返回值：
************************************************************************/
u16 MCP_Send_Confirm(stFCTRL_OF_MCP stfctr,u8 errflag,u8 errid)
{
  u8 pdata[2]={0};
  pdata[0]=errflag;
  pdata[1]=errid;

  return MCP_Send(stfctr.u_VerID,stfctr.u_FN,CNF_NO,FT_CONFIRM,pdata,sizeof(pdata));
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_复位命令
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_复位命令的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：0、-1
************************************************************************/
u16 MCP_Process_FC1_CMD1(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  if (nsize<1)
  {
    if (stfctr.u_CNF == CNF_YES)
    {
      MCP_Send_Confirm(stfctr,CONFIRM_NCK,ERR_DATAOFCMD);
    }
    return 0;
  }
  if (pIdata[0]!=1 && pIdata[0]!=2)
  {  
    if (stfctr.u_CNF == CNF_YES)
    {
      MCP_Send_Confirm(stfctr,CONFIRM_NCK,ERR_INVALID_DATA);
    }
    return 0;
  }

  if (stfctr.u_CNF==CNF_YES)
  {
    MCP_Send_Confirm(stfctr,CONFIRM_ACK,2);  
    Delay_Nop(0x5fffff);
  } 

  //gSysResetFlag = TRUE;
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_文件传输
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_复位命令的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：0、-1
************************************************************************/
int MCP_Process_FC1_CMD127(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_读版本
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读版本的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD0(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  u8 pdata[20]={0}; 
  int offset=0;

  pdata[offset++]=FC_2;
  pdata[offset++]=CMD_0;
  pdata[offset++]=Ver.VenderID[0];
  pdata[offset++]=Ver.VenderID[1];
  pdata[offset++]=Ver.VerIDofMcp;
  pdata[offset++]=Ver.VerIDofSoft;
  pdata[offset++]=(Ver.Year)&0xff;
  pdata[offset++]=(Ver.Year)>>8;
  pdata[offset++]=Ver.Mon;
  pdata[offset++]=Ver.Day;
  pdata[offset++]=Ver.NodeType;

  return MCP_Send(stfctr.u_VerID,stfctr.u_FN,CNF_NO,FT_REPLY,pdata,offset);
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_设置串口速率
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_设置串口速率的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD1(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}
  
/************************************************************************
函数功能：MCP协议的命令帧处理函数_读串口速率
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读串口速率的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD2(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}
  
/************************************************************************
函数功能：MCP协议的命令帧处理函数_设置Mac地址
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_设置Mac地址的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD10(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}
 
/************************************************************************
函数功能：MCP协议的命令帧处理函数_读取Mac地址
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读取Mac地址的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD11(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_设置NID
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD20(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;  
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_读取NID
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD21(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{    
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_添加从节点Mac
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_添加APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD22(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;  
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_读取从节点列表
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读取APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD23(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_删除从节点列表
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_删除APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD24(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{ 
  return 0;  
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_清空从节点MAC
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD25(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;  
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_设置时间
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD30(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{   
  u8 offset = 0;
  u8 year = 0;
  u8 month = 0;
  u8 day = 0;
  u8 hour = 0;
  u8 min = 0;
  u8 sec = 0;
  u8 errID=CONFIRM_ACK;
  u8 errFlag=0;

  if (nsize<6)
  {  
    if (stfctr.u_CNF==CNF_YES)
    {  
      return MCP_Send_Confirm(stfctr,CONFIRM_NCK,ERR_DATAOFCMD);
    } 
    else
    {
      return 0;
    }
  }

  sec  = pIdata[offset++];
  sec = BCDToHex(sec);
  if (sec==0xff)
  {
    errFlag=CONFIRM_NCK;
    errID=ERR_INVALID_DATA;
    goto exit_lable;
  }

  min = pIdata[offset++]; 
  min = BCDToHex(min); 
  if (min==0xff)
  {
    errFlag=CONFIRM_NCK;
    errID=ERR_INVALID_DATA;
    goto exit_lable;
  }

  hour = pIdata[offset++]; 
  hour = BCDToHex(hour); 
  if (hour==0xff)
  {
    errFlag=CONFIRM_NCK;
    errID=ERR_INVALID_DATA;
    goto exit_lable;
  }

  day = pIdata[offset++]; 
  day = BCDToHex(day); 
  if (day==0xff)
  {
    errFlag=CONFIRM_NCK;
    errID=ERR_INVALID_DATA;
    goto exit_lable;
  }

  month = pIdata[offset++]; 
  month = BCDToHex(month); 
  if (month==0xff)
  {
    errFlag=CONFIRM_NCK;
    errID=ERR_INVALID_DATA;
    goto exit_lable;
  }

  year = pIdata[offset++]; 
  year = BCDToHex(year); 
  if (year==0xff)
  {
    errFlag=CONFIRM_NCK;
    errID=ERR_INVALID_DATA;
    goto exit_lable;
  }

  Time_Set(year, month, day, hour, min, sec);

exit_lable:
  if (stfctr.u_CNF==CNF_YES)
  {  
    return MCP_Send_Confirm(stfctr,errFlag,errID);
  } 
  else
  {
    return 0;
  }
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_读取时间
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD31(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  u8 pdata[8]={0}; 
  u16 offset=0;

  pdata[offset++] = FC_2;
  pdata[offset++] = CMD_31;   
  //pdata[offset++] = HexToBCD(TimeRegs.second);    
  //pdata[offset++] = HexToBCD(TimeRegs.minute);
  //pdata[offset++] = HexToBCD(TimeRegs.hour);
  //pdata[offset++] = HexToBCD(TimeRegs.day);
  //pdata[offset++] = HexToBCD(TimeRegs.month);
  //pdata[offset++] = HexToBCD(TimeRegs.year);

  return MCP_Send(stfctr.u_VerID,stfctr.u_FN,CNF_NO,FT_REPLY,pdata,offset);
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_获取端口列表
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读版本的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD40(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_设置端口
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读版本的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD41(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_添加AID
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读版本的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD42(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_删除AID
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读版本的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD43(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_清空AID
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读版本的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD44(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_获取端口信息
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_读版本的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD45(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_设置载波通信参数
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_设置Mac地址的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD100(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_BE使能1或失能0
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_设置Mac地址的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC2_CMD201(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_启动组网
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD1(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_读取组网状态
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD2(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_读取源路由信息
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD3(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_暂停组网
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD4(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_恢复组网
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD5(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_读取定时组网信息
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD6(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_设置定时组网信息
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD7(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_读取未组网APE列表
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD8(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_读取邻居网络列表
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD9(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
函数功能：MCP协议的命令帧处理函数_入网请求
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FC3_CMD12(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
函数功能：自定义读日志
参数说明：参数名称        输入/输出      说明
1、      mFctr            in        MCP协议帧的控制域
2、      pIdata          in        MCP协议的命令帧_清空APE的数据（不包括FC、CMD）
3、      nsize            in        pIdata的长度
返回值：无意义
************************************************************************/
u16 MCP_Process_FCFF_CMDFC(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{     
  return 0;
}

u16 MCP_Process_FCFF_CMDFD(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{    
  return 0;
}

u16 MCP_Process_FCFF_CMDFE(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{    
  return 0;
}

u16 MCP_Process_FCFF_CMDFF(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{    
  return 0;
}


