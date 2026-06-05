#ifndef __TOUCHSCREEN_H
#define __TOUCHSCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"
#include "stm32h7xx_hal.h"
#include "stm32h750b_discovery_lcd.h"
#include "stm32h750b_discovery_ts.h"
#include "stm32_lcd.h"

void Touchscreen_Init(void);
void Touchscreen_Poll(void);
void Touchscreen_SetAlarmStatus(const char *status);
const char* Touchscreen_GetPINBuffer(void);
uint8_t Touchscreen_ValidatePIN(void);
void Touchscreen_ResetPIN(void);

#ifdef __cplusplus
}
#endif

#endif /* __TOUCHSCREEN_H */
