/**
 ******************************************************************************
 * @file           : display_ssd1306.h
 * @brief          : Driver per display OLED SSD1306 I2C
 * @author         : Il tuo nome
 ******************************************************************************
 * Questo modulo gestisce la comunicazione con il display OLED SSD1306 via I2C
 * Supporta testo, grafica base e controllo del contrasto.
 ******************************************************************************
 */

#ifndef DISPLAY_SSD1306_H
#define DISPLAY_SSD1306_H

#include "stm32g4xx_hal.h"
#include "app_config.h"
#include <stdbool.h>

/* ============================================================================
 * ENUMERAZIONI
 * ============================================================================ */
typedef enum {
    SSD1306_COLOR_BLACK = 0x00,  // Pixel spento
    SSD1306_COLOR_WHITE = 0x01   // Pixel acceso
} SSD1306_Color_t;

/* ============================================================================
 * FUNZIONI PUBBLICHE
 * ============================================================================ */

/**
 * @brief Inizializza il display SSD1306
 * @param hi2c Puntatore alla struttura I2C_HandleTypeDef
 * @retval true se inizializzazione riuscita, false altrimenti
 */
bool SSD1306_Init(I2C_HandleTypeDef *hi2c);

/**
 * @brief Aggiorna il display con il contenuto del buffer
 */
void SSD1306_UpdateScreen(void);

/**
 * @brief Riempie tutto lo schermo con un colore
 * @param color Colore (BLACK o WHITE)
 */
void SSD1306_Fill(SSD1306_Color_t color);

/**
 * @brief Disegna un pixel
 * @param x Coordinata X (0-127)
 * @param y Coordinata Y (0-63)
 * @param color Colore del pixel
 */
void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_Color_t color);

/**
 * @brief Scrive una stringa sul display
 * @param x Coordinata X di partenza
 * @param y Coordinata Y di partenza
 * @param str Stringa da scrivere
 * @param color Colore del testo
 * @retval true se successo
 */
bool SSD1306_WriteString(uint8_t x, uint8_t y, const char *str, SSD1306_Color_t color);

/**
 * @brief Scrive un carattere sul display
 * @param x Coordinata X
 * @param y Coordinata Y
 * @param ch Carattere da scrivere
 * @param color Colore del testo
 * @retval true se successo
 */
bool SSD1306_WriteChar(uint8_t x, uint8_t y, char ch, SSD1306_Color_t color);

/**
 * @brief Disegna una linea
 * @param x0, y0 Coordinate punto iniziale
 * @param x1, y1 Coordinate punto finale
 * @param color Colore della linea
 */
void SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, SSD1306_Color_t color);

/**
 * @brief Disegna un rettangolo
 * @param x, y Coordinate angolo superiore sinistro
 * @param w Larghezza
 * @param h Altezza
 * @param color Colore
 */
void SSD1306_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, SSD1306_Color_t color);

/**
 * @brief Test del display (stampa messaggi di test)
 */
void SSD1306_Test(void);

#endif /* DISPLAY_SSD1306_H */
