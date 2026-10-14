#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

TIM_HandleTypeDef htim2;

QueueHandle_t Queue_LED1Handle;
QueueHandle_t Queue_LED2Handle;
QueueHandle_t Queue_LED3Handle;

uint32_t led_duty[3] = {50, 50, 50};
uint8_t selected_led = 0;

#define DUTY_STEP 10

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

void StartTaskButton(void const *argument);
void StartTaskLED1(void const *argument);
void StartTaskLED2(void const *argument);
void StartTaskLED3(void const *argument);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_TIM2_Init();

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, led_duty[0]);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, led_duty[1]);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, led_duty[2]);

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

    Queue_LED1Handle = xQueueCreate(5, sizeof(uint32_t));
    Queue_LED2Handle = xQueueCreate(5, sizeof(uint32_t));
    Queue_LED3Handle = xQueueCreate(5, sizeof(uint32_t));

    if (Queue_LED1Handle == NULL ||
        Queue_LED2Handle == NULL ||
        Queue_LED3Handle == NULL)
    {
        Error_Handler();
    }

    osThreadDef(Task_Button, StartTaskButton, osPriorityNormal, 0, 128);
    osThreadCreate(osThread(Task_Button), NULL);

    osThreadDef(Task_LED1, StartTaskLED1, osPriorityNormal, 0, 64);
    osThreadCreate(osThread(Task_LED1), NULL);

    osThreadDef(Task_LED2, StartTaskLED2, osPriorityNormal, 0, 64);
    osThreadCreate(osThread(Task_LED2), NULL);

    osThreadDef(Task_LED3, StartTaskLED3, osPriorityNormal, 0, 64);
    osThreadCreate(osThread(Task_LED3), NULL);

    osKernelStart();

    while (1)
    {
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

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_TIM2_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 719;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 99;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

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

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 50;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_TIM_MspPostInit(&htim2);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = BTN_SEL_Pin | BTN_UP_Pin | BTN_DOWN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void StartTaskButton(void const *argument)
{
    QueueHandle_t led_queues[3] = {
        Queue_LED1Handle,
        Queue_LED2Handle,
        Queue_LED3Handle
    };

    GPIO_PinState prev_sel = GPIO_PIN_SET;
    GPIO_PinState prev_up = GPIO_PIN_SET;
    GPIO_PinState prev_down = GPIO_PIN_SET;

    for (;;)
    {
        GPIO_PinState curr_sel = HAL_GPIO_ReadPin(GPIOB, BTN_SEL_Pin);
        GPIO_PinState curr_up = HAL_GPIO_ReadPin(GPIOB, BTN_UP_Pin);
        GPIO_PinState curr_down = HAL_GPIO_ReadPin(GPIOB, BTN_DOWN_Pin);

        if (curr_sel == GPIO_PIN_RESET && prev_sel == GPIO_PIN_SET)
        {
            selected_led = (selected_led + 1) % 3;
        }

        if (curr_up == GPIO_PIN_RESET && prev_up == GPIO_PIN_SET)
        {
            if (led_duty[selected_led] + DUTY_STEP <= 100)
            {
                led_duty[selected_led] += DUTY_STEP;
            }
            else
            {
                led_duty[selected_led] = 100;
            }

            xQueueSend(
                led_queues[selected_led],
                &led_duty[selected_led],
                0
            );
        }

        if (curr_down == GPIO_PIN_RESET && prev_down == GPIO_PIN_SET)
        {
            if (led_duty[selected_led] >= DUTY_STEP)
            {
                led_duty[selected_led] -= DUTY_STEP;
            }
            else
            {
                led_duty[selected_led] = 0;
            }

            xQueueSend(
                led_queues[selected_led],
                &led_duty[selected_led],
                0
            );
        }

        prev_sel = curr_sel;
        prev_up = curr_up;
        prev_down = curr_down;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void StartTaskLED1(void const *argument)
{
    uint32_t new_duty = 0;

    for (;;)
    {
        if (xQueueReceive(Queue_LED1Handle, &new_duty, portMAX_DELAY) == pdPASS)
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (uint16_t)new_duty);
        }
    }
}

void StartTaskLED2(void const *argument)
{
    uint32_t new_duty = 0;

    for (;;)
    {
        if (xQueueReceive(Queue_LED2Handle, &new_duty, portMAX_DELAY) == pdPASS)
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint16_t)new_duty);
        }
    }
}

void StartTaskLED3(void const *argument)
{
    uint32_t new_duty = 0;

    for (;;)
    {
        if (xQueueReceive(Queue_LED3Handle, &new_duty, portMAX_DELAY) == pdPASS)
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (uint16_t)new_duty);
        }
    }
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

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif