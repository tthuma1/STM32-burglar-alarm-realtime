#include "demo.h"

#define  CIRCLE_RADIUS        40
#define  LINE_LENGHT          30

#define  CIRCLE_XPOS(i)       ((i * 480) / 5)
#define  CIRCLE_YPOS(i)       (272 - CIRCLE_RADIUS - 60)

TS_State_t  TS_State;
TS_MultiTouch_State_t TS_MultiTouchState;

TS_Init_t hTS;

uint32_t num_of_touches = 0;

void Touchscreen_Init(void)
{
    uint32_t x_size, y_size;
    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    hTS.Width       = x_size;
    hTS.Height      = y_size;
    hTS.Orientation = TS_SWAP_XY;
    hTS.Accuracy    = 5;

    BSP_TS_Init(0, &hTS);

    UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
    UTIL_LCD_FillRect(0, 0, x_size, 80, UTIL_LCD_COLOR_BLUE);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
    UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLUE);
    UTIL_LCD_SetFont(&Font24);
    UTIL_LCD_DisplayStringAt(0, 30, (uint8_t *)"Touchscreen demo", CENTER_MODE);
    Touchscreen_DrawBackground_Circles(0);
}
void Touchscreen_Poll(uint8_t *state)
{
    BSP_TS_GetState(0, &TS_State);
    if (!TS_State.TouchDetected) return;

    uint16_t x1 = TS_State.TouchX;
    uint16_t y1 = TS_State.TouchY;

    if ((y1 > (CIRCLE_YPOS(1) - CIRCLE_RADIUS)) &&
        (y1 < (CIRCLE_YPOS(1) + CIRCLE_RADIUS)))
    {
      if ((x1 > (CIRCLE_XPOS(1) - CIRCLE_RADIUS)) &&
          (x1 < (CIRCLE_XPOS(1) + CIRCLE_RADIUS)))
      {
        if ((*state & 1) == 0)
        {
          Touchscreen_DrawBackground_Circles(*state);
          UTIL_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS, UTIL_LCD_COLOR_BLUE);
          *state = 1;
        }
      }
      if ((x1 > (CIRCLE_XPOS(2) - CIRCLE_RADIUS)) &&
          (x1 < (CIRCLE_XPOS(2) + CIRCLE_RADIUS)))
      {
        if ((*state & 2) == 0)
        {
          Touchscreen_DrawBackground_Circles(*state);
          UTIL_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS, UTIL_LCD_COLOR_RED);
          *state = 2;
        }
      }

      if ((x1 > (CIRCLE_XPOS(3) - CIRCLE_RADIUS)) &&
          (x1 < (CIRCLE_XPOS(3) + CIRCLE_RADIUS)))
      {
        if ((*state & 4) == 0)
        {
          Touchscreen_DrawBackground_Circles(*state);
          UTIL_LCD_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS, UTIL_LCD_COLOR_YELLOW);
          *state = 4;
        }
      }

      if ((x1 > (CIRCLE_XPOS(4) - CIRCLE_RADIUS)) &&
          (x1 < (CIRCLE_XPOS(4) + CIRCLE_RADIUS)))
      {
        if ((*state & 8) == 0)
        {
          Touchscreen_DrawBackground_Circles(*state);
          UTIL_LCD_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS, UTIL_LCD_COLOR_GREEN);
          *state = 8;
        }
      }
    }
}

/**
  * @brief  Draw Touchscreen Background
  * @param  state : touch zone state
  * @retval None
  */
void Touchscreen_DrawBackground_Circles(uint8_t state)
{
  uint16_t x, y;
  switch (state)
  {
    case 0:
      UTIL_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS, UTIL_LCD_COLOR_BLUE);

      UTIL_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS, UTIL_LCD_COLOR_RED);

      UTIL_LCD_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS, UTIL_LCD_COLOR_YELLOW);


      UTIL_LCD_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS, UTIL_LCD_COLOR_GREEN);

      UTIL_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS - 2, UTIL_LCD_COLOR_WHITE);
      UTIL_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS - 2, UTIL_LCD_COLOR_WHITE);
      UTIL_LCD_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, UTIL_LCD_COLOR_WHITE);
      UTIL_LCD_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, UTIL_LCD_COLOR_WHITE);
      break;

    case 1:
      UTIL_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS, UTIL_LCD_COLOR_BLUE);
      UTIL_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS - 2, UTIL_LCD_COLOR_WHITE);
      break;

    case 2:
      UTIL_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS, UTIL_LCD_COLOR_RED);
      UTIL_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS - 2, UTIL_LCD_COLOR_WHITE);
      break;

    case 4:
      UTIL_LCD_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS, UTIL_LCD_COLOR_YELLOW);
      UTIL_LCD_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, UTIL_LCD_COLOR_WHITE);
      break;

    case 8:
      UTIL_LCD_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(4), CIRCLE_RADIUS, UTIL_LCD_COLOR_GREEN);
      UTIL_LCD_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(4), CIRCLE_RADIUS - 2, UTIL_LCD_COLOR_WHITE);
      break;
  }
}

/**
  * @brief  TouchScreen get touch position
  * @param  None
  * @retval None
  */
uint8_t TouchScreen_GetTouchPosition(void)
{
  uint16_t x1, y1;
  uint8_t circleNr = 0;

  /* Check in polling mode in touch screen the touch status and coordinates */
  /* of touches if touch occurred                                           */
  BSP_TS_GetState(0, &TS_State);
  if(TS_State.TouchDetected)
  {
    /* One or dual touch have been detected          */
    /* Only take into account the first touch so far */

    /* Get X and Y position of the first */
    x1 = TS_State.TouchX;
    y1 = TS_State.TouchY;

    if ((y1 > (CIRCLE_YPOS(1) - CIRCLE_RADIUS)) &&
        (y1 < (CIRCLE_YPOS(1) + CIRCLE_RADIUS)))
    {
      if ((x1 > (CIRCLE_XPOS(1) - CIRCLE_RADIUS)) &&
          (x1 < (CIRCLE_XPOS(1) + CIRCLE_RADIUS)))
      {
        circleNr = 1;
      }
      if ((x1 > (CIRCLE_XPOS(2) - CIRCLE_RADIUS)) &&
          (x1 < (CIRCLE_XPOS(2) + CIRCLE_RADIUS)))
      {
        circleNr = 2;
      }

      if ((x1 > (CIRCLE_XPOS(3) - CIRCLE_RADIUS)) &&
          (x1 < (CIRCLE_XPOS(3) + CIRCLE_RADIUS)))
      {
        circleNr = 3;
      }

      if ((x1 > (CIRCLE_XPOS(4) - CIRCLE_RADIUS)) &&
          (x1 < (CIRCLE_XPOS(4) + CIRCLE_RADIUS)))
      {
        circleNr = 4;
      }
    }
    else
    {
      if (((y1 < 220) && (y1 > 140)) &&
          ((x1 < 160) && (x1 > 100)))
      {
        circleNr = 0xFE;   /* top part of the screen */
      }
      else
      {
        circleNr = 0xFF;
      }
    }
  } /* of if(TS_State.TouchDetected) */
  return circleNr;
}

/**
  * @}
  */

/**
  * @}
  */
