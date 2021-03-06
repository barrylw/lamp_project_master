#ifndef _GENERAL_H
#define _GENERAL_H
/*——————————————————————————
* 文 件 名：General.h 
* 文件说明: 普通函数源文件
*
* 当前版本：V8.0
* 作    者：ZL
* 开始日期：2013-12-30
*———————————————————————————*/

#include "Basedefine.h" 

#define CRC16_POLY 0x8005
#define CRC16_INIT 0xFFFF

/*************** 函数声明 ***************/

/*——————————————————————————
* 函 数 名：GetSum
* 输入参数：pMem:内存指针    Len-数据长度 
* 输出参数：None
* 返 回 值：算术累加值
* 功能说明：获取指定长度内存算术累加值
*——————————————————————————*/
u8 GetSum(u8 *pMem, u16 len);

/*——————————————————————————
* 函 数 名：CheckBCDFormat
* 输入参数：*pDat:内存指针    dataLen-数据长度 
* 输出参数：None
* 返 回 值：None
* 功能说明：检查是否是bcd码格式
*——————————————————————————*/
bool CheckBCDFormat(u8 *pDat, u16 dataLen);

/*——————————————————————————
* 函 数 名：GetCRC16
* 输入参数：*buff:内存指针    len:数据长度 
* 输出参数：None
* 返 回 值：检验结果
* 功能说明：计算一个数组的CRC结果
*——————————————————————————*/
u16 GetCRC16(u8 *buff, u16 len);

/*——————————————————————————
* 函 数 名：HexToBCD
* 输入参数：uValue:需要转换的数据
* 输出参数：None
* 返 回 值：转换结果
* 功能说明：将Hex转换成BCD码
*——————————————————————————*/
u8 HexToBCD(u8 uValue);

/*——————————————————————————
* 函 数 名：BCDToHex
* 输入参数：uValue:需要转换的数据
* 输出参数：None
* 返 回 值：转换结果
* 功能说明：将BCD转换成Hex码
*——————————————————————————*/
u8 BCDToHex(u8 uValue);

/*——————————————————————————
* 函 数 名：ShortLToH
* 输入参数：uValue:需要转换的数据
* 输出参数：None
* 返 回 值：转换结果
* 功能说明：将16位数高低位调换
*——————————————————————————*/
u16 ShortLToH(u16 uValue);

/*——————————————————————————
* 函 数 名：PrintBuffer
* 输入参数：pBuf:内存指针  
len:数据长度
* 输出参数：None
* 返 回 值：None
* 功能说明：内存打印
*——————————————————————————*/
void PrintBuff(u8 *pBuf, u16 len);
u8 GetChecksum(u8 *pbuffer, u16 length);
#endif

