/*����������������������������������������������������
* �� �� ����MAC.h  
* ��    �ߣ�����
* ��    �ڣ�2014-04-06
* �ļ�˵����MAC��ͷ�ļ�
*����������������������������������������������������*/
#ifndef _MAC_H
#define _MAC_H

#include "Comm.h"

#pragma pack(1)

#ifdef __MACDEBUG
#define MACDBG(CODE) PRINT(CODE)
#else
#define MACDBG(CODE)
#endif

#define MacProtoVer      1          //MAC��Э��汾��
#define MaxMSDUSize      300          //MAC֡����غɳ���
#define MacBuffLen      320          //MAC����󻺳�������
#define MaxNB        6          //CSMA-CA�㷨�е����ô������ֵ
#define MaxBE        10          //CSMA-CA�㷨�е�����ָ�����ֵ

#define BroadNID      0xffff        //�㲥NID
#define BroadAimAddr    0xffff        //�㲥Ŀ���ַ

#define WaitSendMaxTime    500          //MAC�����ȴ�����ʱ�䣬5��
#define SendingMaxTime    400          //MAC������ͳ���ʱ�䣬4��
#define ReadMaxTime      300          //MAC������ȡ����ʱ�䣬3��

#define NotMatch      0          //��ƥ��
#define Match        1          //ƥ��

#define MacSendFailure    0          //MAC�㷢��ʧ��
#define MacSendSuccess    1          //MAC�㷢�ͳɹ�

//������Ϣ
typedef enum
{
  RET_MAC_SUCCESS,        //���سɹ�
  RET_MAC_PHASE_ERR,        //������λ����  
  RET_MAC_ATTRLEN_ERR,      //���Գ��ȴ���
  RET_MAC_ATTR_NOEXIST,      //������������
}eMacRet;

//����ѡ��
typedef enum
{
  OPTION_DEFAULT  = 0x00,      //Ĭ��  
  OPTION_ACK_READ = 0x01      //���ͷ�����ʶ  1 ��Ҫ��ȡȷ��  0 ����Ҫ��ȡȷ��
}eMacTxOption;

//֡����
typedef enum
{  
  MAC_FRAME_BEACON,        //�ű�֡
  MAC_FRAME_DATA,          //����֡
  MAC_FRAME_ACK,          //Ӧ��֡
  MAC_FRAME_STOPRELAY,      //ֹͣת��֡
  MAC_FRAME_RESERVE        //Ԥ��
}eMacFrameType;

//����״̬
typedef enum
{
  SEND_STATUS_IDLE,        //����δ���
  SEND_STATUS_FINISH,        //�������
  SEND_STATUS_START,        //��������
  SEND_STATUS_TX_ING        //������
}eMacSendStatus;

//������״̬
typedef enum
{
  SEND_SUB_STATUS_TX,        //��������
  SEND_SUB_STATUS_CCA,      //CCA���
  SEND_SUB_STATUS_WAIT      //�ȴ�ʱ϶
}eMacSendSubStatus;

/***************MAC������***************/

typedef struct
{        
  u8  mac_NB;            //CSMA-CA���ô���
  u8  mac_BE;            //CSMA-CA����ָ��  
  u16 mac_NID;          //NID
  u16  mac_Short;          //�̵�ַ 
  u8  mac_Addr[6];        //����ַ 
}sMacAttr;
/***************MAC������***************/

//��������
typedef struct 
{  
  u16 mac_recv_buf_len;      //����֡���ݳ���
  u8   mac_recv_buf[MacBuffLen];  //����֡���ݻ�����
  u8   mac_recv_rssi;        //����֡�ź�ֵ
  u8   mac_recv_data_ready;    //���յ�֡���ݱ�ʶ
}sMacRecv;

//��������
typedef struct
{
  u8   stoprelay_handle;      //����ֹͣת�������
  u8   dataframe_handle;      //�������ݰ����
  u8  follow_send_stoprelay;    //��������ֹͣת����
  u8  follow_send_dataframe;    //�����������ݰ�
  u16  wait_read_time;        //�ȴ���ȡ��ʱʱ��
  u16  wait_send_time;        //�ȴ���������ʱ��  
  u16  wait_send_ack_time;      //�ȴ��������ʱ��    
  u8   send_status;        //����״̬
  u8  send_sub_status;      //������״̬  
  u8   send_result;        //���ͽ��
  u8   send_result_read;      //���ͽ������  1 ��Ҫ�ȴ���ȡ������� 0 ����Ҫ�ȴ���ȡ����
  u8   send_mode;          //����ģʽ
  u16  send_CCA_delay;        //����CCA��ʱ  
  u16  send_buf_len;        //�������ݻ���������
  u8  send_buf[MacBuffLen];    //�������ݻ�����        
  u8  stoprelay_len;        //ֹͣת������������
  u8  stoprelay_buf[4];      //ֹͣת��������
}sMacSend;

//ֹͣת��֡
typedef struct
{
  u8  resv:6;            //Ԥ��
  u8  frame_type:2;        //֡����
  u16 nid;            //NID  
  u8  data;            //������    
}sMacStopRelayFrame;

/***************MAC��������***************/
void Mac_Init(void);
void Mac_Proc(void);
void Mac_CCA_Proc(void);
void Mac_CCA_Confirm(u8 status);
/***************MAC��������***************/

/***************MAC�����ݷ���ӿ�***************/
u8 Mac_StopRelay_Request(u8 handle,u8 life_perild,u8 flag);
u8 Mac_Data_Request(u8 handle,u16 msdulen,u8* msdu,u8 txoptions);
/***************MAC�����ݷ���ӿ�***************/

/***************MAC������ָʾ�ӿ�***************/
extern u8 Mcp_Data_Indication(u16 len,u8* buff,u8 rssi);
/***************MAC������ָʾ�ӿ�***************/

#endif
