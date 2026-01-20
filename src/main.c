#include "stm32g4xx_hal.h"
#include <math.h>

// Handles
DAC_HandleTypeDef hdac1;
TIM_HandleTypeDef htim6;
DMA_HandleTypeDef hdma_dac1_ch1;

// Lookup table sinusoide (256 campioni, 12-bit resolution)
#define SINE_SAMPLES 256
uint16_t sine_wave[SINE_SAMPLES];

// Frequenze delle note (Ottava 4: Do4 a Si4)
const float note_frequencies[12] = {
    261.63,  // Do4  (C4)
    277.18,  // Do#4 (C#4)
    293.66,  // Re4  (D4)
    311.13,  // Re#4 (D#4)
    329.63,  // Mi4  (E4)
    349.23,  // Fa4  (F4)
    369.99,  // Fa#4 (F#4)
    392.00,  // Sol4 (G4)
    415.30,  // Sol#4(G#4)
    440.00,  // La4  (A4)
    466.16,  // La#4 (A#4)
    493.88   // Si4  (B4)
};

const char* note_names[12] = {
    "Do4", "Do#4", "Re4", "Re#4", "Mi4", "Fa4",
    "Fa#4", "Sol4", "Sol#4", "La4", "La#4", "Si4"
};

// Variabili per gestione pulsante
volatile uint8_t current_note = 9;  // Inizia con La4 (440 Hz)
volatile uint32_t last_button_press = 0;
#define DEBOUNCE_DELAY 200  // 200ms debounce

// Function prototypes
void SystemClock_Config(void);
void Error_Handler(void);
void DAC_Init(void);
void TIM6_Init(void);
void DMA_Init(void);
void GPIO_Init(void);
void Generate_Sine_Wave(void);
void Update_Frequency(uint8_t note_index);

// Generate sine wave lookup table
void Generate_Sine_Wave(void) {
    for (int i = 0; i < SINE_SAMPLES; i++) {
        // Genera sinusoide: valore tra 0 e 4095 (12-bit)
        // Offset a 2048 per centrare, ampiezza 1800 (ridotta per evitare distorsione)
        float angle = 2.0f * M_PI * i / SINE_SAMPLES;
        sine_wave[i] = (uint16_t)(2048 + 1800 * sinf(angle));
    }
}

// Aggiorna frequenza del timer per la nota selezionata
void Update_Frequency(uint8_t note_index) {
    if (note_index >= 12) note_index = 0;
    
    // Sample rate = frequenza nota × numero campioni
    uint32_t sample_rate = (uint32_t)(note_frequencies[note_index] * SINE_SAMPLES);
    
    // Calcola periodo timer (assumendo clock a 170 MHz)
    // Se usi HSI a 16 MHz, usa 16000000 invece di 170000000
    uint32_t timer_period = (170000000 / sample_rate) - 1;
    
    // Ferma timer e DAC
    HAL_TIM_Base_Stop(&htim6);
    HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
    
    // Aggiorna periodo
    __HAL_TIM_SET_AUTORELOAD(&htim6, timer_period);
    
    // Riavvia
    HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)sine_wave, 
                      SINE_SAMPLES, DAC_ALIGN_12B_R);
    HAL_TIM_Base_Start(&htim6);
}

int main(void) {
    // Inizializzazione HAL
    HAL_Init();
    
    // Configura clock (usa PLL per 170MHz o lascia HSI a 16MHz)
    SystemClock_Config();
    
    // Genera lookup table sinusoide
    Generate_Sine_Wave();
    
    // Inizializza GPIO (LED, DAC, Pulsante)
    GPIO_Init();
    
    // Inizializza periferiche audio
    DMA_Init();
    DAC_Init();
    TIM6_Init();
    
    // Avvia generazione onda con la nota corrente (La4 - 440 Hz)
    Update_Frequency(current_note);
    
    // Lampeggia LED veloce 3 volte per conferma avvio
    for(int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_Delay(100);
    }
    
    while (1) {
        // Controlla pulsante USER (PC13)
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {  // Pulsante premuto (active low)
            uint32_t current_time = HAL_GetTick();
            
            // Debouncing: ignora se premuto troppo presto
            if ((current_time - last_button_press) > DEBOUNCE_DELAY) {
                last_button_press = current_time;
                
                // Passa alla nota successiva
                current_note = (current_note + 1) % 12;
                Update_Frequency(current_note);
                
                // Lampeggia LED per feedback visivo
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                HAL_Delay(50);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                
                // Aspetta rilascio pulsante
                while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
                    HAL_Delay(10);
                }
            }
        }
        
        HAL_Delay(10);  // Polling ogni 10ms
    }
}

void GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Abilita clock GPIO
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Configura PA5 come output (LED verde - per debug)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configura PA4 come analog (DAC1_OUT1)
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configura PC13 come input (Pulsante USER - con pull-up interno)
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // Pull-up perché il pulsante è active-low
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void DMA_Init(void) {
    // Abilita clock DMA1 e DMAMUX
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    
    // Configura DMA per DAC1 Channel 1
    hdma_dac1_ch1.Instance = DMA1_Channel1;
    hdma_dac1_ch1.Init.Request = DMA_REQUEST_DAC1_CHANNEL1;
    hdma_dac1_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_dac1_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dac1_ch1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dac1_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dac1_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_dac1_ch1.Init.Mode = DMA_CIRCULAR;
    hdma_dac1_ch1.Init.Priority = DMA_PRIORITY_HIGH;
    
    if (HAL_DMA_Init(&hdma_dac1_ch1) != HAL_OK) {
        Error_Handler();
    }
    
    // Link DMA al DAC
    __HAL_LINKDMA(&hdac1, DMA_Handle1, hdma_dac1_ch1);
}

void DAC_Init(void) {
    DAC_ChannelConfTypeDef sConfig = {0};
    
    // Abilita clock DAC1
    __HAL_RCC_DAC1_CLK_ENABLE();
    
    // Configura DAC1
    hdac1.Instance = DAC1;
    if (HAL_DAC_Init(&hdac1) != HAL_OK) {
        Error_Handler();
    }
    
    // Configura canale DAC1_OUT1
    sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
    sConfig.DAC_DMADoubleDataMode = DISABLE;
    sConfig.DAC_SignedFormat = DISABLE;
    sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
    sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL;
    sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
    
    if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
}

void TIM6_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    
    // Abilita clock TIM6
    __HAL_RCC_TIM6_CLK_ENABLE();
    
    // Inizializza con La4 (440 Hz)
    uint32_t sample_rate = (uint32_t)(440.0f * SINE_SAMPLES);  // 112640 Hz
    uint32_t timer_period = (170000000 / sample_rate) - 1;  // Con PLL a 170MHz
    // Se usi HSI: uint32_t timer_period = (16000000 / sample_rate) - 1;
    
    // Configura TIM6
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 0;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = timer_period;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
        Error_Handler();
    }
    
    // Configura TRGO per trigger DAC
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configura voltage scaling per massime prestazioni
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

    // Configura HSI con PLL per 170 MHz
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;     // 16MHz / 4 = 4MHz
    RCC_OscInitStruct.PLL.PLLN = 85;                 // 4MHz * 85 = 340MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;     // 340MHz / 2 = 170MHz
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;     // 340MHz / 2 = 170MHz (SYSCLK)
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // Usa PLL come clock di sistema
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_8) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void) {
    // Lampeggia LED velocissimo in caso di errore
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    while (1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        for(volatile int i = 0; i < 50000; i++);
    }
}

// Required for HAL
void SysTick_Handler(void) {
    HAL_IncTick();
}

void NMI_Handler(void) {}
void HardFault_Handler(void) { Error_Handler(); }
void MemManage_Handler(void) { Error_Handler(); }
void BusFault_Handler(void) { Error_Handler(); }
void UsageFault_Handler(void) { Error_Handler(); }
void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}