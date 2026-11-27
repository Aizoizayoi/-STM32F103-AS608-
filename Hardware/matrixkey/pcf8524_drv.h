#ifndef __PCF8524_DRV_H__
#define __PCF8524_DRV_H__

#include <stdint.h>

#define pcf8524_add 0x41 //W 0x40,R 0x41

typedef enum
{
  pcf8524_OK       = 0x00U,
  pcf8524_ERROR    = 0x01U,
  pcf8524_BUSY     = 0x02U,
  pcf8524_TIMEOUT  = 0x03U
} pcf8524_StatusTypeDef;


uint8_t list(uint8_t rx);
pcf8524_StatusTypeDef PCF8574_GetButton(uint8_t *button);
pcf8524_StatusTypeDef PCF8524_Get1Char(char *ch);

#endif




