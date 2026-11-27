#ifndef __AS608_APP_H__
#define __AS608_APP_H__

/** @defgroup AS608_DEFINE
  * @{
  */
#define AS_CharBuffer1 0x01				//特征缓冲区1
#define AS_CharBuffer2 0x02				//特征缓冲区2
#define AS_MAX_FingerStore 1023			//指纹库最大存储地址，范围0-1023
#define AS_MAX_FingerName_Len 126
#define AS_FP_MAP_NUM 10				//本机姓名表容量，与 AS608 页号一一对应


typedef struct
{
	uint16_t page_id;
	uint8_t used;						//0 空位，1 已录入。页号 0 是合法 ID，不能用 page_id==0 表示空
	uint8_t finger_name[10];
}as608_FPMap_t;


typedef enum
{
	Func_OK = 0,
	Func_timeout = 1,
	Func_error = 2,
	Func_retry = 3,
	Func_overflow = 4,
}as608_FunctionState_t;

typedef enum
{
	getfinger_start = 0,
	getfinger_getimage = 1,
	getfinger_genchar = 2,
	getfinger_match = 3,
	getfinger_regmodel = 4,
	getfinger_storechar = 5,
}as608_GetFinger_Step_t;

typedef enum
{
    FP_OK                        = 0x00, /* 指令执行完毕或 OK */
    FP_ERR_PKT_RECV              = 0x01, /* 数据包接收错误 */
    FP_ERR_NO_FINGER             = 0x02, /* 传感器上没有手指 */
    FP_ERR_IMG_ENROLL            = 0x03, /* 录入指纹图像失败 */
    FP_ERR_IMG_DRY_LIGHT         = 0x04, /* 指纹图像太干、太淡而生不成特征 */
    FP_ERR_IMG_WET_BLUR          = 0x05, /* 指纹图像太湿、太糊而生不成特征 */
    FP_ERR_IMG_CHAOTIC           = 0x06, /* 指纹图像太乱而生不成特征 */
    FP_ERR_IMG_SMALL_AREA        = 0x07, /* 指纹图像正常，但特征点太少（或面积太小）而生不成特征 */
    FP_ERR_NOT_MATCH             = 0x08, /* 指纹不匹配 */
    FP_ERR_NOT_FOUND             = 0x09, /* 没搜索到指纹 */
    FP_ERR_MERGE_FAIL            = 0x0A, /* 特征合并失败 */
    FP_ERR_ADDR_OUT_OF_RANGE     = 0x0B, /* 访问指纹库时地址序号超出指纹库范围 */
    FP_ERR_READ_TPL_FAIL         = 0x0C, /* 从指纹库读模板出错或无效 */
    FP_ERR_UPLOAD_CHAR_FAIL      = 0x0D, /* 上传特征失败 */
    FP_ERR_NO_NEXT_PKT           = 0x0E, /* 模块不能接受后续数据包 */
    FP_ERR_UPLOAD_IMG_FAIL       = 0x0F, /* 上传图像失败 */
    FP_ERR_DEL_TPL_FAIL          = 0x10, /* 删除模板失败 */
    FP_ERR_CLEAR_DB_FAIL         = 0x11, /* 清空指纹库失败 */
    FP_ERR_LOW_POWER_FAIL        = 0x12, /* 不能进入低功耗状态 */
    FP_ERR_PWD_INVALID           = 0x13, /* 口令不正确 */
    FP_ERR_RESET_FAIL            = 0x14, /* 系统复位失败 */
    FP_ERR_NO_RAW_IMG            = 0x15, /* 缓冲区内没有有效原始图而生不成图像 */
    FP_ERR_UPGRADE_FAIL          = 0x16, /* 在线升级失败 */
    FP_ERR_FINGER_NOT_MOVED      = 0x17, /* 残留指纹或两次采集之间手指没有移动过 */
    FP_ERR_FLASH_RW              = 0x18, /* 读写 FLASH 出错 */
    FP_ERR_UNDEF                 = 0x19, /* 未定义错误 */
    FP_ERR_INVALID_REG           = 0x1A, /* 无效寄存器号 */
    FP_ERR_INVALID_REG_VAL       = 0x1B, /* 寄存器设定内容错误号 */
    FP_ERR_NOTE_PAGE_ERR         = 0x1C, /* 记事本页码指定错误 */
    FP_ERR_PORT_OP_FAIL          = 0x1D, /* 端口操作失败 */
    FP_ERR_AUTO_ENROLL_FAIL      = 0x1E, /* 自动注册（enroll）失败 */
    FP_ERR_DB_FULL               = 0x1F, /* 指纹库满 */

    FP_ACK_NEXT_PKT_DATA         = 0xF0, /* 有后续数据包指令，正确接收后应答 */
    FP_ACK_NEXT_PKT_CMD          = 0xF1, /* 有后续数据包指令，命令包应答 */
    FP_ERR_FLASH_CHECKSUM        = 0xF2, /* 烧写内部 FLASH 时，校验和错误 */
    FP_ERR_FLASH_PKT_ID          = 0xF3, /* 烧写内部 FLASH 时，包标识错误 */
    FP_ERR_FLASH_PKT_LEN         = 0xF4, /* 烧写内部 FLASH 时，包长度错误 */
    FP_ERR_FLASH_CODE_TOO_LONG   = 0xF5, /* 烧写内部 FLASH 时，代码长度太长 */
    FP_ERR_FLASH_BURN_FAIL       = 0xF6  /* 烧写内部 FLASH 时，烧写 FLASH 失败 */
} fp_AckDefine_t;

void AS608_UI(void);
void Get_FingerPrint(uint16_t PageID);
void PrintAckMessage(fp_AckDefine_t Ack);
void MatchFinger(void);
fp_AckDefine_t DeleteFinger(uint16_t PageID, uint16_t N);
void EmptyFinger(void);
as608_FunctionState_t StringToNumber(uint8_t *String, uint16_t *Number);
as608_FunctionState_t AS608_ReceiveNumber(uint16_t *Number);
as608_FunctionState_t AS608_ReceiveString(uint8_t *String);
void AS608_ShowMenu(void);
#endif
