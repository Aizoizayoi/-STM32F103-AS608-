#ifndef __AS608_USART_INS_H__
#define __AS608_USART_INS_H__

/* ------------------  Command Send---------------*/
#define PS_GetImage          0x01 // 功能：从传感器上读入图像存于图像缓冲区
#define PS_GenChar           0x02 // 功能：根据原始图像生成指纹特征存于 CharBuffer1 或 CharBuffer2
#define PS_Match             0x03 // 功能：精确比对 CharBuffer1 与 CharBuffer2 中的特征文件
#define PS_Search            0x04 // 功能：以 CharBuffer1 或 CharBuffer2 中的特征文件搜索整个或部分指纹库
#define PS_RegModel          0x05 // 功能：将 CharBuffer1 与 CharBuffer2 中的特征文件合并生成模板存于 CharBuffer2
#define PS_StoreChar         0x06 // 功能：将特征缓冲区中的文件储存到 flash 指纹库中
#define PS_LoadChar          0x07 // 功能：从 flash 指纹库中读取一个模板到特征缓冲区
#define PS_UpChar            0x08 // 功能：将特征缓冲区中的文件上传给上位机
#define PS_DownChar          0x09 // 功能：从上位机下载一个特征文件到特征缓冲区
#define PS_UpImage           0x0A // 功能：上传原始图像
#define PS_DownImage         0x0B // 功能：下载原始图像
#define PS_DeletChar         0x0C // 功能：删除 flash 指纹库中的一个特征文件
#define PS_Empty             0x0D // 功能：清空 flash 指纹库
#define PS_WriteReg          0x0E // 功能：写 SOC 系统寄存器
#define PS_ReadSysPara       0x0F // 功能：读系统基本参数
#define PS_Enroll            0x10 // 功能：注册模板
#define PS_Identify          0x11 // 功能：验证指纹
#define PS_SetPwd            0x12 // 功能：设置设备握手口令
#define PS_VfyPwd            0x13 // 功能：验证设备握手口令
#define PS_GetRandomCode     0x14 // 功能：采样随机数
#define PS_SetChipAddr       0x15 // 功能：设置芯片地址
#define PS_ReadINFpage       0x16 // 功能：读取 FLASH Information Page 内容
#define PS_Port_Control      0x17 // 功能：通讯端口（UART/USB）开关控制
#define PS_WriteNotepad      0x18 // 功能：写记事本
#define PS_ReadNotepad       0x19 // 功能：读记事本
#define PS_BurnCode          0x1A // 功能：烧写片内 FLASH（PS1802 SOC 该指令为烧写片外 FLASH 代码）
#define PS_HighSpeedSearch   0x1B // 功能：高速搜索 FLASH
#define PS_GenBinImage       0x1C // 功能：生成二值化指纹图像
#define PS_ValidTempletenum  0x1D // 功能：读有效模板个数
#define PS_UserGPIOCommand   0x1E // 功能：用户 GPIO 控制命令（PS1802-3 及以后版本适用）
#define PS_ReadIndexTable    0x1F // 功能：读索引表

/*---------------------------Receive Code----------------------------*/
#define AS608_OK                  0x00 // 功能：表示指令执行完毕或 OK
#define AS608_PACKET_ERROR        0x01 // 功能：表示数据包接收错误
#define AS608_NO_FINGER           0x02 // 功能：表示传感器上没有手指
#define AS608_ENROLL_FAIL         0x03 // 功能：表示录入指纹图像失败
#define AS608_IMAGE_TOO_DRY       0x04 // 功能：表示指纹图像太干、太淡而生成不成特征
#define AS608_IMAGE_TOO_WET       0x05 // 功能：表示指纹图像太湿、太糊而生成不成特征
#define AS608_IMAGE_TOO_MESSY     0x06 // 功能：表示指纹图像太乱而生成不成特征
#define AS608_FEAT_TOO_FEW        0x07 // 功能：表示指纹图像正常，但特征点太少（或面积太小）而生成不成特征
#define AS608_MATCH_FAIL          0x08 // 功能：表示指纹不匹配
#define AS608_NOT_FOUND           0x09 // 功能：表示没搜索到指纹
#define AS608_MERGE_FAIL          0x0A // 功能：表示特征合并失败
#define AS608_ADDR_OUT_OF_RANGE   0x0B // 功能：表示访问指纹库时地址序号超出指纹库范围
#define AS608_READ_TEMPLATE_FAIL  0x0C // 功能：表示从指纹库读模板出错或无效
#define AS608_UPLOAD_FEAT_FAIL    0x0D // 功能：表示上传特征失败
#define AS608_CANNOT_ACCEPT       0x0E // 功能：表示模块不能接受后续数据包
#define AS608_UPLOAD_IMAGE_FAIL   0x0F // 功能：表示上传图像失败
#define AS608_DELETE_FAIL         0x10 // 功能：表示删除模板失败
#define AS608_CLEAR_FAIL          0x11 // 功能：表示清空指纹库失败
#define AS608_PASSWORD_ERROR      0x13 // 功能：表示口令不正确
#define AS608_NO_VALID_IMAGE      0x15 // 功能：表示缓冲区内没有有效原始图而生成不成图像
#define AS608_FLASH_ERROR         0x18 // 功能：表示读写 FLASH 出错
#define AS608_UNDEFINED_ERROR     0x19 // 功能：表示未定义错误
#define AS608_INVALID_REG         0x1A // 功能：表示无效寄存器号
#define AS608_REG_CONTENT_ERROR   0x1B // 功能：表示寄存器设定内容错误号
#define AS608_NOTEPAD_PAGE_ERROR  0x1C // 功能：表示记事本页码指定错误
#define AS608_PORT_FAIL           0x1D // 功能：表示端口操作失败
#define AS608_AUTO_ENROLL_FAIL    0x1E // 功能：表示自动注册（enroll）失败
#define AS608_LIBRARY_FULL        0x1F // 功能：表示指纹库满
#endif
