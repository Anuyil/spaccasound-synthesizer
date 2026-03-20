/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main con FreeRTOS Nativo - Versione Corazzata
 ******************************************************************************
 */

#include "main.h"
#include "stm32g4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "buttons.h"
#include "display_ssd1306.h"

/* Variabili Globali */
I2C_HandleTypeDef hi2c1;
TaskHandle_t ButtonTaskHandle = NULL;
TaskHandle_t DisplayTaskHandle = NULL;
TaskHandle_t LEDTaskHandle = NULL;
QueueHandle_t ButtonQueue = NULL;
SemaphoreHandle_t DisplayMutex = NULL;

/* Prototipi */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
void ButtonTask(void *argument);
void DisplayTask(void *argument);
void LEDTask(void *argument);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    
    /* Pausa vitale per far stabilizzare l'energia sul display I2C */
    HAL_Delay(200);
    
    Buttons_Init();
    
    /* Init display silenzioso (se fallisce andiamo avanti lo stesso) */
    if(SSD1306_Init(&hi2c1)) {
        SSD1306_Fill(SSD1306_COLOR_BLACK);
        SSD1306_DrawRectangle(0, 0, 127, 63, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreen();
    }
    
    /* Creazione Oggetti FreeRTOS */
    ButtonQueue = xQueueCreate(10, sizeof(Button_t));
    DisplayMutex = xSemaphoreCreateMutex();
    
    if(ButtonQueue == NULL || DisplayMutex == NULL) {
        Error_Handler(); // Errore: Manca Heap!
    }
    
    /* Creazione Task con controllo errori */
    BaseType_t res1 = xTaskCreate(ButtonTask,  "Btn",  256, NULL, 2, &ButtonTaskHandle);
    BaseType_t res2 = xTaskCreate(DisplayTask, "Disp", 512, NULL, 1, &DisplayTaskHandle);
    BaseType_t res3 = xTaskCreate(LEDTask,     "LED",  128, NULL, 1, &LEDTaskHandle);
    
    /* Se anche UNO SOLO fallisce per memoria insufficiente, andiamo in errore */
    if(res1 != pdPASS || res2 != pdPASS || res3 != pdPASS) {
        Error_Handler(); // Errore: configTOTAL_HEAP_SIZE troppo piccolo!
    }
    
    /* Partenza! */
    vTaskStartScheduler();
    
    /* Se arriva qui, lo scheduler non è riuscito ad allocare l'Idle Task */
    Error_Handler();
    while (1);
}

/* ============================================================================
 * TASKS FREERTOS
 * ============================================================================ */

void ButtonTask(void *argument)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    while(1)
    {
        Buttons_Scan();
        for(int i = BTN_UP; i <= BTN_MENU; i++) {
            if(Buttons_WasPressed((Button_t)i)) {
                Button_t btn = (Button_t)i;
                xQueueSend(ButtonQueue, &btn, 0);
            }
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(20));
    }
}

void DisplayTask(void *argument)
{
    Button_t received_button;
    while(1)
    {
        if(xQueueReceive(ButtonQueue, &received_button, portMAX_DELAY) == pdTRUE) {
            if(xSemaphoreTake(DisplayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                
                SSD1306_Fill(SSD1306_COLOR_BLACK);
                SSD1306_DrawRectangle(0, 0, 127, 63, SSD1306_COLOR_WHITE);
                
                switch(received_button) {
                    case BTN_UP: SSD1306_DrawLine(64, 10, 64, 53, SSD1306_COLOR_WHITE); break;
                    case BTN_DOWN: SSD1306_DrawLine(64, 53, 64, 10, SSD1306_COLOR_WHITE); break;
                    case BTN_LEFT: SSD1306_DrawLine(10, 32, 117, 32, SSD1306_COLOR_WHITE); break;
                    case BTN_RIGHT: SSD1306_DrawLine(117, 32, 10, 32, SSD1306_COLOR_WHITE); break;
                    case BTN_ENTER: SSD1306_DrawRectangle(40, 20, 48, 24, SSD1306_COLOR_WHITE); break;
                    case BTN_BACK: SSD1306_DrawLine(20, 20, 107, 43, SSD1306_COLOR_WHITE); break;
                    case BTN_MENU: SSD1306_DrawLine(20, 15, 107, 15, SSD1306_COLOR_WHITE); break;
                    default: break;
                }
                
                SSD1306_UpdateScreen();
                xSemaphoreGive(DisplayMutex);
            }
        }
    }
}

void LEDTask(void *argument)
{
    while(1)
    {
        /* Il vero battito cardiaco del sistema: 500ms acceso, 500ms spento */
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ============================================================================
 * CONFIG HARDWARE E CALLBACKS
 * ============================================================================ */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
    RCC_OscInitStruct.PLL.PLLN = 85;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Configurazione LED Pin PA5 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void MX_I2C1_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10909CEC;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
    HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);
    HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);
}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize ) 
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize ) 
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/* Questa dice ad HAL che il tempo scorre grazie a TIM6 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        /* LAMPEGGIO LENTO: circa 1 secondo. Se vedi questo, l'Heap è ancora troppo piccolo! */
        for(volatile uint32_t delay = 0; delay < 15000000; delay++);
    }
}