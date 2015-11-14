/*——————————————————————————
* 文 件 名：Flash.h
* 作    者：于涛
* 日    期：2014-01-15
* 文件说明：内部FLASH头文件
*——————————————————————————*/
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

#define DEVICE_ID_ADDR      0x1FFFF7E8    //器件ID存储地址
#define IFLASH_DB_ADDR_ONE     0x0803E800    //FLASH DB 起始地址(250KB)
#define IFLASH_DB_ADDR_OTHER  (IFLASH_DB_ADDR_ONE + PAGE_SIZE)  //FLASH DB 起始地址((250+2)KB)

#define DB_TABLE_INIT       0x5AA5
#define DB_TABLE_VAILD       0xA55A

//STM32_FLASH_TOTAL_SIZE=256k
#define CODE_START_ADDR         0x08000000    //程序代码区(200KB)起始地址
#define METERDOC_STORESTARTADDR   0x08032000    //电表档案区(32KB)起始地址

typedef enum
{
  FLH_SUCCESS = 0,  //返回成功
  FLH_ERASE_ERR,
  FLH_WRITE_ERR,
  FLH_WADDR_ERR,
  FLH_RADDR_ERR,
  FLH_NOSPACE_ERR,
  FLH_NOVAILD_ERR,
  FLH_NOFIELD_ERR,
}eFlashRet;

/*——————————————————————————
* 函 数 名：IFlash_Init  
* 输入参数：None
* 输出参数：None
* 返 回 值：0  成功
      >0  失败
* 功能说明：内部FLASH初始化
*——————————————————————————*/
u8  IFlash_Init(void);

/*——————————————————————————
* 函 数 名：IFlash_Read  
* 输入参数：TableId      表号
      FieldIndex    字段索引
      pBuf      读取缓冲区  
      Len        读取缓冲区长度
* 输出参数：None
* 返 回 值：0  成功
      >0  失败
* 功能说明：内部FLASH读数据
*——————————————————————————*/
u8  IFlash_Read(u8 TableId, u8 FieldIndex, void* pBuf,u16 Len);

/*——————————————————————————
* 函 数 名：IFlash_Write  
* 输入参数：TableId      表号
      FieldIndex    字段索引
      pBuf      写入缓冲区  
      Len        写入缓冲区长度
* 输出参数：None
* 返 回 值：0  成功
      >0  失败
* 功能说明：内部FLASH写数据
*——————————————————————————*/
u8  IFlash_Write(u8 TableId, u8 FieldIndex, void* pBuf,u16 Len);

/*——————————————————————————
* 函 数 名：IFlash_Count  
* 输入参数：None
* 输出参数：None
* 返 回 值：count  字段个数
* 功能说明：获取内部FLASH表字段个数
*——————————————————————————*/
u16 IFlash_Count(void);

/*——————————————————————————
* 函 数 名：IFlash_GetDeviceID  
* 输入参数：None
* 输出参数：None
* 返 回 值：ID  芯片ID
* 功能说明：获取内部FLASH芯片ID
*——————————————————————————*/
u16 IFlash_GetDeviceID(void);

/*——————————————————————————
* 函 数 名：STM32_FlashPageErase  
* 输入参数：pageAddr  页地址
* 输出参数：None
* 返 回 值：0  成功
      1  失败
* 功能说明：内部FLASH页擦除
*——————————————————————————*/
u8 STM32_FlashPageErase(u32 pageAddr);

/*——————————————————————————
* 函 数 名：STM32_FlashWrite  
* 输入参数：programAddr    写地址
      pBuffer      写缓冲区
      size      写缓冲区大小
* 输出参数：None
* 返 回 值：0  成功
      >0  失败      
* 功能说明：内部FLASH写操作
*——————————————————————————*/
u8 STM32_FlashWrite(u32 programAddr, u8* pBuffer, u16 size);

/*——————————————————————————
* 函 数 名：STM32_FlashWriteZero  
* 输入参数：None
* 输出参数：None
* 返 回 值：见错误代码表
* 功能说明：对某一地址写０
*——————————————————————————*/
u8 STM32_FlashWriteZero(u32 programAddr, u16 size);

#endif