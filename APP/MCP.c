#include "MCP.h"

//u8  gMCP_FT = FT_COMMAND;  //��ǰ��֡����

stSEND_INFO_OF_MCP_DF gInfoOfMultFrame ={0};  //����֡��Ϣ
static u8 gMcpFN = 0;  //�����ϱ���֡���
extern const sVersion Ver;//�汾��Ϣ

/************************************************************************
�������ܣ�����MCPЭ�������Ӧ��֡
����˵������������        ����/���      ˵��
apsFn          in        APS���֡���
appProtocol      in        APS���з��ص�Ӧ�����μ�Ӧ��Э��
1��      addrMode        in        ��ַ����
2��      psourAddr        in        Դ��ַ
3��      signalQ        in        �ź�����
4��      recvPhase        in        ������λ
5��      routerMode      in        ·�ɷ�ʽ
6��      routerLevel      in        �м̼���
7��      puserData        in        �û����ݣ�����Ӧ��Э�飩
8��      userdatalen      in        ����
����ֵ��
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

  //�ж��Ƿ���Ҫ��֡
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
     
    
    //Ӧ��Э��
    pdata[nsize++] = 0;

    //Դ�˿� 
    pdata[nsize++] = 0;
      
    //ԴMAC��ַ����ַ����ת��
    if (pDH->u_AT == AID_LONG)
    { 
      //APS��ص��ز�Դ�ڵ��ַ�ǳ���ַ
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
�������ܣ�MCPЭ�������֡������
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pDBuffer          in        MCPЭ��֡��������
3��      dl            in        MCPЭ��֡�������򳤶�
����ֵ����
************************************************************************/
void Process_CmdFrame(stFCTRL_OF_MCP mFctr,u8 *pDBuffer,u16 dl)
{  
  LPstCMD_DATA_OF_MCP pcmdD=(LPstCMD_DATA_OF_MCP)pDBuffer;

  if (dl<2)
  {  
    MCPDBG(printf("ERROR in Process_CmdFrame:֡����̫��!\r\n"););
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
          //modem��λ
          MCPDBG(printf("MCP::��λ����.\r\n"););
          MCP_Process_FC1_CMD1(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_127:
        {
          //�ļ�����   
          MCPDBG(printf("MCP::�ļ�����.\r\n"););
          MCP_Process_FC1_CMD127(mFctr,pcmdD->pData,dl-2);
        }
        break;
      default:
        {      
          MCPDBG(printf("MCP::δ֪����,FC=%d CMD=%d.\r\n",pcmdD->u_FC,pcmdD->u_CMD););
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
          //���汾 
          MCPDBG(printf("MCP::���汾��Ϣ.\r\n"););
          MCP_Process_FC2_CMD0(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_1:
        {
          //���ô�������   
          MCPDBG(printf("MCP::���ô��ڲ�����.\r\n"););
          MCP_Process_FC2_CMD1(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_2:
        {
          //����������   
          MCPDBG(printf("MCP::�����ڲ�����.\r\n"););
          MCP_Process_FC2_CMD2(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_10:
        {
          //����Mac��ַ  
          MCPDBG(printf("MCP::�������ڵ�MAC��ַ.\r\n"););
          MCP_Process_FC2_CMD10(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_11:
        {
          //��Mac��ַ   
          MCPDBG(printf("MCP::�����ڵ�MAC��ַ.\r\n"););
          MCP_Process_FC2_CMD11(mFctr,pcmdD->pData,dl-2);
        }
        break;         
      case CMD_20:
        {
          //����NID
          MCPDBG(printf("MCP::����NID.\r\n"););  
          MCP_Process_FC2_CMD20(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_21:
        {
          //��NID  
          MCPDBG(printf("MCP::��NID.\r\n"););
          MCP_Process_FC2_CMD21(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_22:
        {
          //��Ӵӽڵ�Mac��ַ  
          MCPDBG(printf("MCP::��Ӵӽڵ�Mac��ַ.\r\n"););
          MCP_Process_FC2_CMD22(mFctr,pcmdD->pData,dl-2);
        }
        break; 
      case CMD_23:
        {
          //��ȡ�ӽڵ�Mac��ַ  
          MCPDBG(printf("MCP::��ȡ�ӽڵ�Mac��ַ.\r\n"););
          MCP_Process_FC2_CMD23(mFctr,pcmdD->pData,dl-2);
        }
        break; 
      case CMD_24:
        {
          //ɾ���ӽڵ�Mac��ַ  
          MCPDBG(printf("MCP::ɾ���ӽڵ�Mac��ַ.\r\n"););
          MCP_Process_FC2_CMD24(mFctr,pcmdD->pData,dl-2);
        }
        break; 
      case CMD_25:
        {
          //��մӽڵ�Mac��ַ  
          MCPDBG(printf("MCP::��մӽڵ�Mac��ַ.\r\n"););
          MCP_Process_FC2_CMD25(mFctr,pcmdD->pData,dl-2);
        }
        break; 
      case CMD_30:
        {
          //����ʱ��   
          MCPDBG(printf("MCP::����ʱ��.\r\n"););
          MCP_Process_FC2_CMD30(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_31:
        {
          //��ʱ��    
          MCPDBG(printf("MCP::��ȡʱ��.\r\n"););
          MCP_Process_FC2_CMD31(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_40:
        {
          //��ȡ�˿��б�   
          MCPDBG(printf("MCP::��ȡ�˿��б�.\r\n"););
          MCP_Process_FC2_CMD40(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_41:
        {
          //���ö˿�   
          MCPDBG(printf("MCP::���ö˿�.\r\n"););
          MCP_Process_FC2_CMD41(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_42:
        {
          //���AID   
          MCPDBG(printf("MCP::���AID.\r\n"););
          MCP_Process_FC2_CMD42(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_43:
        {
          //ɾ��AID   
          MCPDBG(printf("MCP::ɾ��AID.\r\n"););
          MCP_Process_FC2_CMD43(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_44:
        {
          //���AID   
          MCPDBG(printf("MCP::���AID.\r\n"););
          MCP_Process_FC2_CMD44(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_45:
        {
          //��ȡ�˿���Ϣ 
          MCPDBG(printf("MCP::��ȡ�˿���Ϣ.\r\n"););
          MCP_Process_FC2_CMD45(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_100:
        {
          //����ͨ�Ų��� 
          MCPDBG(printf("MCP::����ͨ�Ų���.\r\n"););
          MCP_Process_FC2_CMD100(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_201:
        {
          //����BEʹ�ܻ�ʧ�� 
          MCPDBG(printf("MCP::����BEʹ�ܻ�ʧ��.\r\n"););
          MCP_Process_FC2_CMD201(mFctr,pcmdD->pData,dl-2);
        }
        break;
      default:
        {                           
          MCPDBG(printf("MCP::δ֪����,FC=%d CMD=%d.\r\n",pcmdD->u_FC,pcmdD->u_CMD););
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
          //��������   
          MCPDBG(printf("MCP::��������.\r\n"););
          MCP_Process_FC3_CMD1(mFctr,pcmdD->pData,dl-2);
        }
        break;
      case CMD_2:
        {
          //��ȡ����״̬   
          MCPDBG(printf("MCP::��ȡ����״̬.\r\n"););
          MCP_Process_FC3_CMD2(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_3:
        {
          //��ȡ·����Ϣ  
          MCPDBG(printf("MCP::��ȡ·����Ϣ.\r\n"););
          MCP_Process_FC3_CMD3(mFctr,pcmdD->pData,dl-2);
        }
        break;  
      case CMD_4:
        {
          //ֹͣ����
          MCPDBG(printf("MCP::ֹͣ����.\r\n"););
          MCP_Process_FC3_CMD4(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_5:
        {
          //�ָ�����
          MCPDBG(printf("MCP::�ָ�����.\r\n"););
          MCP_Process_FC3_CMD5(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_6:
        {
          //��ȡ��ʱ������Ϣ
          MCPDBG(printf("MCP::��ȡ��ʱ������Ϣ.\r\n"););
          MCP_Process_FC3_CMD6(mFctr,pcmdD->pData,dl-2);
        }
        break;   
      case CMD_7:
        {
          //���ö�ʱ������Ϣ 
          MCPDBG(printf("MCP::���ö�ʱ������Ϣ.\r\n"););
          MCP_Process_FC3_CMD7(mFctr,pcmdD->pData,dl-2);
        }
        break;    
      case CMD_8:
        {
          //���ö�ʱ������Ϣ 
          MCPDBG(printf("MCP::��δ����APE�б�.\r\n"););
          MCP_Process_FC3_CMD8(mFctr,pcmdD->pData,dl-2);
        }
        break;    
      case CMD_9:
        {
          //���ھ����� 
          MCPDBG(printf("MCP::��δ����APE�б�.\r\n"););
          MCP_Process_FC3_CMD9(mFctr,pcmdD->pData,dl-2);
        }
        break;  
      case CMD_12:
        {
          //��������  
          MCPDBG(printf("MCP::��������.\r\n"););
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
�������ܣ�MCPЭ�������֡������     
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pDBuffer          in        MCPЭ��֡��������
3��      dl            in        MCPЭ��֡�������򳤶�
����ֵ����
************************************************************************/
void Process_DataFrame(stFCTRL_OF_MCP mFctr,u8 *pDBuffer,u16 dl)
{  
  u8 errFlag = 0;
  u8 errID = 0;  
  u16 offset = 0;
  LPstDHEAD_OF_MCP_DFRAME pDH;
  //LPstSendParam psendParam;  
  u16 useDataLen = 0;    

  //�ж�������-����ͷ
  if (dl < sizeof(stDHEAD_OF_MCP_DFRAME))
  {     
    MCPDBG(printf("MCP::�����͵�����֡����С��֡ͷ����!\r\n"););
    errFlag = CONFIRM_NCK;
    errID = ERR_DATAOFCMD;
    goto exit_lable;
  }

  offset = 0;
  pDH = (LPstDHEAD_OF_MCP_DFRAME)(pDBuffer+offset);
  offset++;

  if (pDH->u_MF==MF_SINGLE || (pDH->u_MF==MF_MULTI && pDH->u_DSN==DSN_FIRST))  //��֡����֡
  {
    //��֡��Ϣ�����жϣ�֡ͷ1+����ѡ��2+Э��1+Ŀ��˿�1+Ŀ��MAC��ַ6+�û�����n
    if (dl < (sizeof(stDHEAD_OF_MCP_DFRAME)+sizeof(stSendParam)+8))
    {     
      MCPDBG(printf("MCP::�����͵�����֡����֡��Ϣ����̫��!\r\n"););
      errFlag = CONFIRM_NCK;
      errID = ERR_DATAOFCMD;
      goto exit_lable;
    }

    //����
    //psendParam=(LPstSendParam)(pDBuffer+offset); 
    offset+=sizeof(stSendParam); 
    

    //Ӧ��Э��
    offset++;

    //Ŀ��˿�    
    offset++;

    //Ŀ��ڵ�MAC��ַ    
    offset += 2;

    //�û����� 
    useDataLen = dl-offset;

    //�ж��û��û����ݳ��ȣ����ܴ���APS�����ݻ���
    if (useDataLen > MCP_MAXSENDDATA_LEN)
    {   
      MCPDBG(printf("MCP::��Ҫ�ز����͵����ݴ���APS��Ļ���!\r\n"););
      errFlag = CONFIRM_NCK;
      errID = ERR_OVER_BUFFER;
      goto exit_lable;
    }
    
    //���ݷ���         
    gInfoOfMultFrame.b_PreDSN = pDH->u_DSN; 
    gInfoOfMultFrame.b_MF     = pDH->u_MF;    
    gInfoOfMultFrame.len      = useDataLen;
    MemCpy(gInfoOfMultFrame.buff,pDBuffer+offset,useDataLen);

    if (gInfoOfMultFrame.b_MF == MF_SINGLE)
    {
      //��������
      Mac_Data_Request(0,gInfoOfMultFrame.len,gInfoOfMultFrame.buff,0);
      //����ȷ��֡
      errFlag = CONFIRM_ACK;
      errID = 0;      
      goto exit_lable;
    }
    else
    {    
      //�ǵ�֡������ȷ��֡
      errFlag = CONFIRM_ACK;
      errID = 0;      
      goto exit_lable;
    }
    
  }
  else if (pDH->u_MF == MF_MULTI && pDH->u_DSN == DSN_END) //����֡
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
      MCPDBG(printf("MCP::���������ͣ���֡-����֡����ʧ��.\r\n"););
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
  else //�м�֡
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

          //���·�֡���
          gInfoOfMultFrame.b_PreDSN = pDH->u_DSN;
          SendFlag = TRUE;
        }
      }
    }
    

    if (SendFlag == FALSE)
    {
      MCPDBG(printf("MCP::���������ͣ���֡-�м�֡���ݴ���.\r\n"););
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
�������ܣ�MCPЭ���ȷ��֡������  
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pDBuffer          in        MCPЭ��֡��������
3��      dl            in        MCPЭ��֡�������򳤶�
����ֵ����
************************************************************************/
void Process_ConfirmFrame(stFCTRL_OF_MCP mFctr,u8 *pDBuffer,u16 dl)
{
  return;
}

/************************************************************************
�������ܣ�MCPЭ���Ӧ��֡������       
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pDBuffer          in        MCPЭ��֡��������
3��      dl            in        MCPЭ��֡�������򳤶�
����ֵ����
************************************************************************/
void Process_ReplyFrame(stFCTRL_OF_MCP mFctr,u8 *pDBuffer,u16 dl)
{
  return;
}

/************************************************************************
�������ܣ�MCPЭ���֡������       
����˵������������        ����/���      ˵��
2��      pdata            in        ����USART1���յ�����
3��      nsize            in        ����USART1���յ����ݵĳ���
����ֵ����
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
    MCPDBG(printf("MCP::��MCP֡���յ�����-̫��!\r\n"););
    return;
  }

  p_mcp=(LPstMCP)(pdata+offset);
  if (nsize-offset<sizeof(stMCP)+p_mcp->m_DL.u_dl)
  {    
    MCPDBG(printf("MCP::��MCP֡���յ�����-С�������򳤶�!\r\n"););  
    return;
  }

  if (MCP_FEND != p_mcp->pBuffer[p_mcp->m_DL.u_dl + 1])
  {  
    MCPDBG(printf("MCP::��MCP֡���յ�����-֡β����0x%02x!\r\n",MCP_FEND););    
    return;
  }

  cs=GetSum(pdata+offset+2,p_mcp->m_DL.u_dl + 4);
  if (cs != p_mcp->pBuffer[p_mcp->m_DL.u_dl])
  {  
    MCPDBG(printf("MCP::��MCP֡���յ�����-֡У�����!\r\n"););  
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
�������ܣ�MCPЭ��֡��������
����˵������������        ����/���      ˵��
1��      version        in        MCPЭ��֡�Ŀ�����İ汾��
2��      framesn        in        MCPЭ��֡�����
3��      cnf          in        ȷ�ϱ�־
4��      ft          in        MCPЭ��֡���ͣ�FT_CMD,FT_DATA,FT_CONFIRM,FT_REPLY��
5��      pData          in        MCPЭ��֡������
6��      dl          in        MCPЭ��֡�����򳤶�
����ֵ��
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

  MCPDBG(printf("MCP::���ڡ�%d��Ӧ�����ݡ�%d��\r\n",COM1,2+sizeof(stMCP)+dl););
  PrintBuff(buffer,sizeof(stMCP)+2+dl);

  Uart_Send(COM1,buffer,2+sizeof(stMCP)+dl);
  return 2+sizeof(stMCP)+dl;
}

/************************************************************************
�������ܣ�����MCPЭ���ȷ��֡
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      errflag          in        CONFIRM_NCK(����)��CONFIRM_ACK(ȷ��)
3��      errid            in        �������
����ֵ��
************************************************************************/
u16 MCP_Send_Confirm(stFCTRL_OF_MCP stfctr,u8 errflag,u8 errid)
{
  u8 pdata[2]={0};
  pdata[0]=errflag;
  pdata[1]=errid;

  return MCP_Send(stfctr.u_VerID,stfctr.u_FN,CNF_NO,FT_CONFIRM,pdata,sizeof(pdata));
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��λ����
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_��λ��������ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��0��-1
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
�������ܣ�MCPЭ�������֡������_�ļ�����
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_��λ��������ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��0��-1
************************************************************************/
int MCP_Process_FC1_CMD127(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_���汾
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���汾�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
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
�������ܣ�MCPЭ�������֡������_���ô�������
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���ô������ʵ����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD1(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}
  
/************************************************************************
�������ܣ�MCPЭ�������֡������_����������
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���������ʵ����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD2(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}
  
/************************************************************************
�������ܣ�MCPЭ�������֡������_����Mac��ַ
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_����Mac��ַ�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD10(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}
 
/************************************************************************
�������ܣ�MCPЭ�������֡������_��ȡMac��ַ
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_��ȡMac��ַ�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD11(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_����NID
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD20(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;  
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��ȡNID
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD21(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{    
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��Ӵӽڵ�Mac
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD22(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;  
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��ȡ�ӽڵ��б�
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_��ȡAPE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD23(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_ɾ���ӽڵ��б�
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_ɾ��APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD24(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{ 
  return 0;  
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��մӽڵ�MAC
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD25(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;  
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_����ʱ��
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
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
�������ܣ�MCPЭ�������֡������_��ȡʱ��
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
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
�������ܣ�MCPЭ�������֡������_��ȡ�˿��б�
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���汾�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD40(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_���ö˿�
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���汾�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD41(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_���AID
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���汾�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD42(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_ɾ��AID
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���汾�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD43(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_���AID
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���汾�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD44(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��ȡ�˿���Ϣ
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���汾�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD45(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_�����ز�ͨ�Ų���
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_����Mac��ַ�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD100(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_BEʹ��1��ʧ��0
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_����Mac��ַ�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC2_CMD201(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��������
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD1(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��ȡ����״̬
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD2(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��ȡԴ·����Ϣ
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD3(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��ͣ����
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD4(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_�ָ�����
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD5(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��ȡ��ʱ������Ϣ
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD6(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_���ö�ʱ������Ϣ
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD7(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��ȡδ����APE�б�
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD8(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��ȡ�ھ������б�
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD9(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
�������ܣ�MCPЭ�������֡������_��������
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
************************************************************************/
u16 MCP_Process_FC3_CMD12(stFCTRL_OF_MCP stfctr,u8 *pIdata,u16 nsize)
{  
  return 0;
}

/************************************************************************
�������ܣ��Զ������־
����˵������������        ����/���      ˵��
1��      mFctr            in        MCPЭ��֡�Ŀ�����
2��      pIdata          in        MCPЭ�������֡_���APE�����ݣ�������FC��CMD��
3��      nsize            in        pIdata�ĳ���
����ֵ��������
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


