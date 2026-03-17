/**
 ******************************************************************************
 * @file           : buttons.c
 * @brief          : Implementazione driver pulsanti
 ******************************************************************************
 */

#include "buttons.h"
#include <stdio.h>

/* ============================================================================
 * STRUTTURA PER GESTIONE STATO PULSANTE
 * ============================================================================ */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    bool current_state;      // Stato attuale (dopo debounce)
    bool previous_state;     // Stato precedente
    uint32_t last_change;    // Timestamp ultimo cambio
    bool pressed_event;      // Flag per evento pressione
    bool released_event;     // Flag per evento rilascio
} Button_State_t;

/* ============================================================================
 * VARIABILI PRIVATE
 * ============================================================================ */
static Button_State_t button_states[8]; // Indice 0 non usato, 1-7 per i pulsanti

/* ============================================================================
 * FUNZIONI PRIVATE
 * ============================================================================ */

/**
 * @brief Legge lo stato fisico di un pulsante (con logica invertita per pull-up)
 */
static bool ReadButtonPhysical(GPIO_TypeDef *port, uint16_t pin)
{
    // I pulsanti sono collegati a GND, quindi HIGH = non premuto, LOW = premuto
    return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET;
}

/**
 * @brief Inizializza una struttura Button_State
 */
static void InitButtonState(Button_State_t *btn, GPIO_TypeDef *port, uint16_t pin)
{
    btn->port = port;
    btn->pin = pin;
    btn->current_state = false;
    btn->previous_state = false;
    btn->last_change = 0;
    btn->pressed_event = false;
    btn->released_event = false;
}

/* ============================================================================
 * FUNZIONI PUBBLICHE
 * ============================================================================ */

void Buttons_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Abilita i clock per le porte GPIO
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // Configura tutti i pin come INPUT con PULL-UP
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    // Button UP (PA0)
    GPIO_InitStruct.Pin = BUTTON_UP_PIN;
    HAL_GPIO_Init(BUTTON_UP_PORT, &GPIO_InitStruct);
    InitButtonState(&button_states[BTN_UP], BUTTON_UP_PORT, BUTTON_UP_PIN);
    
    // Button DOWN (PA1)
    GPIO_InitStruct.Pin = BUTTON_DOWN_PIN;
    HAL_GPIO_Init(BUTTON_DOWN_PORT, &GPIO_InitStruct);
    InitButtonState(&button_states[BTN_DOWN], BUTTON_DOWN_PORT, BUTTON_DOWN_PIN);
    
    // Button LEFT (PA4)
    GPIO_InitStruct.Pin = BUTTON_LEFT_PIN;
    HAL_GPIO_Init(BUTTON_LEFT_PORT, &GPIO_InitStruct);
    InitButtonState(&button_states[BTN_LEFT], BUTTON_LEFT_PORT, BUTTON_LEFT_PIN);
    
    // Button RIGHT (PB0)
    GPIO_InitStruct.Pin = BUTTON_RIGHT_PIN;
    HAL_GPIO_Init(BUTTON_RIGHT_PORT, &GPIO_InitStruct);
    InitButtonState(&button_states[BTN_RIGHT], BUTTON_RIGHT_PORT, BUTTON_RIGHT_PIN);
    
    // Button ENTER (PA8)
    GPIO_InitStruct.Pin = BUTTON_ENTER_PIN;
    HAL_GPIO_Init(BUTTON_ENTER_PORT, &GPIO_InitStruct);
    InitButtonState(&button_states[BTN_ENTER], BUTTON_ENTER_PORT, BUTTON_ENTER_PIN);
    
    // Button BACK (PA9)
    GPIO_InitStruct.Pin = BUTTON_BACK_PIN;
    HAL_GPIO_Init(BUTTON_BACK_PORT, &GPIO_InitStruct);
    InitButtonState(&button_states[BTN_BACK], BUTTON_BACK_PORT, BUTTON_BACK_PIN);
    
    // Button MENU (PA10)
    GPIO_InitStruct.Pin = BUTTON_MENU_PIN;
    HAL_GPIO_Init(BUTTON_MENU_PORT, &GPIO_InitStruct);
    InitButtonState(&button_states[BTN_MENU], BUTTON_MENU_PORT, BUTTON_MENU_PIN);
    
    printf("[BUTTONS] Inizializzazione completata - 7 pulsanti configurati\r\n");
}

void Buttons_Scan(void)
{
    uint32_t current_time = HAL_GetTick();
    
    for (int i = BTN_UP; i <= BTN_MENU; i++) {
        Button_State_t *btn = &button_states[i];
        
        // Leggi stato fisico del pulsante
        bool physical_state = ReadButtonPhysical(btn->port, btn->pin);
        
        // Debouncing: controlla se è passato abbastanza tempo dall'ultimo cambio
        if (physical_state != btn->current_state) {
            if ((current_time - btn->last_change) >= BUTTON_DEBOUNCE_MS) {
                // Stato stabile, aggiorna
                btn->previous_state = btn->current_state;
                btn->current_state = physical_state;
                btn->last_change = current_time;
                
                // Genera eventi
                if (btn->current_state && !btn->previous_state) {
                    btn->pressed_event = true;
                } else if (!btn->current_state && btn->previous_state) {
                    btn->released_event = true;
                }
            }
        }
    }
}

bool Buttons_IsPressed(Button_t button)
{
    if (button == BTN_NONE || button > BTN_MENU) {
        return false;
    }
    return button_states[button].current_state;
}

bool Buttons_WasPressed(Button_t button)
{
    if (button == BTN_NONE || button > BTN_MENU) {
        return false;
    }
    
    if (button_states[button].pressed_event) {
        button_states[button].pressed_event = false; // Clear flag
        return true;
    }
    return false;
}

bool Buttons_WasReleased(Button_t button)
{
    if (button == BTN_NONE || button > BTN_MENU) {
        return false;
    }
    
    if (button_states[button].released_event) {
        button_states[button].released_event = false; // Clear flag
        return true;
    }
    return false;
}

Button_t Buttons_GetPressed(void)
{
    // Ritorna il primo pulsante premuto trovato (con priorità)
    for (int i = BTN_UP; i <= BTN_MENU; i++) {
        if (button_states[i].current_state) {
            return (Button_t)i;
        }
    }
    return BTN_NONE;
}

void Buttons_Test(void)
{
    printf("[BUTTONS] Modalità test - Premi i pulsanti...\r\n");
    printf("[BUTTONS] UP=PA0, DOWN=PA1, LEFT=PA4, RIGHT=PB0\r\n");
    printf("[BUTTONS] ENTER=PA8, BACK=PA9, MENU=PA10\r\n");
    
    const char *button_names[] = {
        "NONE", "UP", "DOWN", "LEFT", "RIGHT", "ENTER", "BACK", "MENU"
    };
    
    while (1) {
        Buttons_Scan();
        
        for (int i = BTN_UP; i <= BTN_MENU; i++) {
            if (Buttons_WasPressed((Button_t)i)) {
                printf("[BUTTONS] *** %s PREMUTO ***\r\n", button_names[i]);
            }
            if (Buttons_WasReleased((Button_t)i)) {
                printf("[BUTTONS] --- %s rilasciato ---\r\n", button_names[i]);
            }
        }
        
        HAL_Delay(10); // Scan ogni 10ms
    }
}
