/*����������������������������������������������������
* �� �� ����Flash.h
* ��    �ߣ�����
* ��    �ڣ�2014-01-15
* �ļ�˵�����ڲ�FLASHͷ�ļ�
*����������������������������������������������������*/
#ifndef _FLASH_H
#define _FLASH_H

#include "Basedefine.h"
#include "Sys.h"

#ifdef __FLHDEBUG
#define FLHDBG(CODE) PRINT(CODE)
#else
#define FLHDBG(CODE)
#endif

#ifdef STM32F10X_HD
#define PAGE_SIZE   (0x800)
#else
#define PAGE_SIZE   (0x400)
#endif

#define DEVICE_ID_ADDR      0x1FFFF7E8    //����ID�洢��ַ
#define IFLASH_DB_ADDR_ONE     0x0803E800    //FLASH DB ��ʼ��ַ(250KB)
#define IFLASH_DB_ADDR_OTHER  (IFLASH_DB_ADDR_ONE + PAGE_SIZE)  //FLASH DB ��ʼ��ַ((250+2)KB)

#define DB_TABLE_INIT       0x5AA5
#define DB_TABLE_VAILD       0xA55A

//STM32_FLASH_TOTAL_SIZE=256k
#define CODE_START_ADDR         0x08000000    //���������(200KB)��ʼ��ַ
#define METERDOC_STORESTARTADDR   0x08032000    //�������(32KB)��ʼ��ַ

typedef enum
{
  FLH_SUCCESS = 0,  //���سɹ�
  FLH_ERASE_ERR,
  FLH_WRITE_ERR,
  FLH_WADDR_ERR,
  FLH_RADDR_ERR,
  FLH_NOSPACE_ERR,
  FLH_NOVAILD_ERR,
  FLH_NOFIELD_ERR,
}eFlashRet;

/*����������������������������������������������������
* �� �� ����IFlash_Init  
* ���������None
* ���������None
* �� �� ֵ��0  �ɹ�
      >0  ʧ��
* ����˵�����ڲ�FLASH��ʼ��
*����������������������������������������������������*/
u8  IFlash_Init(void);

/*����������������������������������������������������
* �� �� ����IFlash_Read  
* ���������TableId      ���
      FieldIndex    �ֶ�����
      pBuf      ��ȡ������  
      Len        ��ȡ����������
* ���������None
* �� �� ֵ��0  �ɹ�
      >0  ʧ��
* ����˵�����ڲ�FLASH������
*����������������������������������������������������*/
u8  IFlash_Read(u8 TableId, u8 FieldIndex, void* pBuf,u16 Len);

/*����������������������������������������������������
* �� �� ����IFlash_Write  
* ���������TableId      ���
      FieldIndex    �ֶ�����
      pBuf      д�뻺����  
      Len        д�뻺��������
* ���������None
* �� �� ֵ��0  �ɹ�
      >0  ʧ��
* ����˵�����ڲ�FLASHд����
*����������������������������������������������������*/
u8  IFlash_Write(u8 TableId, u8 FieldIndex, void* pBuf,u16 Len);

/*����������������������������������������������������
* �� �� ����IFlash_Count  
* ���������None
* ���������None
* �� �� ֵ��count  �ֶθ���
* ����˵������ȡ�ڲ�FLASH���ֶθ���
*����������������������������������������������������*/
u16 IFlash_Count(void);

/*����������������������������������������������������
* �� �� ����IFlash_GetDeviceID  
* ���������None
* ���������None
* �� �� ֵ��ID  оƬID
* ����˵������ȡ�ڲ�FLASHоƬID
*����������������������������������������������������*/
u16 IFlash_GetDeviceID(void);

/*����������������������������������������������������
* �� �� ����STM32_FlashPageErase  
* ���������pageAddr  ҳ��ַ
* ���������None
* �� �� ֵ��0  �ɹ�
      1  ʧ��
* ����˵�����ڲ�FLASHҳ����
*����������������������������������������������������*/
u8 STM32_FlashPageErase(u32 pageAddr);

/*����������������������������������������������������
* �� �� ����STM32_FlashWrite  
* ���������programAddr    д��ַ
      pBuffer      д������
      size      д��������С
* ���������None
* �� �� ֵ��0  �ɹ�
      >0  ʧ��      
* ����˵�����ڲ�FLASHд����
*����������������������������������������������������*/
u8 STM32_FlashWrite(u32 programAddr, u8* pBuffer, u16 size);

/*����������������������������������������������������
* �� �� ����STM32_FlashWriteZero  
* ���������None
* ���������None
* �� �� ֵ������������
* ����˵������ĳһ��ַд��
*����������������������������������������������������*/
u8 STM32_FlashWriteZero(u32 programAddr, u16 size);

#endif