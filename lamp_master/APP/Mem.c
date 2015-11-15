/*——————————————————————————
* 文 件 名：Mem.c  
* 文件说明：内存拷贝源文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2013-12-30
*———————————————————————————*/
#include "Mem.h"


/************* 函数声明 *************/
/*——————————————————————————
* 函 数 名：MemCpy
* 输入参数：pSrc  源内存指针    
      len    数据长度 
* 输出参数：pDsc  目的内在指针
* 返 回 值：拷贝后的目的偏移地址
* 功能说明：拷贝一段内存内容
*——————————————————————————*/
u8 *MemCpy(void *pDsc, void*pSrc, u16 len)
{
  u8 *pAddr1 = (u8 *)pDsc;
  u8 *pAddr2 = (u8 *)pSrc;

  while(len--)
  {
    *pAddr1++ = *pAddr2++;
  }

  return pAddr1;
}

/*——————————————————————————
* 函 数 名：MemSet
* 输入参数：pAddr  内存指针    
      value  需要设置的值
      len    数据长度 
* 输出参数：None
* 返 回 值：None
* 功能说明：设置某一段内容为某值
*——————————————————————————*/
void MemSet(void *pAddr, u8 value, u16 len)
{
  u8 *pTmp = (u8 *)pAddr;
  
  while(len--)
  {
    *pTmp++ = value;
  }
}

/*——————————————————————————
* 函 数 名：MemCmp
* 输入参数：pDsc  目的地址    
      pSrc  源地址
      Len    数据长度 
* 输出参数：None
* 返 回 值：1  不一致  
      0  一致
* 功能说明：比较两个内存中的值
*——————————————————————————*/
u8 MemCmp(void *pDsc, void *pSrc, u16 len)
{
  u8 *pAddr1 = (u8 *)pDsc;
  u8 *pAddr2 = (u8 *)pSrc;
  
  while(len--)
  {
    if (*pAddr1 != *pAddr2)
    {
      return 1;
    }

    pAddr1++;
    pAddr2++;
  }

  return 0;
}

/*——————————————————————————
* 函 数 名：MemCmpData
* 输入参数：buf    数据区    
      data  比较数据
      len    数据长度 
* 输出参数：None
* 返 回 值：>0  不一致  
      0  一致
* 功能说明：比较内存中的值与某个数据一致
*——————————————————————————*/
u8 MemCmpData(const u8 *buf,u8 data,u16 len)
{
  while(len--)
  {
    if(buf[len] != data)
    {
      return buf[len]-data;
    }
  }

  return 0;
}