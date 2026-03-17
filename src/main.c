/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Progetto: Sistema con Display SSD1306 e 7 Pulsanti
 * Board: STM32 Nucleo G474RE
 * 
 * Hardware:
 * - Display OLED SSD1306 128x64 su I2C1 (PB8=SCL, PB9=SDA)
 * - 7 Pulsanti su PA0, PA1, PA4, PB0, PA8, PA9, PA10
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_config.h"
#include "display_ssd1306.h"
#include "buttons.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    /* USER CODE BEGIN 2 */
    printf("\r\n");
    printf("========================================\r\n");
    printf("  Sistema STM32 Nucleo G474RE\r\n");
    printf("  Display: SSD1306 OLED 128x64\r\n");
    printf("  Input: 7 Pulsanti\r\n");
    printf("========================================\r\n\r\n");

    // Inizializza il display SSD1306
    printf("[MAIN] Inizializzazione display SSD1306...\r\n");
    if (SSD1306_Init(&hi2c1)) {
        printf("[MAIN] Display OK!\r\n");
    } else {
        printf("[MAIN] ERRORE: Display non risponde!\r\n");
    }

    // Inizializza i pulsanti
    printf("[MAIN] Inizializzazione pulsanti...\r\n");
    Buttons_Init();
    printf("[MAIN] Pulsanti OK!\r\n\r\n");

    // Test del display
    printf("[MAIN] Esecuzione test display...\r\n");
    SSD1306_Test();
    HAL_Delay(2000);

    // Schermata iniziale
    SSD1306_Fill(SSD1306_COLOR_BLACK);
    SSD1306_DrawRectangle(0, 0, 127, 63, SSD1306_COLOR_WHITE);
    SSD1306_DrawLine(10, 20, 117, 20, SSD1306_COLOR_WHITE);
    SSD1306_UpdateScreen();

    printf("[MAIN] Sistema pronto!\r\n");
    printf("[MAIN] Premi i pulsanti per testare...\r\n\r\n");

    /* USER CODE END 2 */

    /* Infinite loop */
    uint32_t last_scan = 0;
    const char *button_names[] = {
        "NONE", "UP", "DOWN", "LEFT", "RIGHT", "ENTER", "BACK", "MENU"
    };

    while (1)
    {
        /* USER CODE BEGIN WHILE */
        
        // Scansiona i pulsanti ogni 10ms
        if (HAL_GetTick() - last_scan >= 10) {
            last_scan = HAL_GetTick();
            Buttons_Scan();
            
            // Rileva pressione pulsanti e stampa su UART
            for (int i = BTN_UP; i <= BTN_MENU; i++) {
                if (Buttons_WasPressed((Button_t)i)) {
                    printf("[MAIN] Pulsante %s premuto!\r\n", button_names[i]);
                    
                    // Feedback visivo sul display
                    SSD1306_Fill(SSD1306_COLOR_BLACK);
                    SSD1306_DrawRectangle(20, 20, 80, 30, SSD1306_COLOR_WHITE);
                    // Qui potresti scrivere il nome del pulsante (quando implementi i font)
                    SSD1306_UpdateScreen();
                }
            }
        }

        /* USER CODE END WHILE */
    }
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
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
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
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10909CEC;  // 100kHz @ 170MHz
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        // LED lampeggiante in caso di errore
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // LED onboard
        HAL_Delay(100);
    }
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
