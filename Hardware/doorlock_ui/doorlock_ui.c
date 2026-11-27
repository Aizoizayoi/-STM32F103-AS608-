#include "doorlock_ui/doorlock_ui.h"
#include <stdio.h>

//typedef enum
//{
//	DL_IDLE = 0;
//	密码输入中 = 1；
//	密码输入完成=2；
//	指纹输入中=3；
//	指纹输入完成=4；
//	密码添加中=6
//	密码密码删除中=7
//	密码查询中=8
//	指纹删除中=9
//	指纹添加中=10
//	指纹查询中=11
//	封锁中=12

//}doorlck_state_t;


void Doorlock_ShowMenu(void)
{
	printf("0.administrator mode.\r\n");
	printf("1.change password.\r\n");
	printf("2.Add Password.\r\n");
	printf("3.Password find.\r\n");
	printf("4.Add Fingerprint.\r\n");
	printf("5.Delete fingerprints.\r\n");
	printf("6.fingerprints find.\r\n");
	printf("7.Exit.\r\n");
}

void DoorLock_UI(void)
{
	

}
