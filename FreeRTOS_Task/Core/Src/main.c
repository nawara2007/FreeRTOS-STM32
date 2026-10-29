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

// Standard includes.
#include <stdio.h>

// Scheduler includes.
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define switch_event_flag 0x01 // set the flag to 0x01 (arbitary)

static void prvCalcSwitch1(  void * pvParameters  );
static void prvIncreaseVar(  void * pvParameters  );
static void prvDecreaseVar(  void * pvParameters  );
static void prvUART(  void * pvParameters  );

uint32_t shared = 100; // protected by the mutex
volatile uint32_t counter = 0; // only accessed by calcSwitch1

// will be handled by a queue
typedef struct {
	uint8_t source; // 0 for SRC:counter, 1 for SRC:shared
	uint32_t value;
} UARTData_t;

QueueHandle_t UARTHandle;
SemaphoreHandle_t sharedMutex;
EventGroupHandle_t counterEventHandle;

// Event Counter

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

  // https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/01-xTaskCreate
  xTaskCreate( prvCalcSwitch1, "CalcSwitch1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
  xTaskCreate( prvIncreaseVar, "IncreaseVar", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
  xTaskCreate( prvDecreaseVar, "DecreaseVar", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
  xTaskCreate( prvUART, "UART", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL );


  // https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/01-xQueueCreate
  UARTHandle = xQueueCreate( 10, sizeof(UARTData_t) );

  // https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/06-xSemaphoreCreateMutex
  sharedMutex = xSemaphoreCreateMutex();

  // https://www.freertos.org/Documentation/02-Kernel/04-API-references/12-Event-groups-or-flags/01-xEventGroupCreate
  counterEventHandle = xEventGroupCreate();

  // https://www.freertos.org/Documentation/02-Kernel/04-API-references/04-RTOS-kernel-control/03-vTaskStartScheduler
  vTaskStartScheduler();

  /* USER CODE END 2 */

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
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) // #27
{
	static uint32_t lastTick = 0;
	//https://www.freertos.org/Documentation/02-Kernel/04-API-references/12-Event-groups-or-flags/06-xEventGroupSetBitsFromISR
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (GPIO_Pin != GPIO_PIN_15 || counterEventHandle == NULL)
		return;

	uint32_t now = xTaskGetTickCountFromISR();
	if ((now - lastTick) < 200) // for handling debounce
		return;
	lastTick = now;

	if (counter >= 9)
		counter = 0;
	else
		counter++;

	// https://www.freertos.org/Documentation/02-Kernel/04-API-references/12-Event-groups-or-flags/06-xEventGroupSetBitsFromISR
	xEventGroupSetBitsFromISR((EventGroupHandle_t)counterEventHandle, switch_event_flag, &xHigherPriorityTaskWoken); // to set a flag so the calcswitch know the button is pressed
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // this requests a context switch as soon as the ISR exits
}

static void prvCalcSwitch1(  void * pvParameters  )
{
	UARTData_t  msg;
	uint32_t    counterCopy;
	EventBits_t bits;

	for(;;)
	{
		// https://www.freertos.org/Documentation/02-Kernel/04-API-references/12-Event-groups-or-flags/04-xEventGroupWaitBits
		// blocked for 2 ticks (2ms) unless the event is set
		// xClearOnExit --> true , xWaitForAllBits --> false
		bits = xEventGroupWaitBits((EventGroupHandle_t)counterEventHandle, switch_event_flag, pdTRUE, pdFALSE, 2);

		if (bits & switch_event_flag)
		{
			//https://www.freertos.org/Documentation/02-Kernel/04-API-references/04-RTOS-kernel-control/01-taskENTER_CRITICAL_taskEXIT_CRITICAL
			// This reads the global counter safely by diabling all interrrupts
			taskENTER_CRITICAL();
			counterCopy = counter;
			taskEXIT_CRITICAL();

			msg.source = 0;
			msg.value  = counterCopy * 10U;

		    xQueueSend(UARTHandle , &msg, 0);
		}

	}
}


static void prvIncreaseVar(  void * pvParameters  )
{
	UARTData_t msg;
	//https://www.freertos.org/Documentation/02-Kernel/04-API-references/03-Task-utilities/00-Task-utilities#xtaskgettickcount
	TickType_t xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		// https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/12-xSemaphoreTake
		if (xSemaphoreTake( sharedMutex, ( TickType_t ) 10 ) == pdTRUE )
		{
			shared++;
			if (shared > 200)
			{
				shared = 100;
			}

			msg.source = 1;
			msg.value  = shared;

			// https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/15-xSemaphoreGive
			xSemaphoreGive( sharedMutex );

			//https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/03-xQueueSend
			xQueueSend(UARTHandle , &msg, 0);
		}

		// https://www.freertos.org/Documentation/02-Kernel/04-API-references/02-Task-control/03-xTaskDelayUntil
		xTaskDelayUntil(&xLastWakeTime, 10);

	}
}

static void prvDecreaseVar(  void * pvParameters  )
{
	UARTData_t msg;
	TickType_t xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		if (xSemaphoreTake( sharedMutex, ( TickType_t ) 10 ) == pdTRUE )
		{
			shared--;
			if (shared < 100)
			{
				shared = 200;
			}

			msg.source = 1;
			msg.value  = shared;

			xSemaphoreGive( sharedMutex );

			xQueueSend(UARTHandle , &msg, 0);
		}

		xTaskDelayUntil(&xLastWakeTime, 12);

	}
}

static void prvUART(  void * pvParameters  )
{
	UARTData_t rx;
	char txBuf[48];
	int len;
	TickType_t xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		//https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/09-xQueueReceive
	  while (xQueueReceive(UARTHandle, &rx, 0))
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
			  // #556
			  HAL_UART_Transmit(&huart1, (uint8_t *)txBuf, (uint16_t)len, 100);
		  }
	  }

	  xTaskDelayUntil(&xLastWakeTime, 5);
	}
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2)
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
