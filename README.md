# Sistema STM32 con Display SSD1306 e 7 Pulsanti

## 📋 Hardware
- **Board**: STM32 Nucleo G474RE
- **Display**: SSD1306 OLED 128x64 pixel (I2C)
- **Input**: 7 Pulsanti

---

## 🔌 Schema Collegamento Hardware

### Display SSD1306 (I2C1)
```
SSD1306          Nucleo G474RE
--------         ---------------
VCC      →       3.3V (CN6 pin 4) o 5V (CN6 pin 5)
GND      →       GND (CN6 pin 6)
SCL      →       PB8 (CN10 pin 3 - D15/I2C1_SCL)
SDA      →       PB9 (CN10 pin 5 - D14/I2C1_SDA)
```

### 7 Pulsanti (con pull-up interne)
Ogni pulsante è collegato tra il pin GPIO e GND:

```
Pulsante         Pin STM32       Connettore      Nome Arduino
---------        ----------      -----------     -------------
UP       →       PA0             CN7 pin 28      A0
DOWN     →       PA1             CN7 pin 30      A1
LEFT     →       PA4             CN7 pin 32      A2
RIGHT    →       PB0             CN7 pin 34      A3
ENTER    →       PA8             CN10 pin 23     D7
BACK     →       PA9             CN10 pin 21     D8
MENU     →       PA10            CN10 pin 33     D2
```

**Schema fisico pulsante:**
```
Pin GPIO ----[ Pull-up interna ]---- VDD
         |
         +---- Pulsante ---- GND
```
Quando il pulsante è premuto, il pin va a GND (stato LOW = premuto)

---

## 📁 Struttura del Codice

```
Core/
├── Inc/
│   ├── main.h              - Header principale
│   ├── app_config.h        - Configurazione hardware centralizzata
│   ├── display_ssd1306.h   - API display OLED
│   ├── buttons.h           - API gestione pulsanti
│   └── fonts.h             - Font per display (placeholder)
│
├── Src/
│   ├── main.c              - Main program (pulito e leggibile)
│   ├── display_ssd1306.c   - Implementazione driver display
│   └── buttons.c           - Implementazione driver pulsanti
```

### Principi Architetturali

1. **Modularità**: Ogni periferica ha il suo modulo (.h + .c)
2. **Configurazione centralizzata**: Tutti i pin e costanti in `app_config.h`
3. **API pulite**: Funzioni pubbliche ben documentate
4. **Separazione responsabilità**: Main snello, logica nei moduli
5. **Estendibilità**: Facile aggiungere nuove funzionalità

---

## 🔧 Configurazione STM32CubeIDE

### 1. I2C1 Configuration
- **Mode**: I2C
- **Speed**: Standard Mode (100 kHz)
- **Pins**: 
  - PB8 → I2C1_SCL
  - PB9 → I2C1_SDA

### 2. USART2 Configuration (per debug via printf)
- **Mode**: Asynchronous
- **Baud Rate**: 115200
- **Pins**:
  - PA2 → USART2_TX (ST-Link Virtual COM)
  - PA3 → USART2_RX

### 3. GPIO Configuration
Tutti i pin dei pulsanti sono configurati automaticamente da `Buttons_Init()`:
- Mode: Input
- Pull: Pull-up
- Speed: Low

---

## 💻 Come Usare il Codice

### Step 1: Copia i file nel progetto STM32CubeIDE

```
Copia in Core/Inc/:
- app_config.h
- display_ssd1306.h
- buttons.h
- fonts.h

Copia in Core/Src/:
- main.c (sostituisci quello generato)
- display_ssd1306.c
- buttons.c
```

### Step 2: Compila e carica sulla board

### Step 3: Apri il Serial Monitor
- Baud rate: 115200
- Vedrai i messaggi di debug

### Output atteso:
```
========================================
  Sistema STM32 Nucleo G474RE
  Display: SSD1306 OLED 128x64
  Input: 7 Pulsanti
========================================

[MAIN] Inizializzazione display SSD1306...
[SSD1306] Display inizializzato correttamente!
[MAIN] Display OK!
[MAIN] Inizializzazione pulsanti...
[BUTTONS] Inizializzazione completata - 7 pulsanti configurati
[MAIN] Pulsanti OK!

[MAIN] Esecuzione test display...
[SSD1306] Inizio test display...
[SSD1306] Test completato!
[MAIN] Sistema pronto!
[MAIN] Premi i pulsanti per testare...

[MAIN] Pulsante UP premuto!
[MAIN] Pulsante ENTER premuto!
...
```

---

## 🎯 Funzionalità Implementate

### Display SSD1306
- ✅ Inizializzazione display
- ✅ Buffer grafico completo
- ✅ Funzioni base: Fill, DrawPixel, DrawLine, DrawRectangle
- ✅ Test automatico
- ⏳ Font bitmap (placeholder, da implementare)

### Pulsanti
- ✅ Lettura 7 pulsanti
- ✅ Debouncing software (50ms)
- ✅ Edge detection (pressione/rilascio)
- ✅ API IsPressed, WasPressed, WasReleased
- ✅ Scan periodico (10ms)
- ✅ Test con stampa eventi su UART

---

## 🚀 Prossimi Passi

1. **Implementare font bitmap reale** in `fonts.h`
2. **Aggiungere libreria grafica** (cerchi, triangoli, immagini)
3. **Sistema di menu** navigabile con i pulsanti
4. **State machine** per gestione interfaccia
5. **Salvataggio configurazioni** in EEPROM/Flash

---

## 🐛 Troubleshooting

### Display non risponde
- Verifica indirizzo I2C (può essere 0x78 o 0x7A)
- Controlla collegamenti SDA/SCL
- Prova ad aggiungere resistenze pull-up da 4.7kΩ su SDA e SCL

### Pulsanti non funzionano
- Verifica collegamento a GND
- Controlla i pin nel file `app_config.h`
- Testa con un multimetro la continuità

### UART non stampa
- Verifica driver ST-Link Virtual COM installato
- Controlla baud rate 115200
- Assicurati di aver implementato `_write()` per printf

---

## 📚 Riferimenti

- [STM32G474RE Datasheet](https://www.st.com/resource/en/datasheet/stm32g474re.pdf)
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [HAL User Manual](https://www.st.com/resource/en/user_manual/dm00154093.pdf)

---

## 📄 Licenza
Codice libero per uso educativo e personale.
