#include "touchscreen.h"
#include <stdbool.h>

#define SCREEN_WIDTH         480
#define SCREEN_HEIGHT        272

#define INPUT_RECT_X         20
#define INPUT_RECT_Y         10
#define INPUT_RECT_W         440
#define INPUT_RECT_H         60

#define BUTTON_START_X       40
#define BUTTON_START_Y       (INPUT_RECT_Y + INPUT_RECT_H + 8)
#define BUTTON_WIDTH         120
#define BUTTON_HEIGHT        42
#define BUTTON_X_SPACING     20
#define BUTTON_Y_SPACING     6

#define INPUT_DOT_RADIUS     8
#define INPUT_DOT_COUNT      4

#define TS_KEY_NONE          255
#define TS_KEY_BACK          254
#define TS_KEY_OK            253

static TS_State_t TS_State;
static TS_Init_t hTS;

static const char *keyLabels[4][3] = {
  { "1", "2", "3" },
  { "4", "5", "6" },
  { "7", "8", "9" },
  { "back", "0", "OK" }
};

static const uint8_t keyCodes[4][3] = {
  { 1, 2, 3 },
  { 4, 5, 6 },
  { 7, 8, 9 },
  { TS_KEY_BACK, 0, TS_KEY_OK }
};

static char inputBuffer[INPUT_DOT_COUNT + 1];
static uint8_t inputLength = 0;
static bool touchActive = false;
static char statusText[16] = "Enter PIN";

static void Touchscreen_DrawTextCentered(uint16_t x, uint16_t y, uint16_t width, const char *text)
{
  uint16_t textWidth = strlen(text) * 12u;
  uint16_t textX = x + (width > textWidth ? (width - textWidth) / 2 : 0);
  UTIL_LCD_DisplayStringAt(textX, y, (uint8_t *)text, LEFT_MODE);
}

static uint8_t Touchscreen_GetTouchedKey(uint16_t x, uint16_t y)
{
  for (uint8_t row = 0; row < 4; row++)
  {
    for (uint8_t col = 0; col < 3; col++)
    {
      uint16_t x0 = BUTTON_START_X + col * (BUTTON_WIDTH + BUTTON_X_SPACING);
      uint16_t y0 = BUTTON_START_Y + row * (BUTTON_HEIGHT + BUTTON_Y_SPACING);

      if ((x >= x0) && (x < x0 + BUTTON_WIDTH) &&
          (y >= y0) && (y < y0 + BUTTON_HEIGHT))
      {
        return keyCodes[row][col];
      }
    }
  }

  return TS_KEY_NONE;
}

static void Touchscreen_UpdateInputIndicators(void)
{
  uint16_t baseX = SCREEN_WIDTH / 2 - ((INPUT_DOT_COUNT * INPUT_DOT_RADIUS * 2 + (INPUT_DOT_COUNT - 1) * 14) / 2);
  uint16_t centerY = INPUT_RECT_Y + INPUT_RECT_H - 18;

  for (uint8_t i = 0; i < INPUT_DOT_COUNT; i++)
  {
    uint16_t dotX = baseX + i * (2 * INPUT_DOT_RADIUS + 14);
    UTIL_LCD_FillCircle(dotX, centerY, INPUT_DOT_RADIUS, UTIL_LCD_COLOR_WHITE);
    UTIL_LCD_DrawCircle(dotX, centerY, INPUT_DOT_RADIUS, UTIL_LCD_COLOR_BLACK);

    if (i < inputLength)
    {
      UTIL_LCD_FillCircle(dotX, centerY, INPUT_DOT_RADIUS - 4, UTIL_LCD_COLOR_BLACK);
    }
  }
}

static void Touchscreen_DrawStatusText(void)
{
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_LIGHTGRAY);
  UTIL_LCD_SetFont(&Font20);
  Touchscreen_DrawTextCentered(INPUT_RECT_X, INPUT_RECT_Y + 8, INPUT_RECT_W, statusText);
}

static void Touchscreen_DrawInputField(void)
{
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_DrawRect(INPUT_RECT_X, INPUT_RECT_Y, INPUT_RECT_W, INPUT_RECT_H, UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_FillRect(INPUT_RECT_X + 1, INPUT_RECT_Y + 1, INPUT_RECT_W - 2, INPUT_RECT_H - 2, UTIL_LCD_COLOR_LIGHTGRAY);
  Touchscreen_DrawStatusText();
  Touchscreen_UpdateInputIndicators();
}

static void Touchscreen_DrawButton(uint16_t x, uint16_t y, const char *label, uint32_t color)
{
  UTIL_LCD_FillRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, color);
  UTIL_LCD_DrawRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, UTIL_LCD_COLOR_BLACK);

  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_SetBackColor(color);
  UTIL_LCD_SetFont(&Font20);
  Touchscreen_DrawTextCentered(x, y + 10, BUTTON_WIDTH, label);
}

static void Touchscreen_DrawKeypad(void)
{
  for (uint8_t row = 0; row < 4; row++)
  {
    for (uint8_t col = 0; col < 3; col++)
    {
      uint16_t x0 = BUTTON_START_X + col * (BUTTON_WIDTH + BUTTON_X_SPACING);
      uint16_t y0 = BUTTON_START_Y + row * (BUTTON_HEIGHT + BUTTON_Y_SPACING);
      uint32_t color = UTIL_LCD_COLOR_WHITE;

      if (keyCodes[row][col] == TS_KEY_BACK)
      {
        color = UTIL_LCD_COLOR_LIGHTRED;
      }
      else if (keyCodes[row][col] == TS_KEY_OK)
      {
        color = UTIL_LCD_COLOR_LIGHTGREEN;
      }

      Touchscreen_DrawButton(x0, y0, keyLabels[row][col], color);
    }
  }
}

static void Touchscreen_ProcessKey(uint8_t key)
{
  if (key == TS_KEY_NONE)
  {
    return;
  }

  if (key == TS_KEY_BACK)
  {
    if (inputLength > 0)
    {
      inputLength--;
      inputBuffer[inputLength] = '\0';
      strncpy(statusText, "Enter PIN", sizeof(statusText));
    }
    else
    {
      strncpy(statusText, "Empty", sizeof(statusText));
    }
  }
  else if (key == TS_KEY_OK)
  {
    if (inputLength > 0)
    {
      strncpy(statusText, "OK", sizeof(statusText));
    }
    else
    {
      strncpy(statusText, "No input", sizeof(statusText));
    }
  }
  else if ((key <= 9) && (inputLength < INPUT_DOT_COUNT))
  {
    inputBuffer[inputLength++] = '0' + key;
    inputBuffer[inputLength] = '\0';
    strncpy(statusText, "Enter PIN", sizeof(statusText));
  }

  Touchscreen_DrawInputField();
}

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
  UTIL_LCD_FillRect(0, 0, SCREEN_WIDTH, 50, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetFont(&Font24);
  UTIL_LCD_DisplayStringAt(0, 14, (uint8_t *)"Numeric keypad", CENTER_MODE);

  inputLength = 0;
  memset(inputBuffer, 0, sizeof(inputBuffer));
  strncpy(statusText, "Enter PIN", sizeof(statusText));
  touchActive = false;

  Touchscreen_DrawInputField();
  Touchscreen_DrawKeypad();
}

void Touchscreen_Poll(void)
{
  BSP_TS_GetState(0, &TS_State);

  if (!TS_State.TouchDetected)
  {
    touchActive = false;
    return;
  }

  if (touchActive)
  {
    return;
  }

  touchActive = true;

  uint16_t x = TS_State.TouchX;
  uint16_t y = TS_State.TouchY;
  uint8_t key = Touchscreen_GetTouchedKey(x, y);
  Touchscreen_ProcessKey(key);
}
