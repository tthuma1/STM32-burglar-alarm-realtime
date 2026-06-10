/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "touchscreen.h"
#include "mfrc522.h"
#include "lwip.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "lwip/tcpip.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PWM_STEPS   100
#define STEP_DELAY  10   // ms per step → 1s fade in/out
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
osThreadId_t taskLEDHandle;
const osThreadAttr_t taskLED_attributes = {
  .name = "taskLED",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t taskRFIDHandle;
const osThreadAttr_t taskRFID_attributes = {
  .name = "taskRFID",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t taskUDPHandle;
const osThreadAttr_t taskUDP_attributes = {
  .name = "taskUDP",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

SPI_HandleTypeDef hspi2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim6;
UART_HandleTypeDef huart3;

uint8_t txFWversion[128];
uint8_t msg[256];

uint8_t tim6_running = 0;

AlarmState_t g_alarmState = ALARM_STATE_ALARM_OFF;
osMessageQueueId_t g_http_event_queue;
uint8_t g_validPINEntered = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM6_Init(void);
bool LED_Breathe(void);

void StartTaskLED(void *argument);
void StartTaskRFID(void *argument);
void StartTaskUDP(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t uid[4];

MFRC522_t rfID = {&hspi2, CS_GPIO_Port, CS_Pin, RESET_GPIO_Port, RESET_Pin};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  MPU_Config();
  SCB_EnableICache();
  SCB_EnableDCache();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_SPI2_Init();
  MX_TIM3_Init();
  MX_TIM6_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
  UTIL_LCD_SetFuncDriver(&LCD_Driver);
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  g_http_event_queue = osMessageQueueNew(4, sizeof(HttpEvent_t), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* creation of taskLED */
  taskLEDHandle = osThreadNew(StartTaskLED, NULL, &taskLED_attributes);
  taskRFIDHandle = osThreadNew(StartTaskRFID, NULL, &taskRFID_attributes);
  taskUDPHandle = osThreadNew(StartTaskUDP, NULL, &taskUDP_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
  * @brief Configure MPU non-cacheable regions for Ethernet DMA.
  *        Must be called before SCB_EnableDCache() to avoid cache coherency
  *        HardFaults when the Ethernet MAC-DMA writes to RAM_D2.
  *
  *        Layout at 0x30040000 (last 32KB of RAM_D2):
  *          Region 0 (32KB, Normal WT): covers descriptors + RX pool
  *          Region 1 (256B, Device):    covers descriptor tables only (wins on overlap)
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  HAL_MPU_Disable();

  /* Region 0 — 32KB at 0x30040000: Normal Write-Through, no write-allocate */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress      = 0x30040000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_32KB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 1 — 256B at 0x30040000: Device (non-cacheable).
   * Higher region number wins on overlap, so this overrides region 0
   * for the descriptor tables area. */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress      = 0x30040000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_256B;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 63;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);
}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 49999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 39999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* Configure NVIC for TIM6 so callbacks can safely use RTOS APIs */
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 0x0;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_8, GPIO_PIN_SET);

  /*Configure GPIO pin : PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PI8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PG3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

bool LED_Breathe(void)
{
  // Fade IN: 0% → 100%
  for (uint32_t i = 0; i <= PWM_STEPS; i++)
  {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, i);
      if (osThreadFlagsWait(LED_BREATHE_STOP_FLAG, osFlagsWaitAny, 0) == LED_BREATHE_STOP_FLAG)
      {
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
          return true;
      }
      osDelay(STEP_DELAY);
  }

  // Fade OUT: 100% → 0%
  for (int32_t i = PWM_STEPS; i >= 0; i--)
  {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, i);
      if (osThreadFlagsWait(LED_BREATHE_STOP_FLAG, osFlagsWaitAny, 0) == LED_BREATHE_STOP_FLAG)
      {
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
          return true;
      }
      osDelay(STEP_DELAY);
  }

  return false;
}

/**
* @brief Function implementing the taskLED thread.
* @param argument: Not used
* @retval None
*/
void StartTaskLED(void *argument)
{
  /* USER CODE BEGIN StartTaskLED */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

  for(;;)
  {
    uint32_t flags = osThreadFlagsWait(LED_BREATHE_START_FLAG | LED_BREATHE_STOP_FLAG,
                                      osFlagsWaitAny,
                                      osWaitForever);

    if ((flags & LED_BREATHE_STOP_FLAG) != 0)
    {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
      continue;
    }

    if ((flags & LED_BREATHE_START_FLAG) != 0)
    {
      while (1)
      {
        if (LED_Breathe())
        {
          break;
        }
      }
    }
  }
  /* USER CODE END StartTaskLED */
}

/**
* @brief Function implementing the taskRFID thread.
* @param argument: Not used
* @retval None
*/
void StartTaskRFID(void *argument)
{
  /* USER CODE BEGIN StartTaskLED */
  strcpy((char*)msg, "\r\nLooking for MFRC522... \r\n");
  HAL_UART_Transmit(&huart3, msg, strlen((char*)msg), HAL_MAX_DELAY);
  MFRC522_Init(&rfID);
  USER_LOG("Waiting for motion detection...");
  uint8_t is_card_present = 0;
  bool is_waiting_for_rfid = false;
  
  /* Infinite loop */
  for(;;)
  {
    /* wait for RFID listen flag from touchscreen or other tasks */
    uint32_t flags = osThreadFlagsWait(RFID_LISTEN_FLAG, osFlagsWaitAny, osWaitForever);

    if ((flags & RFID_LISTEN_FLAG) != 0)
    {
      AlarmState_t previousAlarmState = g_alarmState;
      is_waiting_for_rfid = 1;
      USER_LOG("RFID listening started");
      g_validPINEntered = 0;
      Touchscreen_ResetPIN();

      /* Listen for cards until touchscreen or another task signals stop */
      while (is_waiting_for_rfid)
      {
        if (!is_card_present)
        {
          if (checkCardDetect(&rfID) == STATUS_OK)
          {
            is_card_present = 1;
            USER_LOG("Waiting for card removal...");

            if (MFRC522_ReadUid(&rfID, uid) == STATUS_OK)
            {
              USER_LOG("CARD ID:%02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);
              if ((uid[0] == 0xBA) && (uid[1] == 0x37) && (uid[2] == 0xF0) && (uid[3] == 0x81))
              {
                if (g_validPINEntered)
                {
                  g_alarmState = (previousAlarmState == ALARM_STATE_ALARM_OFF ? ALARM_STATE_WAITING_FOR_MOTION : ALARM_STATE_ALARM_OFF);
                  is_waiting_for_rfid = false;

                  if (g_alarmState == ALARM_STATE_WAITING_FOR_MOTION) {
                    HttpEvent_t ev = HTTP_EVENT_ALARM_ON;
                    osMessageQueuePut(g_http_event_queue, &ev, 0, 0);
                    Touchscreen_SetAlarmStatus("Alarm on");
                    USER_LOG("Alarm armed! Waiting for motion...");
                  } else {
                    HttpEvent_t ev = HTTP_EVENT_ALARM_OFF;
                    osMessageQueuePut(g_http_event_queue, &ev, 0, 0);
                    Touchscreen_SetAlarmStatus("Alarm off");
                    USER_LOG("Alarm off");
                    HAL_TIM_Base_Stop_IT(&htim6);
                    __HAL_TIM_SET_COUNTER(&htim6, 0);

                    tim6_running = 0;
                    Touchscreen_StopCountdown();
                    osThreadFlagsSet(taskLEDHandle, LED_BREATHE_STOP_FLAG);
                  }
                  Touchscreen_ResetPIN();
                  /* stop listening once alarm is armed */
                  break;
                }
                else
                {
                  USER_LOG("Invalid PIN. Card rejected.");
                }
              }
            }
          }
        }
        else
        {
          if (checkCardRemoval(&rfID) == STATUS_OK)
          {
            is_card_present = 0;
          }
        }

        osDelay(100);
      }
    }
  }
  /* USER CODE END StartTaskLED */
}

#define TCP_HTTP_DONE_FLAG  (1U << 0)
#define TCP_HTTP_ERROR_FLAG (1U << 1)

static struct tcp_pcb *g_http_pcb = NULL;
static char g_http_request[256];

static void http_tcp_close(struct tcp_pcb *pcb)
{
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  tcp_err(pcb, NULL);
  tcp_close(pcb);
  g_http_pcb = NULL;
}

static err_t http_tcp_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  if (p != NULL) {
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
  }
  /* p == NULL: remote side closed the connection */
  http_tcp_close(tpcb);
  osThreadFlagsSet(taskUDPHandle, TCP_HTTP_DONE_FLAG);
  return ERR_OK;
}

static err_t http_tcp_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  if (err != ERR_OK) {
    g_http_pcb = NULL;
    osThreadFlagsSet(taskUDPHandle, TCP_HTTP_ERROR_FLAG);
    return err;
  }

  tcp_recv(tpcb, http_tcp_recv);

  u16_t len = (u16_t)strlen(g_http_request);
  if (tcp_write(tpcb, g_http_request, len, TCP_WRITE_FLAG_COPY) != ERR_OK) {
    http_tcp_close(tpcb);
    osThreadFlagsSet(taskUDPHandle, TCP_HTTP_ERROR_FLAG);
    return ERR_OK;
  }
  tcp_output(tpcb);
  return ERR_OK;
}

static void http_tcp_err(void *arg, err_t err)
{
  /* PCB is already freed by lwIP when this fires — do not call tcp_* on it */
  g_http_pcb = NULL;
  osThreadFlagsSet(taskUDPHandle, TCP_HTTP_ERROR_FLAG);
}

void StartTaskUDP(void *argument)
{
  static const char *event_names[] = {
    "alarm_on", "alarm_off", "motion_detected", "alarm_triggered"
  };

  osDelay(2000);  /* wait for LwIP to come up */

  ip_addr_t server_ip;
  IP_ADDR4(&server_ip, 192, 168, 1, 1);

  for (;;)
  {
    HttpEvent_t evt;
    osMessageQueueGet(g_http_event_queue, &evt, NULL, osWaitForever);

    char body[40];
    snprintf(body, sizeof(body), "{\"type\":\"%s\"}", event_names[evt]);

    snprintf(g_http_request, sizeof(g_http_request),
      "POST /event HTTP/1.1\r\n"
      "Host: 192.168.1.1\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: %u\r\n"
      "Connection: close\r\n"
      "\r\n%s",
      (unsigned)strlen(body), body);

    osThreadFlagsClear(TCP_HTTP_DONE_FLAG | TCP_HTTP_ERROR_FLAG);

    LOCK_TCPIP_CORE();
    g_http_pcb = tcp_new();
    if (g_http_pcb != NULL) {
      tcp_err(g_http_pcb, http_tcp_err);
      if (tcp_connect(g_http_pcb, &server_ip, 5000, http_tcp_connected) != ERR_OK) {
        tcp_abort(g_http_pcb);
        g_http_pcb = NULL;
      }
    }
    UNLOCK_TCPIP_CORE();

    if (g_http_pcb == NULL) {
      continue;
    }

    uint32_t flags = osThreadFlagsWait(TCP_HTTP_DONE_FLAG | TCP_HTTP_ERROR_FLAG,
                                       osFlagsWaitAny, 10000);
    if ((int32_t)flags < 0) {
      /* timeout — abort the connection */
      LOCK_TCPIP_CORE();
      if (g_http_pcb != NULL) {
        tcp_abort(g_http_pcb);
        g_http_pcb = NULL;
      }
      UNLOCK_TCPIP_CORE();
    }
  }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  MX_LWIP_Init();       // start Ethernet + LwIP (must run under RTOS scheduler)
  Touchscreen_Init();   // init once

  static uint8_t lastPINLength = 0;

  /* Infinite loop */
  for(;;)
  {
    Touchscreen_Poll();
    Touchscreen_Render();

    uint8_t currentPINLength = strlen(Touchscreen_GetPINBuffer());
    
    if (currentPINLength > lastPINLength && currentPINLength == 4)
    {
      if (Touchscreen_ValidatePIN())
      {
        g_validPINEntered = 1;
      }
      lastPINLength = currentPINLength;
    }
    else if (currentPINLength < lastPINLength)
    {
      lastPINLength = currentPINLength;
    }
    
    osDelay(20);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
