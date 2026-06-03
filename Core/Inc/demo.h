#ifndef __DEMO_H
#define __DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"
#include "stm32h7xx_hal.h"
#include "stm32h750b_discovery_lcd.h"
#include "stm32h750b_discovery_ts.h"
#include "stm32_lcd.h"

void Touchscreen_demo(void);
void simple_demo(void);
void Error_Handler(void);
void Touchscreen_DrawBackground_Circles(uint8_t state);
uint8_t TouchScreen_GetTouchPosition(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEMO_H */
