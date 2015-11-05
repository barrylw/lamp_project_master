#include "Str.h"


/*——————————————————————————
* 函 数 名：StrLen
* 输入参数：pStr:字符串指针
* 输出参数：None
* 返 回 值：字符串长度
* 功能说明：计算字符串长度
*——————————————————————————*/
u16 StrLen(char *pStr)
{
  u16 len;

  len = 0;
  while(*pStr++ != 0)
  {
    len++;  
  }

  return len;
}

/*——————————————————————————
* 函 数 名：StrCmp
* 输入参数：pDsc:字符串目的地址    pSrc:字符串源地址
* 输出参数：None
* 返 回 值：成功标志
* 功能说明：比较字符串内容
*——————————————————————————*/
bool StrCmp(char *pDsc, char *pSrc)
{
  while( (*pDsc != NULL) && (*pSrc!=NULL) )
  {
    if (*pDsc != *pSrc)
    {
      return FALSE;
    }

    pDsc++;
    pSrc++;
  }

  return TRUE;
}

/*——————————————————————————
* 函 数 名：GetProStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取协议版本字符串
*——————————————————————————*/
const u8 * GetProStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "旧协议";
  }
  else if (val == 1)
  {
    p = "新协议";
  }
  else
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetValidStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取有效字符串
*——————————————————————————*/
const u8 * GetValidStr(u8 val)
{  
  const u8 * p;
  
  if (val == 0)
  {
    p = "无效";
  }
  else if (val == 1)
  {
    p = "有效";
  }
  else
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetConfirmStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取确认字符串
*——————————————————————————*/
const u8 * GetConfirmStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "未确认";
  }
  else if (val == 1)
  {
    p = "确认";
  }
  else
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetTypeStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取设备类型字符串
*——————————————————————————*/
const u8 * GetTypeStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "主节点";
  }
  else if (val == 1)
  {
    p = "单相表";
  }
  else if (val == 2)
  {
    p = "三相表";
  }
  else if (val == 3)
  {
    p = "I采";
  }
  else if (val == 4)
  {
    p = "II采";
  }
  else if (val == 5)
  {
    p = "中继器";
  }
  else if (val == 6)
  {
    p = "485表";
  }
  else if (val == 7)
  {
    p = "未知";
  }
  else 
  {
    p = "其他";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetProTypeStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取协议类型字符串
*——————————————————————————*/
const u8 * GetProTypeStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "透传";
  }
  else if (val == 1)
  {
    p = "97";
  }
  else if (val == 2)
  {
    p = "07";
  }  
  else 
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetConfigStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取配置字符串
*——————————————————————————*/
const u8 * GetConfigStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "未配置";
  }
  else if (val == 1)
  {
    p = "已配置";
  }
  else
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetReadStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取抄读结果字符串
*——————————————————————————*/
const u8 * GetReadStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "待抄读";
  }  
  else if (val == 1)
  {
    p = "有成功";
  }
  else if (val == 2)
  {
    p = "全成功";
  }
  else if (val == 3)
  {
    p = "全失败";
  }
  else
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetStatusStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取路由状态字符串
*——————————————————————————*/
const u8 * GetStatusStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "点对点";
  }
  else if (val == 1)
  {
    p = "源路由";
  }
  else if (val == 2)
  {
    p = "盲中继";
  }
  else if (val == 3)
  {
    p = "未知";
  }
  else
  {
    p = "其他";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetModeStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取路由方式字符串
*——————————————————————————*/
const u8 * GetModeStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "点对点";
  }
  else if (val == 1)
  {
    p = "源路由";
  }
  else if (val == 2)
  {
    p = "盲中继";
  }
  else
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetBeaconModeStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取组网模式字符串
*——————————————————————————*/
const u8 * GetBeaconModeStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "低门限";
  }
  else if (val == 1)
  {
    p = "中门限";
  }
  else if (val == 2)
  {
    p = "高门限";
  }
  else
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetNIModeStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取收集模式字符串
*——————————————————————————*/
const u8 * GetNIModeStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "配置收集";
  }  
  else if (val == 1)
  {
    p = "组网收集";
  }
  else 
  {
    p = "未知";
  }  

  return p;
}

/*——————————————————————————
* 函 数 名：GetNetStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取网络标识字符串
*——————————————————————————*/
const u8 * GetNetStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "本网";
  }
  else if (val == 1)
  {
    p = "全网";
  }
  else
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetNwkAttrStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取网络层属性字符串
*——————————————————————————*/
const u8 * GetNwkAttrStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "网络层版本";
  }
  else if (val == 1)
  {
    p = "主节点地址";
  }
  else if (val == 2)
  {
    p = "组网模式";
  }
  else if (val == 3)
  {
    p = "收集模式";
  }
  else if (val == 4)
  {
    p = "路由方式";
  }
  else if (val == 5)
  {
    p = "路由级数";
  }
  else if (val == 6)
  {
    p = "网络标识";
  }
  else if (val == 7)
  {
    p = "网络规模";
  }
  else if (val == 8)
  {
    p = "转发级数";
  }
  else if (val == 9)
  {
    p = "低门限值";
  }
  else if (val == 10)
  {
    p = "高门限值";
  }  
  else if (val == 11)
  {
    p = "退让指数";
  }
  else if (val == 12)
  {
    p = "非竞争时隙";
  }
  else if (val == 13)
  {
    p = "帧序号";
  }
  else 
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetNwkStatusStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取网络层主状态字符串
*——————————————————————————*/
const u8 * GetNwkStatusStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "空闲";
  }
  else if (val == 1)
  {
    p = "信标";
  }
  else if (val == 2)
  {
    p = "收集";
  }
  else if (val == 3)
  {
    p = "数据";
  }  
  else 
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetNwkSubStaStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取网络层子状态字符串
*——————————————————————————*/
const u8 * GetNwkSubStaStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "空闲";
  }
  else if (val == 1)
  {
    p = "准备发送";
  }
  else if (val == 2)
  {
    p = "等待确认";
  }
  else if (val == 3)
  {
    p = "信标转发";
  }  
  else 
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetNwkCmdStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取网络层命令字符串
*——————————————————————————*/
const u8 * GetNwkCmdStr(u8 val)
{  
  const u8 * p;

  if (val == 0x10)
  {
    p = "邻居收集请求";
  }
  else if (val == 0x11)
  {
    p = "邻居收集应答";
  }
  else if (val == 0x12)
  {
    p = "节点设置请求";
  }
  else if (val == 0x13)
  {
    p = "节点设置应答";
  }  
  else 
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetPhySpiStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取物理层SPI字符串
*——————————————————————————*/
const u8 * GetPhySpiStr(u8 val)
{  
  const u8 * p;

  if (val == 0)
  {
    p = "正常";
  }
  else if (val == 1)
  {
    p = "初始化超时";
  }
  else if (val == 2)
  {
    p = "COMM错误";
  }
  else if (val == 3)
  {
    p = "发送超时";
  }
  else if (val == 4)
  {
    p = "接收超时";
  }
  else if (val == 5)
  {
    p = "寄存器值错误";
  }
  else if (val == 6)
  {
    p = "寄存器应答错误";
  }  
  else 
  {
    p = "未知";
  }

  return p;
}

/*——————————————————————————
* 函 数 名：GetVssBlockStaStr
* 输入参数：val    传入值
* 输出参数：None
* 返 回 值：字符串
* 功能说明：获取VSS块状态字符串
*——————————————————————————*/
const u8 * GetVssBlockStaStr(u8 val)
{
  const u8 *p = NULL;

  if (0 == val)
  {
    p = "待擦除";
  }
  else if (1 == val)
  {
    p = "全空";
  }
  else if(2 == val)
  {
    p = "全满";
  }
  else if (3 == val)
  {
    p = "有效未满";
  }
  else if (4 == val)
  {
    p = "待回收";
  }
  else
  {
    p = "未知";
  }

  return p;
}