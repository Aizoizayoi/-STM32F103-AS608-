#include "stm32f10x.h"                  // Device header
#include "AS608.h"
#include "AS608_USART_Ins.h"
#include "AS608_USART.h"
#include "Delay.h"
#include "string.h"


uint32_t Address = 0xFFFFFFFF;

/**
  * @brief  发送一个字节
  * @param  Data: 要发送的数据
  * @retval None
  */
void AS608_SendData(uint8_t Data)
{
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
	USART_SendData(USART2, Data);
}

/**
  * @brief  发送包头 EF01
  * @param  None
  * @retval None
  */
void AS608_SendHead(void)
{
	AS608_SendData(0xEF);
	AS608_SendData(0x01);
}

/**
  * @brief  发送芯片地址
  * @param  None
  * @retval None
  */
void AS608_SendAddress(void)
{
	AS608_SendData(Address >> 24);
	AS608_SendData(Address >> 16);
	AS608_SendData(Address >> 8);
	AS608_SendData(Address);
}

/**
  * @brief  发送包标识
  * @param  PID: 包标识
  * @retval None
  */
void AS608_SendPID(uint8_t PID)
{
	AS608_SendData(PID);
}

/**
  * @brief  发送包长度
  * @param  Len: 包长度（指令码到校验和的字节数）
  * @retval None
  */
void AS608_SendLen(uint16_t Len)
{
	AS608_SendData(Len >> 8);
	AS608_SendData(Len);
}

/**
  * @brief  发送指令码
  * @param  Command: 指令码
  * @retval None
  */
void AS608_SendCommand(uint8_t Command)
{
	AS608_SendData(Command);
}

/**
  * @brief  发送字符串数据
  * @param  String: 数据指针
  * @param  Len: 数据长度
  * @retval None
  */
void AS608_SendString(uint8_t *String, uint8_t Len)
{
	uint8_t i = 0;
	for(i = 0; i < Len; i++)
	{
		AS608_SendData(String[i]);
	}
}

/**
  * @brief  发送校验和
  * @param  Check: 校验和（包标识到校验和前所有字节之和）
  * @retval None
  */
void AS608_SendCheck(uint16_t Check)
{
	AS608_SendData(Check >> 8);
	AS608_SendData(Check);
}



/**
  * @brief  验证并接收应答包
  * @param  Timeout: 超时时间（ms）
  * @retval 成功返回接收缓冲区指针，失败返回 0
  */
uint8_t *VertifyAck(uint16_t Timeout)
{
	while(--Timeout)
	{
		Delay_ms(1);
		if((USART2_RX_STA & 0x8000) == 0)			//未收到数据
			continue;

		if(USART2_RX_BUF[0] != 0xEF ||
		   USART2_RX_BUF[1] != 0x01 ||
		   USART2_RX_BUF[2] != (uint8_t)(Address >> 24) ||
		   USART2_RX_BUF[3] != (uint8_t)(Address >> 16) ||
		   USART2_RX_BUF[4] != (uint8_t)(Address >> 8) ||
		   USART2_RX_BUF[5] != (uint8_t)(Address) ||
		   USART2_RX_BUF[6] != 0x07)
		{
			USART_Reset();
			return 0;
		}

		USART_Reset();
		return USART2_RX_BUF;
	}
	USART_Reset();
	return 0;
}

/**
  * @brief  录入图像
  * @param  None
  * @retval Result: 确认码
  *   @arg 00H: 录入成功
  *   @arg 01H: 收包有错
  *   @arg 02H: 传感器上无手指
  *   @arg 03H: 录入不成功
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_GetImage(void)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_GetImage);
	Length = 0x01 + 0x03 + PS_GetImage;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  生成特征
  * @param  BufferID: 特征缓冲区号
  *   @arg 01H: CharBuffer1
  *   @arg 02H: CharBuffer2
  * @retval Result: 确认码
  *   @arg 00H: 生成特征成功
  *   @arg 其它: 生成特征失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_GenChar(uint8_t BufferID)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x04);
	AS608_SendCommand(PS_GenChar);
	AS608_SendData(BufferID);
	Length = 0x01 + 0x04 + PS_GenChar + BufferID;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  精确比对 CharBuffer1 与 CharBuffer2
  * @param  Score: 返回匹配得分（可为 NULL）
  * @retval Result: 确认码
  *   @arg 00H: 指纹匹配
  *   @arg 08H: 指纹不匹配
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_Match(uint16_t *Score)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_Match);
	Length = 0x01 + 0x03 + PS_Match;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		if(Score != NULL)
			*Score = ((uint16_t)Data[10] << 8) | (uint16_t)Data[11];
		Result = Data[9];
	}
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  搜索指纹
  * @param  BufferID: 特征缓冲区号
  *   @arg 01H: CharBuffer1
  *   @arg 02H: CharBuffer2
  * @param  StartPage: 起始页
  * @param  PageNum: 页数
  * @param  PageID: 返回页码（可为 NULL）
  * @param  MatchScore: 返回匹配得分（可为 NULL）
  * @retval Result: 确认码
  *   @arg 00H: 搜索到
  *   @arg 09H: 没搜索到
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_Search(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum,
						uint16_t *PageID, uint16_t *MatchScore)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x08);
	AS608_SendCommand(PS_Search);
	AS608_SendData(BufferID);
	AS608_SendData(StartPage >> 8);
	AS608_SendData(StartPage);
	AS608_SendData(PageNum >> 8);
	AS608_SendData(PageNum);
	Length = 0x01 + 0x08 + PS_Search + BufferID
		   + (StartPage >> 8) + (StartPage & 0xFF)
		   + (PageNum >> 8) + (PageNum & 0xFF);
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		if(PageID != NULL)
			*PageID = ((uint16_t)Data[10] << 8) | (uint16_t)Data[11];
		if(MatchScore != NULL)
			*MatchScore = ((uint16_t)Data[12] << 8) | (uint16_t)Data[13];
		Result = Data[9];
	}
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  合并特征（生成模板）
  * @param  None
  * @retval Result: 确认码
  *   @arg 00H: 合并成功
  *   @arg 0AH: 合并失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_RegModel(void)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_RegModel);
	Length = 0x01 + 0x03 + PS_RegModel;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  储存模板
  * @param  BufferID: 缓冲区号
  *   @arg 01H: CharBuffer1
  *   @arg 02H: CharBuffer2
  * @param  PageID: 指纹库位置号
  * @retval Result: 确认码
  *   @arg 00H: 储存成功
  *   @arg 其它: 储存失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_StoreChar(uint8_t BufferID, uint16_t PageID)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x06);
	AS608_SendCommand(PS_StoreChar);
	AS608_SendData(BufferID);
	AS608_SendData(PageID >> 8);
	AS608_SendData(PageID);
	Length = 0x01 + 0x06 + PS_StoreChar + BufferID
		   + (PageID >> 8) + (PageID & 0xFF);
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  读出模板
  * @param  BufferID: 缓冲区号
  *   @arg 01H: CharBuffer1
  *   @arg 02H: CharBuffer2
  * @param  PageID: 指纹库位置号
  * @retval Result: 确认码
  *   @arg 00H: 读出成功
  *   @arg 其它: 读出失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_LoadChar(uint8_t BufferID, uint16_t PageID)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x06);
	AS608_SendCommand(PS_LoadChar);
	AS608_SendData(BufferID);
	AS608_SendData(PageID >> 8);
	AS608_SendData(PageID);
	Length = 0x01 + 0x06 + PS_LoadChar + BufferID
		   + (PageID >> 8) + (PageID & 0xFF);
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  上传特征或模板
  * @param  BufferID: 缓冲区号
  *   @arg 01H: CharBuffer1
  *   @arg 02H: CharBuffer2
  * @retval Result: 确认码
  *   @arg 00H: 随后紧跟数据包
  *   @arg 其它: 不能发送后续数据包
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_UpChar(uint8_t BufferID)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x04);
	AS608_SendCommand(PS_UpChar);
	AS608_SendData(BufferID);
	Length = 0x01 + 0x04 + PS_UpChar + BufferID;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  下载特征或模板
  * @param  BufferID: 缓冲区号
  *   @arg 01H: CharBuffer1
  *   @arg 02H: CharBuffer2
  * @retval Result: 确认码
  *   @arg 00H: 可以接收后续数据包
  *   @arg 其它: 不能接收后续数据包
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_DownChar(uint8_t BufferID)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x04);
	AS608_SendCommand(PS_DownChar);
	AS608_SendData(BufferID);
	Length = 0x01 + 0x04 + PS_DownChar + BufferID;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  上传原始图像
  * @param  None
  * @retval Result: 确认码
  *   @arg 00H: 随后紧跟数据包
  *   @arg 其它: 不能发送后续数据包
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_UpImage(void)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_UpImage);
	Length = 0x01 + 0x03 + PS_UpImage;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  下载原始图像
  * @param  None
  * @retval Result: 确认码
  *   @arg 00H: 可以接收后续数据包
  *   @arg 其它: 不能接收后续数据包
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_DownImage(void)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_DownImage);
	Length = 0x01 + 0x03 + PS_DownImage;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  删除模板
  * @param  PageID: 指纹库起始位置号
  * @param  N: 删除的模板个数
  * @retval Result: 确认码
  *   @arg 00H: 删除成功
  *   @arg 其它: 删除失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_DeletChar(uint16_t PageID, uint16_t N)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x07);
	AS608_SendCommand(PS_DeletChar);
	AS608_SendData(PageID >> 8);
	AS608_SendData(PageID);
	AS608_SendData(N >> 8);
	AS608_SendData(N);
	Length = 0x01 + 0x07 + PS_DeletChar
		   + (PageID >> 8) + (PageID & 0xFF)
		   + (N >> 8) + (N & 0xFF);
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  清空指纹库
  * @param  None
  * @retval Result: 确认码
  *   @arg 00H: 清空成功
  *   @arg 其它: 清空失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_Empty(void)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_Empty);
	Length = 0x01 + 0x03 + PS_Empty;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  写系统寄存器
  * @param  RegNum: 寄存器序号
  *   @arg 4: 波特率控制寄存器
  *   @arg 5: 比对等级寄存器（1-6）
  *   @arg 6: 包大小寄存器（0-3）
  * @param  VALUE: 写入值
  *   @arg 波特率: N（实际波特率 = 9600 * N）
  *   @arg 比对等级: 1-6
  *   @arg 包大小: 0-3
  * @retval Result: 确认码
  *   @arg 00H: 成功
  *   @arg 其它: 失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_WriteReg(uint8_t RegNum, uint8_t VALUE)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x05);
	AS608_SendCommand(PS_WriteReg);
	AS608_SendData(RegNum);
	AS608_SendData(VALUE);
	Length = 0x01 + 0x05 + PS_WriteReg + RegNum + VALUE;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  读系统基本参数
  * @param  Para: 基本参数缓冲区（16 字节，可为 NULL）
  * @retval Result: 确认码
  *   @arg 00H: 成功
  *   @arg 其它: 失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_ReadSysPara(uint8_t *Para)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;
	uint8_t i;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_ReadSysPara);
	Length = 0x01 + 0x03 + PS_ReadSysPara;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		if(Para != NULL)
		{
			for(i = 0; i < 16; i++)
			{
				Para[i] = Data[10 + i];
			}
		}
		Result = Data[9];
	}
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  自动注册模板
  * @param  PageID: 返回存放页码（可为 NULL）
  * @retval Result: 确认码
  *   @arg 00H: 注册成功
  *   @arg 其它: 注册失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_Enroll(uint16_t *PageID)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_Enroll);
	Length = 0x01 + 0x03 + PS_Enroll;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		if(PageID != NULL)
			*PageID = ((uint16_t)Data[10] << 8) | (uint16_t)Data[11];
		Result = Data[9];
	}
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  自动验证指纹
  * @param  PageID: 返回页码（可为 NULL）
  * @param  MatchScore: 返回匹配得分（可为 NULL）
  * @retval Result: 确认码
  *   @arg 00H: 搜索到
  *   @arg 其它: 失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_Identify(uint16_t *PageID, uint16_t *MatchScore)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_Identify);
	Length = 0x01 + 0x03 + PS_Identify;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		if(PageID != NULL)
			*PageID = ((uint16_t)Data[10] << 8) | (uint16_t)Data[11];
		if(MatchScore != NULL)
			*MatchScore = ((uint16_t)Data[12] << 8) | (uint16_t)Data[13];
		Result = Data[9];
	}
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  设置设备握手口令
  * @param  Password: 口令（4 字节）
  * @retval Result: 确认码
  *   @arg 00H: 成功
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_SetPwd(uint32_t Password)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;
	uint8_t pwd0, pwd1, pwd2, pwd3;

	pwd0 = (Password >> 24) & 0xFF;
	pwd1 = (Password >> 16) & 0xFF;
	pwd2 = (Password >> 8) & 0xFF;
	pwd3 = Password & 0xFF;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x07);
	AS608_SendCommand(PS_SetPwd);
	AS608_SendData(pwd0);
	AS608_SendData(pwd1);
	AS608_SendData(pwd2);
	AS608_SendData(pwd3);
	Length = 0x01 + 0x07 + PS_SetPwd + pwd0 + pwd1 + pwd2 + pwd3;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  验证设备握手口令
  * @param  Password: 口令（4 字节）
  * @retval Result: 确认码
  *   @arg 00H: 口令正确
  *   @arg 13H: 口令不正确
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_VfyPwd(uint32_t Password)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;
	uint8_t pwd0, pwd1, pwd2, pwd3;

	pwd0 = (Password >> 24) & 0xFF;
	pwd1 = (Password >> 16) & 0xFF;
	pwd2 = (Password >> 8) & 0xFF;
	pwd3 = Password & 0xFF;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x07);
	AS608_SendCommand(PS_VfyPwd);
	AS608_SendData(pwd0);
	AS608_SendData(pwd1);
	AS608_SendData(pwd2);
	AS608_SendData(pwd3);
	Length = 0x01 + 0x07 + PS_VfyPwd + pwd0 + pwd1 + pwd2 + pwd3;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  采样随机数
  * @param  Random: 返回 4 字节随机数（可为 NULL）
  * @retval Result: 确认码
  *   @arg 00H: 成功
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_GetRandomCode(uint8_t *Random)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;
	uint8_t i;
	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_GetRandomCode);
	Length = 0x01 + 0x03 + PS_GetRandomCode;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		if(Random != NULL)
		{
			for(i = 0; i < 4; i++)
			{
				Random[i] = Data[10 + i];
			}
		}
		Result = Data[9];
	}
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  设置芯片地址
  * @param  NewAddr: 新芯片地址
  * @retval Result: 确认码
  *   @arg 00H: 设置成功
  *   @arg 01H: 收包有错
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_SetChipAddr(uint32_t NewAddr)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;
	uint8_t a0, a1, a2, a3;

	a0 = (NewAddr >> 24) & 0xFF;
	a1 = (NewAddr >> 16) & 0xFF;
	a2 = (NewAddr >> 8) & 0xFF;
	a3 = NewAddr & 0xFF;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x07);
	AS608_SendCommand(PS_SetChipAddr);
	AS608_SendData(a0);
	AS608_SendData(a1);
	AS608_SendData(a2);
	AS608_SendData(a3);
	Length = 0x01 + 0x07 + PS_SetChipAddr + a0 + a1 + a2 + a3;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		Result = Data[9];
		if(Result == AS608_OK)
			Address = NewAddr;
	}
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  读取 FLASH Information Page
  * @param  None
  * @retval Result: 确认码
  *   @arg 00H: 随后紧跟数据包
  *   @arg 其它: 失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_ReadINFpage(void)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_ReadINFpage);
	Length = 0x01 + 0x03 + PS_ReadINFpage;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  通讯端口控制
  * @param  ControlCode: 控制码
  *   @arg 0: 关闭端口
  *   @arg 1: 打开端口
  * @retval Result: 确认码
  *   @arg 00H: 成功
  *   @arg 其它: 失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_Port_Control(uint8_t ControlCode)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	if(ControlCode != 0 && ControlCode != 1)
		ControlCode = 1;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x04);
	AS608_SendCommand(PS_Port_Control);
	AS608_SendData(ControlCode);
	Length = 0x01 + 0x04 + PS_Port_Control + ControlCode;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  写记事本
  * @param  NotePageNum: 记事本页码（0-15）
  * @param  UserContent: 用户内容（32 字节）
  * @retval Result: 确认码
  *   @arg 00H: 成功
  *   @arg 其它: 失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_WriteNotepad(uint8_t NotePageNum, uint8_t *UserContent)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;
	uint8_t i;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x36);
	AS608_SendCommand(PS_WriteNotepad);
	AS608_SendData(NotePageNum);
	Length = 0x01 + 0x36 + PS_WriteNotepad + NotePageNum;
	for(i = 0; i < 32; i++)
	{
		uint8_t byte = (UserContent != NULL) ? UserContent[i] : 0;
		AS608_SendData(byte);
		Length += byte;
	}
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  读记事本
  * @param  NotePageNum: 记事本页码（0-15）
  * @param  UserContent: 用户内容缓冲区（32 字节，可为 NULL）
  * @retval Result: 确认码
  *   @arg 00H: 成功
  *   @arg 其它: 失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_ReadNotepad(uint8_t NotePageNum, uint8_t *UserContent)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;
	uint8_t i;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x04);
	AS608_SendCommand(PS_ReadNotepad);
	AS608_SendData(NotePageNum);
	Length = 0x01 + 0x04 + PS_ReadNotepad + NotePageNum;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		if(UserContent != NULL)
		{
			for(i = 0; i < 32; i++)
			{
				UserContent[i] = Data[10 + i];
			}
		}
		Result = Data[9];
	}
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  烧写片内 FLASH
  * @param  Mode: 升级模式
  *   @arg 0: 仅进行信息页升级
  *   @arg 1: 完整升级
  * @retval Result: 确认码
  *   @arg 00H: 可以接收后续数据包
  *   @arg 其它: 失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_BurnCode(uint8_t Mode)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x04);
	AS608_SendCommand(PS_BurnCode);
	AS608_SendData(Mode);
	Length = 0x01 + 0x04 + PS_BurnCode + Mode;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  高速搜索指纹
  * @param  BufferID: 特征缓冲区号
  *   @arg 01H: CharBuffer1
  *   @arg 02H: CharBuffer2
  * @param  StartPage: 起始页
  * @param  PageNum: 页数
  * @param  PageID: 返回页码（可为 NULL）
  * @param  MatchScore: 返回匹配得分（可为 NULL）
  * @retval Result: 确认码
  *   @arg 00H: 搜索到
  *   @arg 09H: 没搜索到
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_HighSpeedSearch(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum,
								 uint16_t *PageID, uint16_t *MatchScore)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x08);
	AS608_SendCommand(PS_HighSpeedSearch);
	AS608_SendData(BufferID);
	AS608_SendData(StartPage >> 8);
	AS608_SendData(StartPage);
	AS608_SendData(PageNum >> 8);
	AS608_SendData(PageNum);
	Length = 0x01 + 0x08 + PS_HighSpeedSearch + BufferID
		   + (StartPage >> 8) + (StartPage & 0xFF)
		   + (PageNum >> 8) + (PageNum & 0xFF);
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		if(PageID != NULL)
			*PageID = ((uint16_t)Data[10] << 8) | (uint16_t)Data[11];
		if(MatchScore != NULL)
			*MatchScore = ((uint16_t)Data[12] << 8) | (uint16_t)Data[13];
		Result = Data[9];
	}
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  生成二值化/细化指纹图像
  * @param  BinImgType: 图像类型
  *   @arg 0: 二值化图像
  *   @arg 1: 不含特征点标识的细化图像
  *   @arg 2或其他: 带特征点标识的细化图像
  * @retval Result: 确认码
  *   @arg 00H: 成功
  *   @arg 其它: 失败
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_GenBinImage(uint8_t BinImgType)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x04);
	AS608_SendCommand(PS_GenBinImage);
	AS608_SendData(BinImgType);
	Length = 0x01 + 0x04 + PS_GenBinImage + BinImgType;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
		Result = Data[9];
	else
		Result = 0xFF;
	return Result;
}

/**
  * @brief  读有效模板个数
  * @param  ValidN: 返回有效模板个数（可为 NULL）
  * @retval Result: 确认码
  *   @arg 00H: 成功
  *   @arg FFH: 通信失败
  */
uint8_t AS608_PS_ValidTempleteNum(uint16_t *ValidN)
{
	uint16_t Length;
	uint8_t *Data;
	uint8_t Result;

	AS608_SendHead();
	AS608_SendAddress();
	AS608_SendPID(0x01);
	AS608_SendLen(0x03);
	AS608_SendCommand(PS_ValidTempletenum);
	Length = 0x01 + 0x03 + PS_ValidTempletenum;
	AS608_SendCheck(Length);

	Data = VertifyAck(2000);
	if(Data)
	{
		if(ValidN != NULL)
			*ValidN = ((uint16_t)Data[10] << 8) | (uint16_t)Data[11];
		Result = Data[9];
	}
	else
		Result = 0xFF;
	return Result;
}


/**
  * @brief  将确认码转换为可读字符串（使用英文，避免 Keil/ARMCC 对 UTF-8 中文源文件解析出错）
  * @param  ensure: 模块返回的确认码
  * @retval 对应说明字符串
  */
const char *EnsureMessage(uint8_t ensure)
{
	const char *p;

	switch(ensure)
	{
		case 0x00: p = "OK"; break;
		case 0x01: p = "Packet receive error"; break;
		case 0x02: p = "No finger on sensor"; break;
		case 0x03: p = "Fail to enroll image"; break;
		case 0x04: p = "Image too dry/light"; break;
		case 0x05: p = "Image too wet/blurry"; break;
		case 0x06: p = "Image too messy"; break;
		case 0x07: p = "Feature points too few"; break;
		case 0x08: p = "Finger not match"; break;
		case 0x09: p = "Finger not found"; break;
		case 0x0A: p = "Fail to merge feature"; break;
		case 0x0B: p = "PageID out of range"; break;
		case 0x0C: p = "Read template error"; break;
		case 0x0D: p = "Upload feature fail"; break;
		case 0x0E: p = "Cannot accept more packets"; break;
		case 0x0F: p = "Upload image fail"; break;
		case 0x10: p = "Delete template fail"; break;
		case 0x11: p = "Clear library fail"; break;
		case 0x13: p = "Password incorrect"; break;
		case 0x15: p = "No valid image in buffer"; break;
		case 0x18: p = "FLASH read/write error"; break;
		case 0x19: p = "Undefined error"; break;
		case 0x1A: p = "Invalid register number"; break;
		case 0x1B: p = "Register content error"; break;
		case 0x1C: p = "Notepad page error"; break;
		case 0x1D: p = "Port operate fail"; break;
		case 0x1E: p = "Auto enroll fail"; break;
		case 0x1F: p = "Fingerprint library full"; break;
		default:   p = "Unknown ensure code"; break;
	}
	return p;
}
