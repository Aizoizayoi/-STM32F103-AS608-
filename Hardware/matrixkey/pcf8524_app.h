#ifndef __PCF8524_APP_H__
#define __PCF8524_APP_H__

#include "matrixkey/pcf8524_drv.h"

#define pcf8524_MAX_BUF 127		//max buffer store length
#define pcf8524_MAX_LEN 6			//max needed length

pcf8524_StatusTypeDef PCF8524_Get09Password(uint8_t *pwd,uint8_t pwdlen);
uint8_t Key_09Map(uint8_t key);

#endif




