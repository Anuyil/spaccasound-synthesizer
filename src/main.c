/**
 ******************************************************************************
 * @file           : main_complete.c
 * @brief          : Sistema Completo: UART + Pulsanti + Display SSD1306
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"
#include "app_config.h"
#include "buttons.h"
#include "display_ssd1306.h"
#include <stdio.h>
#include <string.h>

/* Private variables */
UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c1;

/* Private function prototypes */
void SystemClock_Config(void);
void MX_I2C1_Init(void);

int main(void)
{
    /* HAL Init */
    HAL_Init();
    
    /* Configure the system clock */
    SystemClock_Config();
    
    /* Enable GPIO Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /* ========================================
     * Configure LED (PA5)
     * ======================================== */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* LED lampeggia 3 volte = boot OK */
    for(int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_Delay(100);
    }
    
    /* ========================================
     * Configure USART2
     * ======================================== */
    __HAL_RCC_USART2_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
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
    HAL_UART_Init(&huart2);
    
    HAL_Delay(1000);
    
    /* ========================================
     * MESSAGGI INIZIALI
     * ======================================== */
    printf("\r\n\r\n");
    printf("╔════════════════════════════════════════╗\r\n");
    printf("║   SISTEMA COMPLETO - Nucleo G474RE    ║\r\n");
    printf("║   Display SSD1306 + 7 Pulsanti        ║\r\n");
    printf("╚════════════════════════════════════════╝\r\n");
    printf("\r\n");
    printf("[OK] Sistema avviato!\r\n");
    printf("[OK] UART OK!\r\n");
    
    /* ========================================
     * INIZIALIZZA I2C e DISPLAY
     * ======================================== */
    printf("[INIT] Inizializzazione I2C1...\r\n");
    MX_I2C1_Init();
    printf("[OK] I2C1 OK!\r\n");
    
    printf("[INIT] Inizializzazione Display SSD1306...\r\n");
    if (SSD1306_Init(&hi2c1)) {
        printf("[OK] Display OK!\r\n");
        
        /* Test grafico del display */
        SSD1306_Fill(SSD1306_COLOR_BLACK);
        SSD1306_DrawRectangle(0, 0, 127, 63, SSD1306_COLOR_WHITE);
        SSD1306_DrawLine(10, 10, 117, 53, SSD1306_COLOR_WHITE);
        SSD1306_DrawLine(117, 10, 10, 53, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreen();
        HAL_Delay(1000);
        
        SSD1306_Fill(SSD1306_COLOR_BLACK);
        SSD1306_UpdateScreen();
    } else {
        printf("[ERROR] Display non risponde!\r\n");
    }
    
    /* ========================================
     * INIZIALIZZA PULSANTI
     * ======================================== */
    printf("[INIT] Inizializzazione pulsanti...\r\n");
    Buttons_Init();
    printf("[OK] Pulsanti OK!\r\n");
    printf("\r\n");
    
    printf("Sistema pronto!\r\n");
    printf("Premi i pulsanti per testare...\r\n");
    printf("\r\n");
    
    /* ========================================
     * MAIN LOOP
     * ======================================== */
    uint32_t last_scan = 0;
    uint32_t last_blink = 0;
    uint32_t last_display_update = 0;
    
    const char *button_names[] = {
        "NONE", "UP", "DOWN", "LEFT", "RIGHT", "ENTER", "BACK", "MENU"
    };
    
    char display_text[32] = "Premi un tasto";
    uint8_t update_display = 1;
    
    while (1)
    {
        /* LED lampeggia ogni 500ms = sistema vivo */
        if(HAL_GetTick() - last_blink >= 500) {
            last_blink = HAL_GetTick();
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
        
        /* Scansiona pulsanti ogni 10ms */
        if (HAL_GetTick() - last_scan >= 10) {
            last_scan = HAL_GetTick();
            Buttons_Scan();
            
            /* Rileva pressione pulsanti */
            for (int i = BTN_UP; i <= BTN_MENU; i++) {
                if (Buttons_WasPressed((Button_t)i)) {
                    printf("[BTN] >>> %s PREMUTO! <<<\r\n", button_names[i]);
                    
                    /* Aggiorna testo display */
                    snprintf(display_text, sizeof(display_text), "BTN: %s", button_names[i]);
                    update_display = 1;
                    
                    /* LED flash */
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                    HAL_Delay(50);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                }
            }
        }
        
        /* Aggiorna display ogni 100ms se necessario */
        if(update_display && (HAL_GetTick() - last_display_update >= 100)) {
            last_display_update = HAL_GetTick();
            update_display = 0;
            
            /* Disegna su display */
            SSD1306_Fill(SSD1306_COLOR_BLACK);
            
            /* Bordo */
            SSD1306_DrawRectangle(0, 0, 127, 63, SSD1306_COLOR_WHITE);
            
            /* Titolo (simulato con linee - mancano i font veri) */
            for(int i = 0; i < strlen("STM32"); i++) {
                SSD1306_DrawRectangle(10 + i*12, 10, 10, 8, SSD1306_COLOR_WHITE);
            }
            
            /* Testo pulsante (simulato) */
            for(int i = 0; i < strlen(display_text) && i < 10; i++) {
                SSD1306_DrawRectangle(10 + i*12, 35, 10, 8, SSD1306_COLOR_WHITE);
            }
            
            /* Barra in basso */
            SSD1306_DrawLine(5, 55, 122, 55, SSD1306_COLOR_WHITE);
            
            SSD1306_UpdateScreen();
        }
    }
}

/**
  * @brief System Clock Configuration
  */
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

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
}

/**
  * @brief I2C1 Initialization Function
  */
void MX_I2C1_Init(void)
{
    /* Enable clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    
    /* I2C1 GPIO Configuration: PB8=SCL, PB9=SDA */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* I2C1 configuration */
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10909CEC;  // 100kHz @ 170MHz
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

void SysTick_Handler(void)
{
    HAL_IncTick();
}

int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, 1000);
    return len;
}