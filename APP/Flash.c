/*����������������������������������������������������
* �� �� ����Flash.c  
* ��    �ߣ�����
* ��    �ڣ�2014-01-15
* �ļ�˵�����ڲ�FLASHԴ�ļ�
*����������������������������������������������������*/
#include "Flash.h"
#include "General.h"
#include "Mem.h"


typedef struct
{    
  u16 InitFlag;    //��Ч��ʶ 0x5AA5 ���������Ƿ��Ѿ�����
  u16 VaildFlag;    //��Ч��ʶ 0xA55A ���������Ƿ�����Ч�洢
  u16 SaveIndex;    //�洢��������˫���ݻ����������ķ������µ�  
  u16 FieldCount;    //�ֶ�����
  u16 Check;      //У�飬Ŀǰ����CRC16У�飬У������Ϊ�ýṹ��Checkǰ����+�洢��������
}DB_TABLE;

typedef struct
{
  u8  TableId;    //��ID��һ�㰴Ӧ�ò����֣����ڼ���
  u8  FieldIndex;    //�ֶ���������ͬһӦ�ò��£����ֲ�ͬ��Ϣ  
  u16 Len;      //�洢���ݵĳ���
  u16 Offset;      //�洢�����ڿ��ڵ�ƫ�Ƶ�ַ
  u16 Check;      //У�飬Ŀǰ����CRC16У�飬У������Ϊ�ýṹ��Checkǰ����+�洢��������
}DB_FIELD;

/*����������������������������������������������������
* �� �� ����STM32_FlashPageErase  
* ���������pageAddr  ҳ��ַ
* ���������None
* �� �� ֵ��0  �ɹ�
      1  ʧ��
* ����˵�����ڲ�FLASHҳ����
*����������������������������������������������������*/
u8 STM32_FlashPageErase(u32 pageAddr)
{  
  FLASH_Status status = FLASH_COMPLETE;

  FEED_WDG;

  FLASH_Unlock();
  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
  //FLHDBG(printf("STM32_FlashPageErase pageAddr = %08x\r\n",pageAddr););
  status = FLASH_ErasePage(pageAddr);

  if(status != FLASH_COMPLETE)
  {
    DISP(printf("STM32_FlashPageErase Err pageAddr = %08x status = %d\r\n",pageAddr,status););
    return FLH_ERASE_ERR;
  }

  FLASH_Lock();

  return FLH_SUCCESS;
}

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
u8 STM32_FlashWrite(u32 programAddr, u8* pBuffer, u16 size)
{
  u8 sflag;  
  u16 offset = 0,len = size;  
  FLASH_Status status = FLASH_COMPLETE;

  FLASH_Unlock();
  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
  //FLHDBG(printf("STM32_FlashWrite programAddr = %08x pBuffer[0] = %02x size = %d\r\n",programAddr, pBuffer[0], size););
  sflag = len%2;
  if(sflag)
  {
    len --;
  }

  for(offset = 0; offset < len; offset += 2)
  {
    status = FLASH_ProgramHalfWord(programAddr + offset, ((u16)(pBuffer[offset + 1]) << 8) | (pBuffer[offset]));
    if(status != FLASH_COMPLETE)
    {
      DISP(printf("STM32_FlashWrite Err programAddr = %08x size = %d offset = %d status = %d\r\n",programAddr,size,offset,status););
      return FLH_WRITE_ERR;
    }
  }

  //FLHDBG(printf("STM32_FlashWrite programAddr = %08x offset = %d size = %d\r\n",programAddr, offset, size););

  if(sflag && size>=1)
  {
    u16 val;

    val = pBuffer[size - 1]|0xFF00;

    //FLHDBG(printf("STM32_FlashWrite programAddr = %08x offset = %d val = %04x\r\n",programAddr, offset, val););

    status = FLASH_ProgramHalfWord(programAddr + offset, val);
    if(status != FLASH_COMPLETE)
    {
      DISP(printf("STM32_FlashWrite Err programAddr = %08x size = %d offset = %d status = %d\r\n",programAddr,size,offset,status););
      return FLH_WRITE_ERR;
    }        
  }

  FLASH_Lock();

  return FLH_SUCCESS;
}

/*����������������������������������������������������
* �� �� ����IFlash_Init  
* ���������None
* ���������None
* �� �� ֵ��0  �ɹ�
      >0  ʧ��
* ����˵�����ڲ�FLASH��ʼ��
*����������������������������������������������������*/
u8 IFlash_Init(void)
{
  u8 ret;
  u16 crc;
  u8 oneVaildFlag = 0,otherVaildFlag = 0;
  u8 oneEraseFlag = 0,otherEraseFlag = 0;
  u16 oneSaveIndex = 0,otherSaveIndex = 0;  
  DB_TABLE dbTable,*pdbTable;  
  
  //FLHDBG(printf("FLH: Init into\r\n"););
  //���洢���Ƿ��Ѿ��������������������ΪDB_TABLE_INIT
  
  pdbTable = (DB_TABLE*)IFLASH_DB_ADDR_ONE;

  //FLHDBG(printf("FLH: Init one pdbTable->InitFlag = %d \r\n",pdbTable->InitFlag););
  //FLHDBG(printf("FLH: Init one pdbTable->VaildFlag = %d\r\n",pdbTable->VaildFlag););

  if(pdbTable->VaildFlag == DB_TABLE_VAILD)//��Flash_DB_Init����ֻ֤��һ����Ч��
  {
    crc = GetCRC16((u8*)pdbTable,sizeof(DB_TABLE)-sizeof(u16));    
    if(pdbTable->Check == crc)
    {
      oneEraseFlag = 0;
      oneVaildFlag = 1;
      oneSaveIndex = pdbTable->SaveIndex;
    }        
    else
    {
      oneEraseFlag = 1;
    }      
  }  
  else if(pdbTable->InitFlag != DB_TABLE_INIT)
  {
    oneEraseFlag = 1;
  }

  pdbTable = (DB_TABLE*)IFLASH_DB_ADDR_OTHER;
  
  //FLHDBG(printf("FLH: Init other pdbTable->InitFlag = %d \r\n",pdbTable->InitFlag););
  //FLHDBG(printf("FLH: Init other pdbTable->VaildFlag = %d\r\n",pdbTable->VaildFlag););
  
  if(pdbTable->VaildFlag == DB_TABLE_VAILD)//��Flash_DB_Init����ֻ֤��һ����Ч��
  {
    crc = GetCRC16((u8*)pdbTable,sizeof(DB_TABLE)-sizeof(u16));    
    if(pdbTable->Check == crc)
    {
      otherEraseFlag = 0;
      otherVaildFlag = 1;
      otherSaveIndex = pdbTable->SaveIndex;
    }        
    else
    {
      otherEraseFlag = 1;
    }  
  }  
  else if(pdbTable->InitFlag != DB_TABLE_INIT)
  {
    otherEraseFlag = 1;
  }

  //FLHDBG(printf("FLH: Init oneVaildFlag = %d otherVaildFlag = %d\r\n",oneVaildFlag,otherVaildFlag););
  //FLHDBG(printf("FLH: Init oneSaveIndex = %d otherSaveIndex = %d\r\n",oneSaveIndex,otherSaveIndex););
  
  if(oneVaildFlag && otherVaildFlag)
  {
    oneSaveIndex += 1;
    if(oneSaveIndex == otherSaveIndex)//�ٶ����θ�д֮��һ���Ǽ��1��������Ǽ��1���϶��ǳ������ش�����
    {
      oneEraseFlag = 1;  
    }
    else
    {
      otherEraseFlag = 1;
    }
  }  
  
  //FLHDBG(printf("FLH: Init oneEraseFlag = %d otherEraseFlag = %d\r\n",oneEraseFlag,otherEraseFlag););

  dbTable.InitFlag = DB_TABLE_INIT;
  if(oneEraseFlag)
  {
    ret = STM32_FlashPageErase(IFLASH_DB_ADDR_ONE);    
    if(ret != FLH_SUCCESS) 
    {
      return ret;
    }
    ret = STM32_FlashWrite(IFLASH_DB_ADDR_ONE,(u8*)&dbTable,sizeof(u16));      
    if(ret != FLH_SUCCESS) 
    {
      return ret;
    }
  }  

  if(otherEraseFlag)
  {
    ret = STM32_FlashPageErase(IFLASH_DB_ADDR_OTHER);    
    if(ret != FLH_SUCCESS) 
    {
      return ret;
    }
    ret = STM32_FlashWrite(IFLASH_DB_ADDR_OTHER,(u8*)&dbTable,sizeof(u16));        
    if(ret != FLH_SUCCESS)
    {
      return ret;
    }
  }
  
  FLHDBG(printf("FLH: �ڲ�Flash��ʼ�����\r\n"););

  return FLH_SUCCESS;
}

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
u8 IFlash_Read(u8 TableId,u8 FieldIndex,void* pBuf,u16 Len)
{
  u8* pData;
  u16 i;
  DB_TABLE *pdbTable;
  DB_FIELD *pdbField;  
  u32 oldDbAddr;

  FLHDBG(printf("FLH: �����%d �ֶκ�%d\r\n",TableId,FieldIndex););

  pdbTable = (DB_TABLE*)IFLASH_DB_ADDR_ONE;
  if(pdbTable->VaildFlag == DB_TABLE_VAILD)//��Flash_DB_Init����ֻ֤��һ����Ч��
  {
    oldDbAddr = IFLASH_DB_ADDR_ONE;
  }
  else
  {
    pdbTable = (DB_TABLE*)IFLASH_DB_ADDR_OTHER;
    if(pdbTable->VaildFlag == DB_TABLE_VAILD)
    {
      oldDbAddr = IFLASH_DB_ADDR_OTHER;      
    }
  }

  //FLHDBG(printf("IFlash_Read oldDbAddr = %08x\r\n",oldDbAddr););
  //FLHDBG(printf("IFlash_Read pdbTable->InitFlag = %d \r\n",pdbTable->InitFlag););
  //FLHDBG(printf("IFlash_Read pdbTable->VaildFlag = %d\r\n",pdbTable->VaildFlag););

  if(pdbTable->VaildFlag == DB_TABLE_VAILD)
  {                    
    //FLHDBG(printf("FLH: Read dbTable.SaveIndex = %d dbTable.FieldCount = %d\r\n",pdbTable->SaveIndex,pdbTable->FieldCount););
    pdbField = (DB_FIELD*)(oldDbAddr + sizeof(DB_TABLE));  
    for(i=0;i<pdbTable->FieldCount;i++)
    {            
      //FLHDBG(printf("IFlash_Read i = %d FieldIndex = %d TableId = %d Len = %d\r\n",i,pdbField[i].FieldIndex,pdbField[i].TableId,pdbField[i].Len););
      if(pdbField[i].FieldIndex == FieldIndex && pdbField[i].TableId == TableId)
      {        
        pData = (u8*)(oldDbAddr + pdbField[i].Offset);
        //FLHDBG(printf("IFlash_Read pdbField[%d].Len = %d Len = %d pData[0] = %02x Offset = %d\r\n",i,pdbField[i].Len,Len,pData[0],pdbField[i].Offset););
        if(pdbField[i].Len < Len)
        {
          Len = pdbField[i].Len;
        }
        MemCpy(pBuf,pData,Len);  

        FLHDBG(printf("FLH: ��ȡ�ɹ�\r\n"););

        return FLH_SUCCESS;              
      }      
    }
    return FLH_NOFIELD_ERR;      
  }

  return FLH_NOVAILD_ERR;
}

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
u8 IFlash_Write(u8 TableId,u8 FieldIndex,void* pBuf,u16 Len)
{
  u8 ret;
  u8* pData;
  u16 i,findindex,wlen;
  u16 offset,total,dataoffset;
  DB_TABLE dbTable,*pdbTable;
  DB_FIELD dbField,*pdbField;  
  u32 oldDbAddr = 0 , newDbAddr = 0;

  FLHDBG(printf("FLH: д���%d �ֶκ�%d\r\n",TableId,FieldIndex););

  pdbTable = (DB_TABLE*)IFLASH_DB_ADDR_ONE;
  if(pdbTable->VaildFlag == DB_TABLE_VAILD)//��Flash_DB_Init����ֻ֤��һ����Ч��
  {
    oldDbAddr = IFLASH_DB_ADDR_ONE;
    newDbAddr = IFLASH_DB_ADDR_OTHER;
  }
  else
  {
    pdbTable = (DB_TABLE*)IFLASH_DB_ADDR_OTHER;
    if(pdbTable->VaildFlag == DB_TABLE_VAILD)
    {
      oldDbAddr = IFLASH_DB_ADDR_OTHER;      
    }
    else
    {
      oldDbAddr = 0;          
    }
    newDbAddr = IFLASH_DB_ADDR_ONE;  
  }
  
  //FLHDBG(printf("IFlash_Write oldDbAddr = %08x newDbAddr = %08x\r\n",oldDbAddr,newDbAddr););
  //FLHDBG(printf("IFlash_Write pdbTable->InitFlag = %d \r\n",pdbTable->InitFlag););
  //FLHDBG(printf("IFlash_Write pdbTable->VaildFlag = %d\r\n",pdbTable->VaildFlag););
  
  if(newDbAddr != IFLASH_DB_ADDR_ONE && newDbAddr != IFLASH_DB_ADDR_OTHER)
  {
    return FLH_WADDR_ERR;
  }  
 
  findindex = 0xFFFF;
  total = sizeof(DB_TABLE);      
  if(pdbTable->VaildFlag == DB_TABLE_VAILD)
  {      
    dbTable.SaveIndex = pdbTable->SaveIndex;      
    dbTable.FieldCount = pdbTable->FieldCount;    
    pdbField = (DB_FIELD*)(oldDbAddr + sizeof(DB_TABLE));    
    for(i=0;i<dbTable.FieldCount;i++)
    {            
      //FLHDBG(printf("IFlash_Write i = %d FieldIndex = %d TableId = %d Len = %d\r\n",i,pdbField[i].FieldIndex,pdbField[i].TableId,pdbField[i].Len););
      if(pdbField[i].FieldIndex == FieldIndex && pdbField[i].TableId == TableId)
      {        
        findindex = i;                
      }
      else
      {        
        total += sizeof(DB_FIELD) + pdbField[i].Len;
      }      
    }
  }
  else
  {
    dbTable.SaveIndex = 0;
    dbTable.FieldCount = 0;    
  }

  if(dbTable.FieldCount > 0 && (oldDbAddr != IFLASH_DB_ADDR_ONE && oldDbAddr != IFLASH_DB_ADDR_OTHER))//�����ǰ������ɵ�ַӦ��Ϊ��Ч��ַ����֤�ж�    
  {
    return FLH_RADDR_ERR;  
  } 

  //FLHDBG(printf("FLH: Write dbTable.SaveIndex = %d dbTable.FieldCount = %d\r\n",dbTable.SaveIndex,dbTable.FieldCount););

  if(total + sizeof(DB_FIELD) + Len + (Len%2)> PAGE_SIZE)
  {
    return FLH_NOSPACE_ERR;
  }  
  
  dbTable.InitFlag = DB_TABLE_INIT;
  dbTable.VaildFlag = DB_TABLE_VAILD;
  dbTable.SaveIndex ++;    

  if(findindex == 0xFFFF)//δ�ҵ�
  {  
    findindex = dbTable.FieldCount;
    dbTable.FieldCount ++;
  }

  dbTable.Check = GetCRC16((u8*)&dbTable,sizeof(DB_TABLE)-sizeof(u16));
  
  //FLHDBG(printf("IFlash_Write findindex = %d dbTable.FieldCount = %d\r\n",findindex,dbTable.FieldCount););
    
  ret = STM32_FlashWrite(newDbAddr+sizeof(u16),(u8*)&dbTable.VaildFlag,sizeof(DB_TABLE)-sizeof(u16)-sizeof(u16));
  if(ret != FLH_SUCCESS) 
  {
    return ret;
  }
    
  offset = sizeof(DB_TABLE);  
  dataoffset = sizeof(DB_TABLE) + dbTable.FieldCount*sizeof(DB_FIELD);  
  
  //FLHDBG(printf("IFlash_Write begin offset = %d dataoffset = %d\r\n",offset,dataoffset););

  for(i=0;i<dbTable.FieldCount;i++)
  {          
    if(i == findindex)
    {        
      dbField.TableId = TableId;
      dbField.FieldIndex = FieldIndex;
      dbField.Len = Len + (Len%2);
      dbField.Offset = dataoffset;
      dbField.Check = GetCRC16(pBuf,Len);    
      pData = pBuf;    
      wlen = Len;                                  
    }
    else
    {              
      pdbField = (DB_FIELD*)(oldDbAddr + offset);  
      dbField.TableId = pdbField->TableId;
      dbField.FieldIndex = pdbField->FieldIndex;
      dbField.Len = pdbField->Len;
      dbField.Offset = dataoffset;
      dbField.Check = pdbField->Check;
      pData = (u8*)(oldDbAddr + pdbField->Offset);  
      wlen = pdbField->Len;                      
    }
    ret = STM32_FlashWrite(newDbAddr+offset,(u8*)&dbField,sizeof(DB_FIELD));  
    if(ret != FLH_SUCCESS) 
    {
      return ret;      
    }
    offset += sizeof(DB_FIELD);

    //FLHDBG(printf("IFlash_Write i =%d wlen = %d dataoffset = %d\r\n",i,wlen,dataoffset););

    ret = STM32_FlashWrite(newDbAddr+dataoffset,pData,wlen);  
    if(ret != FLH_SUCCESS)
    {
      return ret;  
    }
    dataoffset += dbField.Len;

    //FLHDBG(printf("IFlash_Write i =%d offset = %d dataoffset = %d\r\n",i,offset,dataoffset););
  }
  
  ret = STM32_FlashWrite(newDbAddr+sizeof(DB_TABLE)-sizeof(u16),(u8*)&dbTable.Check,sizeof(u16));
  if(ret != FLH_SUCCESS) 
  {
    return ret;
  }
    
  if(oldDbAddr == IFLASH_DB_ADDR_ONE || oldDbAddr == IFLASH_DB_ADDR_OTHER)
  {    
    ret = STM32_FlashPageErase(oldDbAddr);    
    if(ret != FLH_SUCCESS) 
    {
      return ret;  
    }
    ret = STM32_FlashWrite(oldDbAddr,(u8*)&dbTable,sizeof(u16));    
    if(ret != FLH_SUCCESS) 
    {
      return ret;    
    }
  }

  FLHDBG(printf("FLH: д��ɹ�\r\n"););

  return FLH_SUCCESS;  
}

/*����������������������������������������������������
* �� �� ����IFlash_Count  
* ���������None
* ���������None
* �� �� ֵ��count  �ֶθ���
* ����˵������ȡ�ڲ�FLASH���ֶθ���
*����������������������������������������������������*/
u16 IFlash_Count(void)
{
  u16 count = 0;
  DB_TABLE *pdbTable;  

  pdbTable = (DB_TABLE*)IFLASH_DB_ADDR_ONE;
  if(pdbTable->VaildFlag == DB_TABLE_VAILD)//��Flash_DB_Init����ֻ֤��һ����Ч��
  {
    count = pdbTable->FieldCount;  
  }
  else
  {
    pdbTable = (DB_TABLE*)IFLASH_DB_ADDR_OTHER;
    if(pdbTable->VaildFlag == DB_TABLE_VAILD)
    {
      count = pdbTable->FieldCount;      
    }
  }

  FLHDBG(printf("FLH: �ֶ����� %d\r\n",count););

  return count;
}

/*����������������������������������������������������
* �� �� ����IFlash_GetDeviceID  
* ���������None
* ���������None
* �� �� ֵ��ID  оƬID
* ����˵������ȡ�ڲ�FLASHоƬID
*����������������������������������������������������*/
u16 IFlash_GetDeviceID(void)
{
  u16 ID = 0;

  ID = GetCRC16((u8*)DEVICE_ID_ADDR, 12);

  FLHDBG(printf("FLH: �豸��0x%04x\r\n",ID););

  return ID;
}

/*����������������������������������������������������
* �� �� ����STM32_FlashWriteZero  
* ���������None
* ���������None
* �� �� ֵ������������
* ����˵������ĳһ��ַд��
*����������������������������������������������������*/
u8 STM32_FlashWriteZero(u32 programAddr, u16 size)
{
  u16 offset = 0;
  FLASH_Status status = FLASH_COMPLETE;

  FLASH_Unlock();
  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

  if (size%2==1)
  {
    size +=1;
  }

  for(offset = 0; offset < size; offset += 2)
  {
    status = FLASH_ProgramHalfWord(programAddr + offset, 0);
    if(status != FLASH_COMPLETE)
    {
      //FLHDBG(printf("STM32FLASH: Write zero error."););
      return FLH_WRITE_ERR;
    }
  }

  FLASH_Lock();

  return FLH_SUCCESS;
}
