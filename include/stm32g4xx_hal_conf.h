#ifndef __STM32G4xx_HAL_CONF_H
#define __STM32G4xx_HAL_CONF_H

#define HAL_MODULE_ENABLED
#define HAL_DAC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED

// System configuration
#define HSE_VALUE    24000000U
#define HSI_VALUE    16000000U
#define VDD_VALUE    3300U
#define TICK_INT_PRIORITY 15U
#define USE_RTOS     0U
#define PREFETCH_ENABLE 1U
#define INSTRUCTION_CACHE_ENABLE 1U
#define DATA_CACHE_ENABLE 1U

#include "stm32g4xx_hal_rcc.h"
#include "stm32g4xx_hal_rcc_ex.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_dma.h"
#include "stm32g4xx_hal_dma_ex.h"
#include "stm32g4xx_hal_cortex.h"
#include "stm32g4xx_hal_dac.h"
#include "stm32g4xx_hal_dac_ex.h"
#include "stm32g4xx_hal_tim.h"
#include "stm32g4xx_hal_tim_ex.h"
#include "stm32g4xx_hal_pwr.h"
#include "stm32g4xx_hal_pwr_ex.h"
#include "stm32g4xx_hal_flash.h"
#include "stm32g4xx_hal_flash_ex.h"
#include "stm32g4xx_hal_exti.h"

#endif