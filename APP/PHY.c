/**
  ******************************************************************************
  * @file    phy.c
  * @author  William Liang
  * @version V1.0.0
  * @date    09/23/2013
  * @brief   This file contains physical layer task.
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
//#include "Radio.h"
#include "hal_radio.h" 
#include "sx1276-LoRa.h"
#include "PHY.h"
#include "sx1276.h"
/** @addtogroup Physical Layer
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

//�����汾
__root const Manufacturer_Version phyVersion = 
{
  {'B', 'R'},    //���̴��� 
  {'2', '3'},    //оƬ����
   0x16, 0x12, 0x15,//������
  {0x02, 0x00}//�汾
};

u8 phyCurrentChannel = 0;
u8 phyCurrentChannelGroup = 27;
u8 phyCCAMode = 1;
u32 phyChannelsSupported = 0xFFFFFFFF;

u8 phyTransmitPower = HIGHEST_LEVEL;//16dBm
u8 phyRSSIThreshold = RSSI_THRESHOLD;//-96dBm
u8 phyCCATimout = CSMA_CCA_TIMEOUT;

extern st_RF_LoRa_DypeDef g_sRF;
extern ST_EVENT_CONTROL sRFCCAEvent;
extern ST_EVENT_CONTROL sRFReceiveEvent;
extern ST_EVENT_CONTROL sRFTransmitEvent;
extern ST_EVENT_METHOD RFEvents[];
extern float RxPacketRssiValue;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handle physical layer task in the main loop.
  * @param  None.
  * @retval None.
  */
void Phy_Proc(void)
{  
  //hal_RunRFEvents(RFEvents);
}

/**
  * @brief  Initialize system hardware.
  * @param  None.
  * @retval None.
  */
void Phy_Init(void)
{  
  hal_InitRF();
  PHYDBG(printf("PHY: ��ʼ�����\r\n"););
}


/******************************************************************************/
/*           protocol stack primitive interface                        */
/******************************************************************************/

u8 Phy_Data_Request(u16 psduLen, u8 *psdu, u8 channel)
{
  hal_sRF_Transmit(psdu, psduLen, channel);
  return 1;
}

void Phy_CCA_Request(void)
{
 
}

u8 Phy_GetAttr(u8 attr,u8 attrlen,u8* val)
{
  return 0;
}

u8 Phy_SetAttr(u8 attr,u8 attrlen,u8* val)
{
  return 1;
}

void PLME_SET_TRX_STATE_Request(PHY_STATUS status)
{
}

/*����������������������������������������������������
* �� �� ����Phy_RxData
* ���������phase    ��λ
* ���������None
* �� �� ֵ��0   δ���յ�����
            1   ���յ�����
* ����˵������������ݽ���
*����������������������������������������������������*/
u8 Phy_RxData()
{
  return 0;
}

/*����������������������������������������������������
* �� �� ����Phy_GetData
* ���������phase        ��λ
            psdulen     �������ݳ���
            psdu        �������ݻ�����
            rxmode      ����ģʽ
            rssi        ��������
* ���������None
* �� �� ֵ��0   ��ȡ�ɹ�
* ����˵����������ȡ��������
*����������������������������������������������������*/
u8 Phy_GetData(u16* psdulen, u8* psdu, u8* rssi)
{
  return 0;
}


void ReadVersion(void)
{
  u8 buf[sizeof(Manufacturer_Version)];
  
  buf[0] = phyVersion.venderID[0];
  buf[1] = phyVersion.venderID[1];
  buf[2] = phyVersion.chipID[0];
  buf[3] = phyVersion.chipID[1];
  buf[4] = phyVersion.date;
  buf[5] = phyVersion.month;
  buf[6] = phyVersion.year;
  buf[7] = phyVersion.version[0];
  buf[8] = phyVersion.version[1];
  
  printf("APL Version=%c%c%c%c-%02x%02x%02x-V%02x.%02x\r\n", \
     buf[1], buf[0], buf[3], buf[2], buf[6], buf[5], buf[4], buf[8], buf[7]);
     
 
}
/******************* (C) COPYRIGHT 2013 Robulink Technology Ltd.*****END OF FILE****/
