/**
 ******************************************************************************
 * @file           : app_config.h
 * @brief          : Configurazione centrale dell'applicazione
 * @author         : Il tuo nome
 ******************************************************************************
 * Questo file contiene tutte le configurazioni hardware e costanti
 * del progetto, centralizzate per facile manutenzione.
 * 
 * Hardware: STM32 Nucleo G474RE + SSD1306 OLED + 7 Pulsanti
 ******************************************************************************
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "stm32g4xx_hal.h"

/* ============================================================================
 * CONFIGURAZIONE DISPLAY SSD1306 OLED
 * ============================================================================ */
#define SSD1306_I2C_ADDR        0x78  // Indirizzo I2C (0x3C << 1)
#define SSD1306_WIDTH           128   // Larghezza display in pixel
#define SSD1306_HEIGHT          64    // Altezza display in pixel (può essere anche 32)

/* ============================================================================
 * CONFIGURAZIONE PULSANTI - NUCLEO G474RE
 * ============================================================================ */
// Pin dei pulsanti distribuiti sui connettori CN7 e CN10
// Uso pin facilmente accessibili sui connettori Arduino

// Connettore CN7 (lato sinistro)
#define BUTTON_LEFT_PIN           GPIO_PIN_0   // PA0 - CN7 pin 28 (A0)
#define BUTTON_UP_PIN         GPIO_PIN_1   // PA1 - CN7 pin 30 (A1)
#define BUTTON_DOWN_PIN         GPIO_PIN_4   // PA4 - CN7 pin 32 (A2)
#define BUTTON_RIGHT_PIN        GPIO_PIN_0   // PB0 - CN7 pin 34 (A3)

// Connettore CN10 (lato destro)
#define BUTTON_ENTER_PIN        GPIO_PIN_13   // PB13 -  pin 30 
#define BUTTON_BACK_PIN         GPIO_PIN_14   // PB14 -  pin 28 
#define BUTTON_MENU_PIN         GPIO_PIN_15  // PB15 -  pin 26 


// PB15 26
// PB14 28
// PB13 30

// Port GPIO per ogni pulsante
#define BUTTON_UP_PORT          GPIOA
#define BUTTON_DOWN_PORT        GPIOA
#define BUTTON_LEFT_PORT        GPIOA
#define BUTTON_RIGHT_PORT       GPIOB
#define BUTTON_ENTER_PORT       GPIOB
#define BUTTON_BACK_PORT        GPIOB
#define BUTTON_MENU_PORT        GPIOB

// Tempo di debounce in ms
#define BUTTON_DEBOUNCE_MS      50

/* ============================================================================
 * CONFIGURAZIONE I2C
 * ============================================================================ */
// I2C1 sui pin standard Arduino (D14=SDA, D15=SCL)
// PB9 = I2C1_SDA (CN10 pin 5 - D14)
// PB8 = I2C1_SCL (CN10 pin 3 - D15)

/* ============================================================================
 * ENUMERAZIONI
 * ============================================================================ */
typedef enum {
    BTN_NONE = 0,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_ENTER,
    BTN_BACK,
    BTN_MENU
} Button_t;

#endif /* APP_CONFIG_H */
