/**
 ******************************************************************************
 * @file           : buttons.h
 * @brief          : Driver per gestione pulsanti
 * @author         : Il tuo nome
 ******************************************************************************
 * Questo modulo gestisce i 7 pulsanti con debouncing software
 ******************************************************************************
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include "stm32g4xx_hal.h"
#include "app_config.h"
#include <stdbool.h>

/* ============================================================================
 * FUNZIONI PUBBLICHE
 * ============================================================================ */

/**
 * @brief Inizializza i GPIO per i pulsanti
 * @note Configura i pin come input con pull-up interne
 */
void Buttons_Init(void);

/**
 * @brief Scansiona i pulsanti (chiamare periodicamente, es. ogni 10ms)
 * @note Gestisce il debouncing automaticamente
 */
void Buttons_Scan(void);

/**
 * @brief Verifica se un pulsante è attualmente premuto
 * @param button Pulsante da verificare
 * @retval true se premuto, false altrimenti
 */
bool Buttons_IsPressed(Button_t button);

/**
 * @brief Verifica se un pulsante è stato appena premuto (edge detection)
 * @param button Pulsante da verificare
 * @retval true se appena premuto (solo primo rilevamento)
 */
bool Buttons_WasPressed(Button_t button);

/**
 * @brief Verifica se un pulsante è stato appena rilasciato
 * @param button Pulsante da verificare
 * @retval true se appena rilasciato
 */
bool Buttons_WasReleased(Button_t button);

/**
 * @brief Ottiene il pulsante premuto (con priorità)
 * @retval Button_t Il primo pulsante premuto rilevato, BTN_NONE se nessuno
 */
Button_t Buttons_GetPressed(void);

/**
 * @brief Test dei pulsanti (stampa eventi su UART)
 */
void Buttons_Test(void);

#endif /* BUTTONS_H */
