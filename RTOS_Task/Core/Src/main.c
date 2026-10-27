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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "event_groups.h"
//#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define EVT_SWITCH_PRESSED (0x01U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* Definitions for CalcSwitch1 */
osThreadId_t CalcSwitch1Handle;
const osThreadAttr_t CalcSwitch1_attributes = {
  .name = "CalcSwitch1",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for IncreaseVar */
osThreadId_t IncreaseVarHandle;
const osThreadAttr_t IncreaseVar_attributes = {
  .name = "IncreaseVar",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DecreaseVar */
osThreadId_t DecreaseVarHandle;
const osThreadAttr_t DecreaseVar_attributes = {
  .name = "DecreaseVar",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UART */
osThreadId_t UARTHandle;
const osThreadAttr_t UART_attributes = {
  .name = "UART",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UARTData */
osMessageQueueId_t UARTDataHandle;
const osMessageQueueAttr_t UARTData_attributes = {
  .name = "UARTData"
};
/* Definitions for sharedMutex */
osMutexId_t sharedMutexHandle;
const osMutexAttr_t sharedMutex_attributes = {
  .name = "sharedMutex"
};
/* Definitions for counterEvent */
osEventFlagsId_t counterEventHandle;
const osEventFlagsAttr_t counterEvent_attributes = {
  .name = "counterEvent"
};
/* USER CODE BEGIN PV */

typedef struct {
	uint8_t source; // 0 for SRC:counter, 1 for SRC:shared
	uint32_t value;
} UARTData_t;

volatile uint32_t counter;
uint32_t shared = 100; // protected by the mutex
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
void StartCalcSwitch1(void *argument);
void StartIncreaseVar(void *argument);
void StartDecreaseVar(void *argument);
void StartUART(void *argument);

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of sharedMutex */
  sharedMutexHandle = osMutexNew(&sharedMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of UARTData */
  UARTDataHandle = osMessageQueueNew (16, sizeof(UARTData_t), &UARTData_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of CalcSwitch1 */
  CalcSwitch1Handle = osThreadNew(StartCalcSwitch1, NULL, &CalcSwitch1_attributes);

  /* creation of IncreaseVar */
  IncreaseVarHandle = osThreadNew(StartIncreaseVar, NULL, &IncreaseVar_attributes);

  /* creation of DecreaseVar */
  DecreaseVarHandle = osThreadNew(StartDecreaseVar, NULL, &DecreaseVar_attributes);

  /* creation of UART */
  UARTHandle = osThreadNew(StartUART, NULL, &UART_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* creation of counterEvent */
  counterEventHandle = osEventFlagsNew(&counterEvent_attributes);

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) // #27
{
	static uint32_t lastTick = 0;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (GPIO_Pin != GPIO_PIN_15 || counterEventHandle == NULL)
		return;

	uint32_t now = HAL_GetTick();
	if ((now - lastTick) < 200) // for handling debounce
		return;
	lastTick = now;

	if (counter >= 9)
		counter = 0;
	else
		counter++;

	xEventGroupSetBitsFromISR((EventGroupHandle_t)counterEventHandle, EVT_SWITCH_PRESSED, &xHigherPriorityTaskWoken); // to set a flag so the calcswitch know the button is pressed
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // this requests a context switch as soon as the ISR exits

}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartCalcSwitch1 */
/**
  * @brief  Function implementing the CalcSwitch1 thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartCalcSwitch1 */
void StartCalcSwitch1(void *argument)
{
  /* USER CODE BEGIN 5 */
	UARTData_t  msg;
	uint32_t    counterCopy;
	EventBits_t bits;
  /* Infinite loop */
  for(;;)
  {
	  bits = xEventGroupWaitBits((EventGroupHandle_t)counterEventHandle, EVT_SWITCH_PRESSED, pdTRUE, pdFALSE, 2);
	  // The task blocks here, using no CPU, until the bit is set or 2 ticks (2 ms) pass

	  // checks whether the switch bit was in the returned value.
	  if (bits & EVT_SWITCH_PRESSED)
	  {
		  // This reads the global counter safely
		  taskENTER_CRITICAL();
		  counterCopy = counter;
		  taskEXIT_CRITICAL();

		  msg.source = 0;
		  msg.value  = counterCopy * 10U;

		  osMessageQueuePut(UARTDataHandle, &msg, 0, 0);
	  }
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartIncreaseVar */
/**
* @brief Function implementing the IncreaseVar thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartIncreaseVar */
void StartIncreaseVar(void *argument)
{
  /* USER CODE BEGIN StartIncreaseVar */
	UARTData_t msg;
	uint32_t nextWake = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {

	if (osMutexAcquire(sharedMutexHandle, osWaitForever) == osOK)
	{
		shared++;
		if (shared > 200)
		{
			shared = 100;
		}

		msg.source = 1;
		msg.value  = shared;

		osMutexRelease(sharedMutexHandle);

		osMessageQueuePut(UARTDataHandle, &msg, 0, 0);
	}

	nextWake += 10;
	osDelayUntil(nextWake);
  }
  /* USER CODE END StartIncreaseVar */
}

/* USER CODE BEGIN Header_StartDecreaseVar */
/**
* @brief Function implementing the DecreaseVar thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDecreaseVar */
void StartDecreaseVar(void *argument)
{
  /* USER CODE BEGIN StartDecreaseVar */
  UARTData_t msg;
  uint32_t nextWake = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
	  if (osMutexAcquire(sharedMutexHandle, osWaitForever) == osOK)
	  {
		  shared--;
		  if (shared < 100)
		  {
			  shared = 200;
		  }

		  msg.source = 1;
		  msg.value  = shared;

		  osMutexRelease(sharedMutexHandle);

		  osMessageQueuePut(UARTDataHandle, &msg, 0, 0);
	  }

	  nextWake += 12;
	  osDelayUntil(nextWake);

  }
  /* USER CODE END StartDecreaseVar */
}

/* USER CODE BEGIN Header_StartUART */
/**
* @brief Function implementing the UART thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUART */
void StartUART(void *argument)
{
  /* USER CODE BEGIN StartUART */
	UARTData_t rx;
	char txBuf[48];
	int len;
	uint32_t nextWake = osKernelGetTickCount();

  /* Infinite loop */
  for(;;)
  {
	  while (osMessageQueueGet(UARTDataHandle, &rx, NULL, 0) == osOK)
	  {
		  if (rx.source == 0)
		  {
			  len = snprintf(txBuf, sizeof(txBuf),
							 "Counter: %lu\r\n", (unsigned long)rx.value);
		  }
		  else
		  {
			  len = snprintf(txBuf, sizeof(txBuf),
							 "Shared: %lu\r\n", (unsigned long)rx.value);
		  }

		  if (len > 0)
		  {
			  HAL_UART_Transmit(&huart1, (uint8_t *)txBuf, (uint16_t)len, 100);
		  }
	  }


	  nextWake += 5;
	  osDelayUntil(nextWake);
  }
  /* USER CODE END StartUART */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
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
