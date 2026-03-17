/**
 ******************************************************************************
 * @file           : display_ssd1306.c
 * @brief          : Implementazione driver display SSD1306
 ******************************************************************************
 */

#include "display_ssd1306.h"
#include "fonts.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * VARIABILI PRIVATE
 * ============================================================================ */
static I2C_HandleTypeDef *ssd1306_i2c;
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/* ============================================================================
 * COMANDI SSD1306
 * ============================================================================ */
#define SSD1306_CMD_DISPLAY_OFF         0xAE
#define SSD1306_CMD_DISPLAY_ON          0xAF
#define SSD1306_CMD_SET_CONTRAST        0x81
#define SSD1306_CMD_NORMAL_DISPLAY      0xA6
#define SSD1306_CMD_INVERSE_DISPLAY     0xA7
#define SSD1306_CMD_SET_MUX_RATIO       0xA8
#define SSD1306_CMD_SET_DISPLAY_OFFSET  0xD3
#define SSD1306_CMD_SET_START_LINE      0x40
#define SSD1306_CMD_SEGMENT_REMAP       0xA1
#define SSD1306_CMD_COM_SCAN_DEC        0xC8
#define SSD1306_CMD_SET_COM_PINS        0xDA
#define SSD1306_CMD_SET_PRECHARGE       0xD9
#define SSD1306_CMD_SET_VCOMH           0xDB
#define SSD1306_CMD_CHARGE_PUMP         0x8D
#define SSD1306_CMD_ADDRESSING_MODE     0x20
#define SSD1306_CMD_SET_COLUMN_ADDR     0x21
#define SSD1306_CMD_SET_PAGE_ADDR       0x22

/* ============================================================================
 * FUNZIONI PRIVATE
 * ============================================================================ */

/**
 * @brief Invia un comando al display
 */
static bool SSD1306_WriteCommand(uint8_t cmd)
{
    uint8_t data[2] = {0x00, cmd}; // 0x00 = Co=0, DC=0 (command)
    return HAL_I2C_Master_Transmit(ssd1306_i2c, SSD1306_I2C_ADDR, data, 2, HAL_MAX_DELAY) == HAL_OK;
}

/**
 * @brief Invia dati al display
 */
static bool SSD1306_WriteData(uint8_t *data, size_t size)
{
    uint8_t buffer[size + 1];
    buffer[0] = 0x40; // 0x40 = Co=0, DC=1 (data)
    memcpy(buffer + 1, data, size);
    return HAL_I2C_Master_Transmit(ssd1306_i2c, SSD1306_I2C_ADDR, buffer, size + 1, HAL_MAX_DELAY) == HAL_OK;
}

/* ============================================================================
 * FUNZIONI PUBBLICHE
 * ============================================================================ */

bool SSD1306_Init(I2C_HandleTypeDef *hi2c)
{
    ssd1306_i2c = hi2c;
    
    // Attendi stabilizzazione
    HAL_Delay(100);
    
    // Sequenza di inizializzazione
    SSD1306_WriteCommand(SSD1306_CMD_DISPLAY_OFF);
    
    SSD1306_WriteCommand(SSD1306_CMD_SET_MUX_RATIO);
    SSD1306_WriteCommand(SSD1306_HEIGHT - 1);
    
    SSD1306_WriteCommand(SSD1306_CMD_SET_DISPLAY_OFFSET);
    SSD1306_WriteCommand(0x00);
    
    SSD1306_WriteCommand(SSD1306_CMD_SET_START_LINE | 0x00);
    
    SSD1306_WriteCommand(SSD1306_CMD_SEGMENT_REMAP);
    SSD1306_WriteCommand(SSD1306_CMD_COM_SCAN_DEC);
    
    SSD1306_WriteCommand(SSD1306_CMD_SET_COM_PINS);
    SSD1306_WriteCommand(0x12); // 0x12 per 64 righe, 0x02 per 32 righe
    
    SSD1306_WriteCommand(SSD1306_CMD_SET_CONTRAST);
    SSD1306_WriteCommand(0x7F);
    
    SSD1306_WriteCommand(SSD1306_CMD_NORMAL_DISPLAY);
    
    SSD1306_WriteCommand(SSD1306_CMD_SET_PRECHARGE);
    SSD1306_WriteCommand(0xF1);
    
    SSD1306_WriteCommand(SSD1306_CMD_SET_VCOMH);
    SSD1306_WriteCommand(0x40);
    
    SSD1306_WriteCommand(SSD1306_CMD_CHARGE_PUMP);
    SSD1306_WriteCommand(0x14); // Enable charge pump
    
    SSD1306_WriteCommand(SSD1306_CMD_ADDRESSING_MODE);
    SSD1306_WriteCommand(0x00); // Horizontal addressing mode
    
    SSD1306_WriteCommand(SSD1306_CMD_DISPLAY_ON);
    
    // Pulisci buffer e schermo
    SSD1306_Fill(SSD1306_COLOR_BLACK);
    SSD1306_UpdateScreen();
    
    printf("[SSD1306] Display inizializzato correttamente!\r\n");
    return true;
}

void SSD1306_UpdateScreen(void)
{
    SSD1306_WriteCommand(SSD1306_CMD_SET_COLUMN_ADDR);
    SSD1306_WriteCommand(0);   // Column start
    SSD1306_WriteCommand(127); // Column end
    
    SSD1306_WriteCommand(SSD1306_CMD_SET_PAGE_ADDR);
    SSD1306_WriteCommand(0);   // Page start
    SSD1306_WriteCommand(7);   // Page end (8 pages per 64 righe)
    
    SSD1306_WriteData(SSD1306_Buffer, sizeof(SSD1306_Buffer));
}

void SSD1306_Fill(SSD1306_Color_t color)
{
    memset(SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_Color_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }
    
    if (color == SSD1306_COLOR_WHITE) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

bool SSD1306_WriteChar(uint8_t x, uint8_t y, char ch, SSD1306_Color_t color)
{
    // Implementazione semplificata - usa font 7x10 (da implementare fonts.h)
    // Per ora stampiamo un rettangolo al posto del carattere
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return false;
    }
    
    // TODO: Implementare font reale
    // Per ora disegniamo un piccolo rettangolo come placeholder
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 10; j++) {
            SSD1306_DrawPixel(x + i, y + j, color);
        }
    }
    
    return true;
}

bool SSD1306_WriteString(uint8_t x, uint8_t y, const char *str, SSD1306_Color_t color)
{
    uint8_t cur_x = x;
    
    while (*str) {
        if (cur_x + 7 > SSD1306_WIDTH) {
            return false;
        }
        
        SSD1306_WriteChar(cur_x, y, *str, color);
        cur_x += 8; // 7 pixel + 1 spazio
        str++;
    }
    
    return true;
}

void SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, SSD1306_Color_t color)
{
    // Algoritmo di Bresenham
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        SSD1306_DrawPixel(x0, y0, color);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void SSD1306_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, SSD1306_Color_t color)
{
    SSD1306_DrawLine(x, y, x + w, y, color);
    SSD1306_DrawLine(x + w, y, x + w, y + h, color);
    SSD1306_DrawLine(x + w, y + h, x, y + h, color);
    SSD1306_DrawLine(x, y + h, x, y, color);
}

void SSD1306_Test(void)
{
    printf("[SSD1306] Inizio test display...\r\n");
    
    // Test 1: Schermo tutto bianco
    SSD1306_Fill(SSD1306_COLOR_WHITE);
    SSD1306_UpdateScreen();
    HAL_Delay(500);
    
    // Test 2: Schermo tutto nero
    SSD1306_Fill(SSD1306_COLOR_BLACK);
    SSD1306_UpdateScreen();
    HAL_Delay(500);
    
    // Test 3: Linee diagonali
    for (int i = 0; i < SSD1306_WIDTH; i += 10) {
        SSD1306_DrawLine(0, 0, i, SSD1306_HEIGHT - 1, SSD1306_COLOR_WHITE);
    }
    SSD1306_UpdateScreen();
    HAL_Delay(1000);
    
    // Test 4: Rettangoli
    SSD1306_Fill(SSD1306_COLOR_BLACK);
    SSD1306_DrawRectangle(10, 10, 40, 30, SSD1306_COLOR_WHITE);
    SSD1306_DrawRectangle(60, 20, 50, 40, SSD1306_COLOR_WHITE);
    SSD1306_UpdateScreen();
    
    printf("[SSD1306] Test completato!\r\n");
}
