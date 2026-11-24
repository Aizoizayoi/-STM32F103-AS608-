#ifndef __AS608_H__
#define __AS608_H__

#include "stm32f10x.h"

extern uint32_t Address;

void AS608_SendData(uint8_t Data);
void AS608_SendHead(void);
void AS608_SendAddress(void);
void AS608_SendPID(uint8_t PID);
void AS608_SendLen(uint16_t Len);
void AS608_SendCommand(uint8_t Command);
void AS608_SendString(uint8_t *String, uint8_t Len);
void AS608_SendCheck(uint16_t Check);
uint8_t *VertifyAck(uint16_t Timeout);

uint8_t AS608_PS_GetImage(void);
uint8_t AS608_PS_GenChar(uint8_t BufferID);
uint8_t AS608_PS_Match(uint16_t *Score);
uint8_t AS608_PS_Search(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum,
						uint16_t *PageID, uint16_t *MatchScore);
uint8_t AS608_PS_RegModel(void);
uint8_t AS608_PS_StoreChar(uint8_t BufferID, uint16_t PageID);
uint8_t AS608_PS_LoadChar(uint8_t BufferID, uint16_t PageID);
uint8_t AS608_PS_UpChar(uint8_t BufferID);
uint8_t AS608_PS_DownChar(uint8_t BufferID);
uint8_t AS608_PS_UpImage(void);
uint8_t AS608_PS_DownImage(void);
uint8_t AS608_PS_DeletChar(uint16_t PageID, uint16_t N);
uint8_t AS608_PS_Empty(void);
uint8_t AS608_PS_WriteReg(uint8_t RegNum, uint8_t VALUE);
uint8_t AS608_PS_ReadSysPara(uint8_t *Para);
uint8_t AS608_PS_Enroll(uint16_t *PageID);
uint8_t AS608_PS_Identify(uint16_t *PageID, uint16_t *MatchScore);
uint8_t AS608_PS_SetPwd(uint32_t Password);
uint8_t AS608_PS_VfyPwd(uint32_t Password);
uint8_t AS608_PS_GetRandomCode(uint8_t *Random);
uint8_t AS608_PS_SetChipAddr(uint32_t NewAddr);
uint8_t AS608_PS_ReadINFpage(void);
uint8_t AS608_PS_Port_Control(uint8_t ControlCode);
uint8_t AS608_PS_WriteNotepad(uint8_t NotePageNum, uint8_t *UserContent);
uint8_t AS608_PS_ReadNotepad(uint8_t NotePageNum, uint8_t *UserContent);
uint8_t AS608_PS_BurnCode(uint8_t Mode);
uint8_t AS608_PS_HighSpeedSearch(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum,
								 uint16_t *PageID, uint16_t *MatchScore);
uint8_t AS608_PS_GenBinImage(uint8_t BinImgType);
uint8_t AS608_PS_ValidTempleteNum(uint16_t *ValidN);

const char *EnsureMessage(uint8_t ensure) ;
#endif
