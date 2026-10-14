#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
UART_HandleTypeDef huart1;
osThreadId Task_UARTHandle;
osMessageQId Queue_ButtonHandle;

volatile uint32_t button_press_count = 0;
volatile uint32_t last_exti_time = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
void StartTaskUART(void const * argument);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  osMessageQDef(Queue_Button, 10, uint32_t);
  Queue_ButtonHandle = osMessageCreate(osMessageQ(Queue_Button), NULL);
  osThreadDef(Task_UART, StartTaskUART, osPriorityNormal, 0, 256);
  Task_UARTHandle = osThreadCreate(osThread(Task_UART), NULL);

  osKernelStart();

  while (1)
  {
  }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_0)
  {
    uint32_t current_time = HAL_GetTick();
    if ((current_time - last_exti_time) > 200)
    {
      last_exti_time = current_time;

      button_press_count++;
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      uint32_t count_to_send = button_press_count;
      xQueueSendFromISR(Queue_ButtonHandle, &count_to_send, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}

void StartTaskUART(void const * argument)
{
  uint32_t received_count = 0;
  char tx_buffer[64];

  char *init_msg = "He thong khoi dong! Nhan nut PB0 de dem...\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t *)init_msg, strlen(init_msg), HAL_MAX_DELAY);

  for(;;)
  {
    if (xQueueReceive(Queue_ButtonHandle, &received_count, portMAX_DELAY) == pdPASS)
    {
      int len = snprintf(tx_buffer, sizeof(tx_buffer), 
                         "Phat hien nhan nut! So lan nhan: %lu\r\n", 
                         (unsigned long)received_count);
      if (len > 0)
      {
        HAL_UART_Transmit(&huart1, (uint8_t *)tx_buffer, (uint16_t)len, 100);
      }
    }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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

static void MX_USART1_UART_Init(void)
{
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
}
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}