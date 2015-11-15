/*����������������������������������������������������
* �� �� ����MAC.c  
* ��    �ߣ�����
* ��    �ڣ�2014-04-06
* �ļ�˵����MAC�����ļ�
*����������������������������������������������������*/
#include "MAC.h"
#include "PHY.h"


/***************MAC�����***************/
sMacAttr   MacAttr;      //MAC������
sMacRecv   MacRecv;      //MAC���������
sMacSend   MacSend;      //MAC�㷢������
/***************MAC�����***************/


/***************MAC���ⲿ����***************/

/***************MAC���ⲿ����***************/


/***************MAC�㺯��***************/
static void Mac_Vari_Init(void);
static void Mac_Tick(void);
static u8   Mac_Send(u8 handle,u8 frametype,u8 txoptions);
/***************MAC�㺯��***************/


/***************MAC���ʼ��***************/

/*����������������������������������������������������
* �� �� ����Mac_Init  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����MAC���ʼ��
*����������������������������������������������������*/
void Mac_Init(void)
{
  //��ʼ��
  Mac_Vari_Init();  

  MACDBG(printf("MAC: ��ʼ�����\r\n"););

  return;  
}

/*����������������������������������������������������
* �� �� ����Mac_Vari_Init  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����MAC����س�ʼ��
*����������������������������������������������������*/
static void Mac_Vari_Init(void)
{  
  u16 NID = 0;  
  
  MemSet((u8 *)&MacAttr,0,sizeof(sMacAttr));
  MemSet((u8 *)&MacRecv,0,sizeof(sMacRecv));  
  MemSet((u8 *)&MacSend,0,sizeof(sMacSend));        
  
  MacAttr.mac_NID = NID;  
  
  return;
}

/***************MAC���ʼ��***************/


/***************MAC����������***************/

/*����������������������������������������������������
* �� �� ����Mac_Proc  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����MAC����������
*����������������������������������������������������*/
void Mac_Proc(void)
{  
  //MAC����
  Mac_Tick();

  return;
}

/*����������������������������������������������������
* �� �� ����Mac_Tick  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����MAC������
*����������������������������������������������������*/
static void Mac_Tick(void)
{
  if (MacSend.send_status == SEND_STATUS_IDLE)  
  {
    if(MacSend.follow_send_stoprelay == 1)
    {
      DISP(printf("MAC: ��һ�η���ֹͣת��\r\n"););
      MacSend.follow_send_stoprelay = 0;      
      Mac_Send(MacSend.stoprelay_handle,MAC_FRAME_STOPRELAY,MacSend.send_result_read);

    }
    else if(MacSend.follow_send_dataframe == 1)
    {
      DISP(printf("MAC: ��һ�η�������\r\n"););
      MacSend.follow_send_dataframe = 0;
      Mac_Send(MacSend.dataframe_handle,MAC_FRAME_DATA,MacSend.send_result_read);      
    }
  }

  if(MacRecv.mac_recv_data_ready == 1)      
  {  
    sMacRecv mac_recv_temp;

    MacRecv.mac_recv_data_ready = 0;  
    
    //���������ݿ�������ʱ���������ϴ� ��ֹ�ϴ�����δ����������жϽ��ո���
    MemCpy(&mac_recv_temp,&MacRecv,sizeof(sMacRecv));
    
    MACDBG(printf("MAC: �յ�����\r\n"););
    PrintBuff(mac_recv_temp.mac_recv_buf,mac_recv_temp.mac_recv_buf_len);  
    
    //���ݽ���MCP�ϱ�
    Mcp_Data_Indication(mac_recv_temp.mac_recv_buf_len,mac_recv_temp.mac_recv_buf,mac_recv_temp.mac_recv_rssi);  
  }

  return;
}

/***************MAC����������***************/


/***************MAC���ж�����***************/

/*����������������������������������������������������
* �� �� ����Mac_CCA_Proc  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����MAC��CCA�ж�����
*����������������������������������������������������*/
void Mac_CCA_Proc(void)
{  
  u8 tmp;
  
  //���ݷ�������
  if (MacSend.send_status == SEND_STATUS_START)
  {
    MacSend.wait_send_time++;
    if(MacSend.wait_send_time >= WaitSendMaxTime)
    {    
      //�����ȴ�������ʱ�� ����ʧ��
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

    //������״̬
    switch (MacSend.send_sub_status)
    {
    case SEND_SUB_STATUS_TX:  
      //������
      break;

    case SEND_SUB_STATUS_CCA:      
      //������            
      break;

    case SEND_SUB_STATUS_WAIT:
      //�ȴ�ʱ϶
      if(MacSend.send_CCA_delay != 0)    
      {
        MacSend.send_CCA_delay--;
      }

      if(0 == MacSend.send_CCA_delay)
      {  
        //�ȴ�ʱ϶�� �ٴη���CCA����  
        MacSend.send_sub_status = SEND_SUB_STATUS_CCA;
        Phy_CCA_Request();        
      }      
      break;
      
    default:
      break;
    }    
  }

  //���ݷ�����
  if(MacSend.send_status == SEND_STATUS_TX_ING)
  {
    MacSend.wait_send_ack_time++;
    if(MacSend.wait_send_ack_time >= SendingMaxTime)
    {    
      //��������Ӧ����ʱ�� ����ʧ��
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
      //��ȡ����㷢��ȷ�ϱ�־
      if(PHYSending != tmp)
      {
        //������ѷ�����ɶ�ȡ���ͽ��
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
  
  //�������
  if (MacSend.send_status == SEND_STATUS_FINISH)
  {  
    //�ȴ���ȡ����
    MacSend.wait_read_time++;
    if(MacSend.wait_read_time >= ReadMaxTime)
    {
      //������ȡ������ʱ�� ���Ϊ����
      //MACDBG(printf("MAC: %d RT\r\n",phase););      
      MacSend.send_status = SEND_STATUS_IDLE;
    }
  }

  //������������ݴ���
  if(Phy_RxData())
  {    
    MacRecv.mac_recv_data_ready = 0;
    
    //��ȡ���յ�������
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
        //ֹͣת����ֱ�Ӷ��� �˴�ֻ����ֹͣת���� ��������ȫ���ϱ�
        return;
      }  
    }

    //����ä�м̷�ֹͣת������������ֹͣת��
    //Mac_Check_StopRelay();    
    
    //��������Ч��־
    MacRecv.mac_recv_data_ready = 1;      
  }

  return;
}

/*����������������������������������������������������
* �� �� ����Mac_CCA_Confirm  
* ���������None
* ���������None
* �� �� ֵ��None
* ����˵����MAC�����CCAȷ��
*����������������������������������������������������*/
void Mac_CCA_Confirm(u8 status)
{
  if (status == 0)
  {
    //��������������
    MacSend.send_sub_status = SEND_SUB_STATUS_TX;    
  }
  else
  {
    //�����æ�����˱�ʱ϶    
    MacSend.send_CCA_delay = GetRand(1,MacAttr.mac_BE);        
    //MACDBG(printf("MAC: %d SD %d\r\n",phase,mac_send[phase].send_CCA_delay););
    MacSend.send_sub_status = SEND_SUB_STATUS_WAIT;

    MacAttr.mac_NB++;
    if(MacAttr.mac_NB >= MaxNB)
    {
      //MACDBG(printf("MAC: %d NB\r\n",phase););
      //����������ǿ�Ʒ���
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

/*����������������������������������������������������
* �� �� ����Mac_Send  
* ���������handle  ���
      frametype  ֡����
      txoptions  ����ѡ��  b0 Э��汾 b1 ��ȡ������־
* ���������None
* �� �� ֵ��None
* ����˵����MAC�㷢�Ͳ���
*����������������������������������������������������*/
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

/***************MAC���ж�����***************/


/***************MAC�����ݷ���ӿ�***************/

/*����������������������������������������������������
* �� �� ����Mac_StopRelay_Request  
* ���������handle    ���
      life_perild  ��������
      flag    ȫ����ʶ 1 ȫ��      
* ���������None
* �� �� ֵ��0  �ɹ�
      2  ���ݳ��ȴ���
* ����˵����MAC��ֹͣת������������
*����������������������������������������������������*/
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
    //ȫ��
    macstoprelayframe->nid = BroadNID;
  }
  else
  {
    macstoprelayframe->nid = MacAttr.mac_NID;
  }

  macstoprelayframe->data = life_perild;  

  MacSend.stoprelay_len = sizeof(sMacStopRelayFrame);    

  MacSend.stoprelay_handle = handle;

  //CCA�жϷ���ֹͣת��֡ ȡ����ӡ
  if(MacSend.send_status != SEND_STATUS_IDLE) 
  {
    //DISP(printf("MAC: Phase %d In Sending...\r\n",phase););

    MacSend.follow_send_stoprelay = 1;
    MacSend.send_result_read = OPTION_DEFAULT;//����Ҫȷ��
  }
  else
  {
    //DISP(printf("MAC: Stop Relay Request Phase %d\r\n",phase););
    //PrintBuffer(MacSend[phase].stoprelay_buf,MacSend[phase].stoprelay_len);
    
    Mac_Send(handle,MAC_FRAME_STOPRELAY,OPTION_DEFAULT);//����Ҫȷ��    
  }

  return RET_MAC_SUCCESS;
}

/*����������������������������������������������������
* �� �� ����Mac_Data_Request  
* ���������handle    ���      
      msdulen    �������ݳ���
      msdu    �������ݻ�����
      txoptions  ����ѡ��  b0 Э��汾  1 �°汾 0 �ɰ汾
* ���������None
* �� �� ֵ��0  �ɹ�
      2  ���ݳ��ȴ���
* ����˵����MAC�����ݷ�������
*����������������������������������������������������*/
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
    DISP(printf("MAC: ���ڷ�����...\r\n"););

    MacSend.follow_send_dataframe = 1;    
    MacSend.send_result_read = OPTION_DEFAULT;//����Ҫȷ��
  }
  else
  {    
    DISP(printf("MAC: ��������\r\n"););
    PrintBuff(MacSend.send_buf,MacSend.send_buf_len);

    Mac_Send(handle,MAC_FRAME_DATA,OPTION_DEFAULT);
  }

  return RET_MAC_SUCCESS;
  
}

/***************MAC�����ݷ���ӿ�***************/

