#include "touchscreen.h"
#include "main.h"
#include <stdbool.h>

#define SCREEN_WIDTH         480
#define SCREEN_HEIGHT        272

#define INPUT_RECT_X         20
#define INPUT_RECT_Y         10
#define INPUT_RECT_W         440
#define INPUT_RECT_H         60

#define ALARM_STATUS_RECT_W  180
#define ALARM_STATUS_RECT_H  30

#define BUTTON_START_X       20
#define BUTTON_START_Y       (INPUT_RECT_Y + INPUT_RECT_H + 8)
#define BUTTON_WIDTH         60
#define BUTTON_HEIGHT        42
#define BUTTON_X_SPACING     10
#define BUTTON_Y_SPACING     6

#define SIDE_BUTTON_WIDTH    180
#define SIDE_BUTTON_HEIGHT   40
#define SIDE_BUTTON_X        (SCREEN_WIDTH - SIDE_BUTTON_WIDTH - 60)
#define ALARM_STATUS_RECT_X  (SIDE_BUTTON_X)
#define ALARM_STATUS_RECT_Y  (BUTTON_START_Y)
#define SIDE_BUTTON_Y        (ALARM_STATUS_RECT_Y + ALARM_STATUS_RECT_H + 10)

#define INPUT_DOT_RADIUS     8
#define INPUT_DOT_COUNT      4

#define TS_KEY_NONE          255
#define TS_KEY_BACK          254
#define TS_KEY_SIDE_CONTROL  252

static TS_State_t TS_State;
static TS_Init_t hTS;

static const char *keyLabels[4][3] = {
  { "1", "2", "3" },
  { "4", "5", "6" },
  { "7", "8", "9" },
  { "", "0", "B" }
};

static const uint8_t keyCodes[4][3] = {
  { 1, 2, 3 },
  { 4, 5, 6 },
  { 7, 8, 9 },
  { 10, 0, TS_KEY_BACK }
};

static const char *sideButtonLabels[2] = {
  "Turn On",
  "Turn Off",
};

static char inputBuffer[INPUT_DOT_COUNT + 1];
static uint8_t inputLength = 0;
static uint8_t sideButtonIndex = 0;
static bool touchActive = false;
static bool redrawPending = false;
static char alarmStatus[16] = "Alarm off";

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

  if ((x >= SIDE_BUTTON_X) && (x < SIDE_BUTTON_X + SIDE_BUTTON_WIDTH) &&
      (y >= SIDE_BUTTON_Y) && (y < SIDE_BUTTON_Y + SIDE_BUTTON_HEIGHT))
  {
    return TS_KEY_SIDE_CONTROL;
  }

  return TS_KEY_NONE;
}

static void Touchscreen_UpdateInputIndicators(void)
{
  uint16_t baseX = SCREEN_WIDTH / 2 - ((INPUT_DOT_COUNT * INPUT_DOT_RADIUS * 2 + (INPUT_DOT_COUNT - 1) * 14) / 2);
  uint16_t centerY = INPUT_RECT_Y + INPUT_RECT_H - 28;

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

// Removed status text display per request

static void Touchscreen_DrawAlarmStatus(void)
{
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetFont(&Font20);

  UTIL_LCD_DrawRect(ALARM_STATUS_RECT_X, ALARM_STATUS_RECT_Y, ALARM_STATUS_RECT_W, ALARM_STATUS_RECT_H, UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_FillRect(ALARM_STATUS_RECT_X + 1, ALARM_STATUS_RECT_Y + 1, ALARM_STATUS_RECT_W - 2, ALARM_STATUS_RECT_H - 2, UTIL_LCD_COLOR_WHITE);

  uint16_t textY = ALARM_STATUS_RECT_Y + (ALARM_STATUS_RECT_H - Font20.Height) / 2;
  Touchscreen_DrawTextCentered(ALARM_STATUS_RECT_X + 2, textY, ALARM_STATUS_RECT_W - 4, alarmStatus);
}

static void Touchscreen_DrawSideButton(void)
{
  uint32_t color = sideButtonIndex == 0 ? UTIL_LCD_COLOR_LIGHTGREEN : UTIL_LCD_COLOR_LIGHTRED;
  UTIL_LCD_FillRect(SIDE_BUTTON_X, SIDE_BUTTON_Y, SIDE_BUTTON_WIDTH, SIDE_BUTTON_HEIGHT, color);
  UTIL_LCD_DrawRect(SIDE_BUTTON_X, SIDE_BUTTON_Y, SIDE_BUTTON_WIDTH, SIDE_BUTTON_HEIGHT, UTIL_LCD_COLOR_BLACK);

  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_SetBackColor(color);
  UTIL_LCD_SetFont(&Font20);
  uint16_t textY = SIDE_BUTTON_Y + (SIDE_BUTTON_HEIGHT - Font20.Height) / 2;
  Touchscreen_DrawTextCentered(SIDE_BUTTON_X, textY, SIDE_BUTTON_WIDTH, sideButtonLabels[sideButtonIndex]);
}

static void Touchscreen_DrawInputField(void)
{
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_DrawRect(INPUT_RECT_X, INPUT_RECT_Y, INPUT_RECT_W, INPUT_RECT_H, UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_FillRect(INPUT_RECT_X + 1, INPUT_RECT_Y + 1, INPUT_RECT_W - 2, INPUT_RECT_H - 2, UTIL_LCD_COLOR_LIGHTGRAY);
  // status text removed; keep input indicators only
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
    }
    else
    {
      /* status text removed */
    }
  }
  else if (key == TS_KEY_SIDE_CONTROL)
  {
    /* Toggle side button state and notify RFID task when turning ON */
    if (sideButtonIndex == 0)
    {
      /* Turn On pressed: signal RFID task to start listening */
      if (taskRFIDHandle != NULL)
      {
        osThreadFlagsSet(taskRFIDHandle, RFID_LISTEN_FLAG);
      }
      Touchscreen_SetAlarmStatus("Enter PIN");
      sideButtonIndex = 1;
    }
    else
    {
      /* Turn Off pressed: set alarm state off */
      /* Notify RFID task to stop listening if it's active */
      if (taskRFIDHandle != NULL)
      {
        osThreadFlagsSet(taskRFIDHandle, RFID_LISTEN_FLAG);
      }
      Touchscreen_SetAlarmStatus("Enter PIN");
      sideButtonIndex = 0;
    }
  }
  else if ((key <= 9) && (inputLength < INPUT_DOT_COUNT))
  {
    inputBuffer[inputLength++] = '0' + key;
    inputBuffer[inputLength] = '\0';
  }

  redrawPending = true;
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
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetFont(&Font24);
  UTIL_LCD_DisplayStringAt(0, 14, (uint8_t *)"Numeric keypad", CENTER_MODE);

  inputLength = 0;
  memset(inputBuffer, 0, sizeof(inputBuffer));
  strncpy(alarmStatus, "Alarm off", sizeof(alarmStatus));
  touchActive = false;

  Touchscreen_DrawInputField();
  Touchscreen_DrawAlarmStatus();
  Touchscreen_DrawKeypad();
  Touchscreen_DrawSideButton();
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

void Touchscreen_Render(void)
{
  if (!redrawPending)
  {
    return;
  }

  redrawPending = false;
  Touchscreen_DrawInputField();
  Touchscreen_DrawAlarmStatus();
  Touchscreen_DrawKeypad();
  Touchscreen_DrawSideButton();
}

void Touchscreen_SetAlarmStatus(const char *status)
{
  if (status != NULL)
  {
    strncpy(alarmStatus, status, sizeof(alarmStatus) - 1);
    alarmStatus[sizeof(alarmStatus) - 1] = '\0';
    Touchscreen_DrawAlarmStatus();
  }
}

const char* Touchscreen_GetPINBuffer(void)
{
  return inputBuffer;
}

uint8_t Touchscreen_ValidatePIN(void)
{
  return (strcmp(inputBuffer, "1234") == 0) ? 1 : 0;
}

void Touchscreen_ResetPIN(void)
{
  inputLength = 0;
  memset(inputBuffer, 0, sizeof(inputBuffer));
  Touchscreen_DrawInputField();
}
