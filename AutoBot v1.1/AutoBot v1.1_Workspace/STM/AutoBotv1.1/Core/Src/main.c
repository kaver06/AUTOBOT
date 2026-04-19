/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include<stdio.h>
#include<string.h>
#include "ir_array.h"
#include "motor_control.h"
#include "ultrasonic.h"
#include "imu.h"
#include "encoder.h"
#include "battery_status.h"
#include "uart.h"
#include "system_flags.h"
#include "lcd.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define Buzzer_Port	GPIOC
#define BUZZER_PIN	GPIO_PIN_11
#define ENABLE_PRINT     0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart1_rx;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for IMU */
osThreadId_t IMUHandle;
const osThreadAttr_t IMU_attributes = {
  .name = "IMU",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for IMU_Print */
osThreadId_t IMU_PrintHandle;
const osThreadAttr_t IMU_Print_attributes = {
  .name = "IMU_Print",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for IR */
osThreadId_t IRHandle;
const osThreadAttr_t IR_attributes = {
  .name = "IR",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Motor */
osThreadId_t MotorHandle;
const osThreadAttr_t Motor_attributes = {
  .name = "Motor",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for ESP_Data */
osThreadId_t ESP_DataHandle;
const osThreadAttr_t ESP_Data_attributes = {
  .name = "ESP_Data",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ESP_RX */
osThreadId_t ESP_RXHandle;
const osThreadAttr_t ESP_RX_attributes = {
  .name = "ESP_RX",
  .stack_size = 384 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Telemetry */
osThreadId_t TelemetryHandle;
const osThreadAttr_t Telemetry_attributes = {
  .name = "Telemetry",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for imuQueue */
osMessageQueueId_t imuQueueHandle;
const osMessageQueueAttr_t imuQueue_attributes = {
  .name = "imuQueue"
};
/* Definitions for irQueue */
osMessageQueueId_t irQueueHandle;
const osMessageQueueAttr_t irQueue_attributes = {
  .name = "irQueue"
};
/* USER CODE BEGIN PV */
char tx_buffer[1024];
volatile uint8_t send_flag = 0;
volatile uint8_t tx_busy = 0;
osEventFlagsId_t obstacleFlags;
uint8_t usart2_rx_byte;
char usart2_cmd_buffer[10];
uint8_t usart2_cmd_index = 0;
char pickup_block[2] = "A";
char drop_block[2]   = "B";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM7_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
void StartDefaultTask(void *argument);
void IMU_Task(void *argument);
void IMU_Print_Task(void *argument);
void IR_Task(void *argument);
void Motor_Task(void *argument);
void ESP_Data_Task(void *argument);
void ESP_RX_Task(void *argument);
void Telemetry_Task(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_ADC1_Init();
  MX_TIM7_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_UART_Receive_IT(&huart2, &usart2_rx_byte, 1);
  Motor_Init();
  encoder_init();
  BatteryMonitor_Init();
  UART1_Init();
  lcd_init(&hi2c1);
  lcd_clear();
  lcd_set_cursor(0,0);
  lcd_print("Initializing ...");
  HAL_Delay(4000);
  lcd_clear();
  lcd_set_cursor(0,4);
  lcd_print("AUTOBOT");
  lcd_set_cursor(1,0);
  lcd_print("Operational");
  char buf[8];
  uint8_t percent;
  float volt;

  BatteryMonitor_Get(&volt, &percent);

  snprintf(buf, sizeof(buf), "%2d%%", percent);

  lcd_set_cursor(0, 13);   // top-right
  lcd_print(buf);
  HAL_Delay(2000);




  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  obstacleFlags = osEventFlagsNew(NULL);
  if (obstacleFlags == NULL)
  {
      Error_Handler();  // or while(1)
  }

  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of imuQueue */
  imuQueueHandle = osMessageQueueNew (3, sizeof(IMU_Packet_t), &imuQueue_attributes);

  /* creation of irQueue */
  irQueueHandle = osMessageQueueNew (1, sizeof(uint8_t), &irQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of IMU */
  IMUHandle = osThreadNew(IMU_Task, NULL, &IMU_attributes);

  /* creation of IMU_Print */
  IMU_PrintHandle = osThreadNew(IMU_Print_Task, NULL, &IMU_Print_attributes);

  /* creation of IR */
  IRHandle = osThreadNew(IR_Task, NULL, &IR_attributes);

  /* creation of Motor */
  MotorHandle = osThreadNew(Motor_Task, NULL, &Motor_attributes);

  /* creation of ESP_Data */
  ESP_DataHandle = osThreadNew(ESP_Data_Task, NULL, &ESP_Data_attributes);

  /* creation of ESP_RX */
  ESP_RXHandle = osThreadNew(ESP_RX_Task, NULL, &ESP_RX_attributes);

  /* creation of Telemetry */
  TelemetryHandle = osThreadNew(Telemetry_Task, NULL, &Telemetry_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 8399;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 59999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 10;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

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

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 83;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 59999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
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
  sConfigOC.Pulse = 10;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 65535;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim5, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

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
  htim6.Init.Prescaler = 8399;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 99;
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
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 8999;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 9999;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Buzzer_Pin_GPIO_Port, Buzzer_Pin_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_1_Pin */
  GPIO_InitStruct.Pin = IR_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_3_Pin */
  GPIO_InitStruct.Pin = IR_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_4_Pin */
  GPIO_InitStruct.Pin = IR_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : IR_2_Pin USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = IR_2_Pin|USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Buzzer_Pin_Pin */
  GPIO_InitStruct.Pin = Buzzer_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Buzzer_Pin_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	 if (huart->Instance == USART1)
	    {
	        uart1_tx_busy = 0;
	    }
	    else if (huart->Instance == USART2)
	    {
	    	tx_busy = 0;
	    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	    {
	    	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
	        char ch = usart2_rx_byte;

	        if (ch == '\n' || ch == '\r')
	        {
	            usart2_cmd_buffer[usart2_cmd_index] = '\0';

	            if (strcmp(usart2_cmd_buffer, "A,B") == 0)
	            {
	                osEventFlagsSet(obstacleFlags, SYSTEM_START);
	            }

	            usart2_cmd_index = 0;
	        }
	        else
	        {
	            if (usart2_cmd_index < sizeof(usart2_cmd_buffer) - 1)
	            {
	                usart2_cmd_buffer[usart2_cmd_index++] = ch;
	            }
	        }

	        // restart interrupt
	        HAL_UART_Receive_IT(&huart2, &usart2_rx_byte, 1);
	    }
}
static void LCD_UpdateBattery(uint8_t percent)
{
    char buf[8];

    snprintf(buf, sizeof(buf), "%2d%%", percent);

    // Top row, right aligned (16x2 LCD → last 6 chars)
    lcd_set_cursor(0, 13);   // column 10 to 15
    lcd_print("      ");     // clear old
    lcd_set_cursor(0, 13);
    lcd_print(buf);
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
	  Ultrasonic_Init();
  /* Infinite loop */
  for(;;)
  {
	  //printf("%d\n\r",SYSTEM_START);
	  if (!(osEventFlagsGet(obstacleFlags) & SYSTEM_START))
	     {
	        // printf("System is ready, waiting for pickup and destination location...\r\n");
	         osDelay(2000);
	         continue;
	     }
	  Ultrasonic_Process();

	  if (object1_near_flag || object2_near_flag || object3_near_flag)
	  {
	      osEventFlagsSet(obstacleFlags, OBSTACLE_PRESENT);
	      HAL_GPIO_WritePin(Buzzer_Port, BUZZER_PIN,GPIO_PIN_SET);
	  }
	  else
	  {
	      osEventFlagsClear(obstacleFlags, OBSTACLE_PRESENT);
	      HAL_GPIO_WritePin(Buzzer_Port, BUZZER_PIN,GPIO_PIN_RESET);
	  }
	          osDelay(50);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_IMU_Task */
/**
* @brief Function implementing the IMU thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_IMU_Task */
void IMU_Task(void *argument)
{
  /* USER CODE BEGIN IMU_Task */
	IMU_Packet_t pkt = {0};
	static uint32_t imu_i2c_error_cnt = 0;
	mpu9250_init();
  /* Infinite loop */
  for(;;)
  {
	  // 1. Wait for TIM6 trigger
	  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
	  // 2. Read IMU
	  HAL_StatusTypeDef ret=HAL_I2C_Mem_Read( &hi2c1, MPU9250_ADDR, ACCEL_XOUT_H_REG, I2C_MEMADD_SIZE_8BIT, mpu_raw, 14, 20 );
	  if (ret != HAL_OK)
	  {
		  imu_i2c_error_cnt++;
		  continue;
	  }
	  // 3. Process IMU data
	  mpu9250_parse_accel_gyro(mpu_raw);
	  mpu9250_read_mag();

	  int32_t l = encoder_get_left_position();
	  int32_t r = encoder_get_right_position();

	  // 4. Fill packet
	  pkt.Left_Cnt  = l;
	  pkt.Right_Cnt = r;
	  pkt.Left_Deg  = encoder_get_left_degree(l);
	  pkt.Right_Deg = encoder_get_right_degree(r);

	  Battery_Process();
	  BatteryMonitor_Get(&pkt.battery_voltage, &pkt.battery_percentage);


	  pkt.ax = imu.ax;
	  pkt.ay = imu.ay;
	  pkt.az = imu.az;
	  pkt.gx = imu.gx;
	  pkt.gy = imu.gy;
	  pkt.gz = imu.gz;
	  pkt.pitch = imu.pitch;
	  pkt.roll = imu.roll;
	  pkt.yaw = imu.yaw;

	  pkt.timestamp = osKernelGetTickCount();
	  pkt.obstacle_s = osEventFlagsGet(obstacleFlags) & OBSTACLE_PRESENT;

	  pkt.tag_id    = esp32_tag_id;
	  pkt.tag_yaw   = esp32_yaw;
	  pkt.tag_pitch = esp32_pitch;
	  pkt.tag_roll  = esp32_roll;
	  pkt.tag_x     = esp32_x;
	  pkt.tag_y     = esp32_y;
	  pkt.tag_z     = esp32_z;


	  osStatus_t status = osMessageQueuePut(imuQueueHandle, &pkt, 0, 0);
	  if (status != osOK)
	  {
		  // Queue full → overwrite oldest
		  IMU_Packet_t dummy;
		  osMessageQueueGet(imuQueueHandle, &dummy, NULL, 0);
		  osMessageQueuePut(imuQueueHandle, &pkt, 0, 0);
	  }
   }

  /* USER CODE END IMU_Task */
}

/* USER CODE BEGIN Header_IMU_Print_Task */
/**
* @brief Function implementing the IMU_Print thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_IMU_Print_Task */
void IMU_Print_Task(void *argument)
{
  /* USER CODE BEGIN IMU_Print_Task */
	 IMU_Packet_t pkt;

	    uint8_t last_system_state   = 255;
	    uint8_t last_obstacle_state = 255;
	    uint8_t last_reached_state  = 255;

	    static uint8_t arrow_pos = 0;
	    static uint32_t last_anim_tick = 0;
  /* Infinite loop */
	    for (;;)
	    {
	    	if (osMessageQueueGet(imuQueueHandle, &pkt, NULL, osWaitForever) == osOK)
	    	    {
	    	            /* --------- READ FLAGS ONCE --------- */
	    	            uint32_t flags = osEventFlagsGet(obstacleFlags);

	    	            uint8_t system_started   = (flags & SYSTEM_START) ? 1 : 0;
	    	            uint8_t current_obstacle = (flags & OBSTACLE_PRESENT) ? 1 : 0;
	    	            uint8_t reached_dest     = (flags & ESP_APRILTAG_ENABLE) ? 1 : 0;

	    	            /* --------- STATE CHANGE HANDLING --------- */
	    	            if (system_started   != last_system_state   ||
	    	                current_obstacle != last_obstacle_state ||
	    	                reached_dest     != last_reached_state)
	    	            {
	    	                lcd_clear();

	    	                /* Top row title */
	    	                lcd_set_cursor(0, 4);
	    	                lcd_print("AUTOBOT");

	    	                /* Bottom row logic */
	    	                if (!system_started)
	    	                {
	    	                    lcd_set_cursor(1, 0);
	    	                    lcd_print("Waiting...");
	    	                }
	    	                else if (current_obstacle)
	    	                {
	    	                    lcd_set_cursor(1, 0);
	    	                    lcd_print("OBSTACLE !");
	    	                }
	    	                else if (reached_dest)
	    	                {
	    	                    lcd_set_cursor(1, 3);
	    	                    lcd_print("Reached B");
	    	                }
	    	                else
	    	                {
	    	                    lcd_set_cursor(1, 1);
	    	                    lcd_print(pickup_block);

	    	                    lcd_set_cursor(1, 14);
	    	                    lcd_print(drop_block);

	    	                    arrow_pos = 0;
	    	                    last_anim_tick = osKernelGetTickCount();
	    	                }

	    	                last_system_state   = system_started;
	    	                last_obstacle_state = current_obstacle;
	    	                last_reached_state  = reached_dest;
	    	            }

	    	            /* --------- ANIMATION SECTION --------- */
	    	            if (system_started && !current_obstacle && !reached_dest)
	    	            {
	    	                uint32_t now = osKernelGetTickCount();

	    	                if ((now - last_anim_tick) >= 200)
	    	                {
	    	                    last_anim_tick = now;

	    	                    lcd_set_cursor(1, 3);
	    	                    lcd_print("          ");   // clear 10 chars

	    	                    lcd_set_cursor(1, 3);

	    	                    for (uint8_t i = 0; i < arrow_pos; i++)
	    	                    {
	    	                        lcd_print("-");
	    	                    }

	    	                    lcd_print(">");

	    	                    arrow_pos++;

	    	                    if (arrow_pos > 9)
	    	                        arrow_pos = 0;
	    	                }
	    	            }

	    	            /* --------- ALWAYS UPDATE BATTERY LAST --------- */
	    	            LCD_UpdateBattery(pkt.battery_percentage);
		  #if ENABLE_PRINT
		  printf("Accel: %.2f %.2f %.2f\r\n", pkt.ax, pkt.ay, pkt.az);
		  printf("Gyro:  %.2f %.2f %.2f\r\n", pkt.gx, pkt.gy, pkt.gz);
		  printf("Euler: Y %.2f P %.2f R %.2f\r\n",
				 pkt.yaw, pkt.pitch, pkt.roll);

		  printf("\nR_Cnt: %ld L_Cnt: %ld\r\n",pkt.Right_Cnt,pkt.Left_Cnt);
		  printf("R_Deg: %.2f L_Deg: %.2f\r\n",pkt.Right_Deg,pkt.Left_Deg);

		  printf("\nBat_V: %.2f Bat_P: %u%%\r\n",pkt.battery_voltage,pkt.battery_percentage);
		  printf("\nTAG_ID:%u\r\n",pkt.tag_id);
		  printf("\nESP_X:%.2f Y:%.2f Z:%.2f\r\n", pkt.tag_x,pkt.tag_y,pkt.tag_z);
		  printf("\nPitch:%.2f Yaw:%.2f Roll:%.2f\r\n",pkt.tag_pitch,pkt.tag_yaw,pkt.tag_roll);
		  printf(pkt.obstacle_s ? "\nObstacle Detected\r\n" : "\nPath Clear\r\n");
		  printf("Time:  %lu\r\n", pkt.timestamp);
		  osDelay(100);
		#endif
	            }
	        }

  /* USER CODE END IMU_Print_Task */
}

/* USER CODE BEGIN Header_IR_Task */
/**
* @brief Function implementing the IR thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_IR_Task */
void IR_Task(void *argument)
{
  /* USER CODE BEGIN IR_Task */
	IR_Array_T irdata;
	uint8_t pattern;

  /* Infinite loop */
  for(;;)
  {
	 IR_Array_Read(&irdata);
	 pattern = IR_Array_GetPattern(&irdata);
	 IR_PatternMonitor(pattern);
	 // overwrite-oldest behavior
	 if (osMessageQueuePut(irQueueHandle, &pattern, 0, 0) != osOK)
	 {
	     uint8_t dummy;
	     osMessageQueueGet(irQueueHandle, &dummy, NULL, 0); // drop old
	     osMessageQueuePut(irQueueHandle, &pattern, 0, 0);
	 }

	 osDelay(10);
  }
  /* USER CODE END IR_Task */
}

/* USER CODE BEGIN Header_Motor_Task */
/**
* @brief Function implementing the Motor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Motor_Task */
void Motor_Task(void *argument)
{
  /* USER CODE BEGIN Motor_Task */
    uint8_t pattern = 0xFF;
    uint8_t new_pattern = 0xFF;
	Motor_Init();
  /* Infinite loop */
  for(;;)
  {
	      if (!(osEventFlagsGet(obstacleFlags) & SYSTEM_START))
	      {
	          Motor_Stop();
	          osDelay(100);
	          continue;
	      }

	  if (osMessageQueueGet(irQueueHandle, &new_pattern, NULL, 10) == osOK)
	  {
		pattern = new_pattern;   // latch latest pattern
	  }
	  if (osEventFlagsGet(obstacleFlags) & OBSTACLE_PRESENT)
	  {
	      Motor_Stop();
	  }
	  else
	  {
		  switch (pattern)
		  {
			  case 0b0000: Motor_Forward(); break;
			  case 0b0100: Motor_Left(); break;
			  case 0b0010: Motor_Right(); break;
			  case 0b0001: Motor_SharpRight(); break;
			  case 0b1000: Motor_SharpLeft(); break;
			  case 0b1100: Motor_SharpLeft(); break;
			  case 0b0011: Motor_SharpRight(); break;
			  case 0b1111: Motor_Stop(); break;
			  default:     Motor_Stop(); break;
		  }
	  }
	  osDelay(10);
  }
  /* USER CODE END Motor_Task */
}

/* USER CODE BEGIN Header_ESP_Data_Task */
/**
* @brief Function implementing the ESP_Data thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ESP_Data_Task */
void ESP_Data_Task(void *argument)
{
  /* USER CODE BEGIN ESP_Data_Task */
	uint32_t last_state = 0;
  /* Infinite loop */
  for(;;)
  {
	  uint32_t flags = osEventFlagsGet(obstacleFlags);

	  if ((flags & ESP_APRILTAG_ENABLE) && !(last_state & ESP_APRILTAG_ENABLE))
	  {
		  UART1_SendString("YES\n");
	  }
	  else if (!(flags & ESP_APRILTAG_ENABLE) && (last_state & ESP_APRILTAG_ENABLE))
	  {
		  UART1_SendString("STOP\n");
	  }

	  last_state = flags;
	  osDelay(20);
  }
  /* USER CODE END ESP_Data_Task */
}

/* USER CODE BEGIN Header_ESP_RX_Task */
/**
* @brief Function implementing the ESP_RX thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ESP_RX_Task */
void ESP_RX_Task(void *argument)
{
  /* USER CODE BEGIN ESP_RX_Task */
  /* Infinite loop */
  for(;;)
  {
	  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
	  //printf("ESP_RX_Task woke up\r\n");

	  __HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
	  UART1_ProcessReceivedData();
	  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

	  if (new_esp32_data_flag)
	  {
		  new_esp32_data_flag = 0;


	  }
  }
  /* USER CODE END ESP_RX_Task */
}

/* USER CODE BEGIN Header_Telemetry_Task */
/**
* @brief Function implementing the Telemetry thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Telemetry_Task */
void Telemetry_Task(void *argument)
{
  /* USER CODE BEGIN Telemetry_Task */
	IMU_Packet_t pkt;
	static char tx_buf[512];
	static uint8_t send_div = 0;
  /* Infinite loop */
  for(;;)
  {
	  if (osMessageQueueGet(imuQueueHandle, &pkt, NULL, osWaitForever) == osOK)
	  {
		  if (++send_div < 2)
			  continue;

		  send_div = 0;

		  if (tx_busy)
			  continue;

		  int len = snprintf(tx_buf, sizeof(tx_buf),
			  "{"
				  "\"enc\":{"
					  "\"L\":%ld,"
					  "\"R\":%ld,"
					  "\"left_deg\":%.2f,"
					  "\"right_deg\":%.2f"
				  "},"
				  "\"imu\":{"
					  "\"acc\":[%.1f,%.1f,%.1f],"
					  "\"gyro\":[%.1f,%.1f,%.1f],"
					  "\"euler\":[%.1f,%.1f,%.1f]"
				  "},"
				  "\"battery\":{"
					  "\"voltage\":%.2f,"
					  "\"percent\":%u"
				  "},"
				  "\"esp\":{"
					  "\"tag\":%d,"
					  "\"yaw\":%.1f,"
					  "\"pitch\":%.1f,"
					  "\"roll\":%.1f,"
					  "\"pos\":[%.1f,%.1f,%.1f]"
				  "}"
			  "}\n",
			  (long)pkt.Left_Cnt,
			  (long)pkt.Right_Cnt,
			  pkt.Left_Deg,
			  pkt.Right_Deg,

			  pkt.ax, pkt.ay, pkt.az,
			  pkt.gx, pkt.gy, pkt.gz,
			  pkt.pitch, pkt.roll, pkt.yaw,

			  pkt.battery_voltage,
			  pkt.battery_percentage,

			  pkt.tag_id,
			  pkt.tag_yaw,
			  pkt.tag_pitch,
			  pkt.tag_roll,
			  pkt.tag_x,
			  pkt.tag_y,
			  pkt.tag_z
		  );

		  if (len > 0 && len < sizeof(tx_buf))
		  {
			  tx_busy = 1;

			  if (HAL_UART_Transmit_IT(&huart2, (uint8_t*)tx_buf, len) != HAL_OK)
			  {
				  tx_busy = 0;
			  }
		  }
	  }
  }
  /* USER CODE END Telemetry_Task */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM9 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	if(htim->Instance == TIM6)
	{
        osThreadFlagsSet(IMUHandle, 0x01);
	}
	    else if(htim->Instance == TIM4 || htim->Instance == TIM5)
	    {
	    	 encoder_tim_update_callback(htim);
	    }
	    else if (htim->Instance == TIM7)
		{
			battery_tick_counter++;
			if (battery_tick_counter >= 5)  // 5 sec interval
			{
				battery_tick_counter = 0;
				Start_Battery_ADC_Read();
			}
		}
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM9)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
