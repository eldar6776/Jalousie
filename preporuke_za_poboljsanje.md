# Preporuke za poboljšanje koda

Nakon analize `main.c` fajla, evo nekoliko preporuka za poboljšanje koda:

## 1. Organizacija koda

Trenutno, `main.c` fajl sadrži svu logiku aplikacije, što ga čini veoma dugim i teškim za održavanje. Preporučuje se podela koda u manje, logičke module. Na primer:

*   **jalousie_control.c/.h:** Funkcije za kontrolu žaluzina (`jalousie_control`, `jalousie_ports_up`, `jalousie_pins_up`, itd.).
*   **led_matrix.c/.h:** Funkcije za kontrolu LED matrice (`refresh_led_matrix`, `led_state`, `start_effect`, `update_snake`).
*   **button_handler.c/.h:** Funkcije za obradu tastera (`handle_buttons`, `menu_logic`).
*   **rs485_com.c/.h:** Funkcije za RS485 komunikaciju i TinyFrame protokol (`RS485_Init`, `RS485_Tick`, `TF_WriteImpl`, i svi `Listener`-i).
*   **eeprom_handler.c/.h:** Funkcije za rad sa EEPROM-om (`ee_save`, `ee_load`, `ee_check`).

Ova podela bi znatno poboljšala čitljivost, olakšala testiranje i buduće izmene.

## 2. Korišćenje "magičnih brojeva"

U kodu se na više mesta koriste neimenovani brojevi (magični brojevi), što otežava razumevanje.

**Primer:**
```c
if (jalousie[i] == 1) { ... }
else if (jalousie[i] == 2) { ... }

// ...

jalousie[idx] = (msg->data[2] < 3) ? msg->data[2] : 0;
```

**Preporuka:**
Definisati `enum` za stanja žaluzina:
```c
typedef enum {
    JALOUSIE_STATE_OFF = 0,
    JALOUSIE_STATE_UP = 1,
    JALOUSIE_STATE_DOWN = 2
} JalousieState_t;
```
I onda koristiti `JALOUSIE_STATE_UP`, `JALOUSIE_STATE_DOWN`, itd. u kodu. Ovo čini kod mnogo čitljivijim i smanjuje mogućnost greške.

## 3. Upravljanje greškama

Upravljanje greškama je veoma osnovno. Uglavnom se postavlja flag `eesta` ili se vraća `1`.

**Preporuka:**
Implementirati centralizovani mehanizam za upravljanje greškama. Na primer, funkcija `ErrorHandler(ErrorCode_t error)` koja bi mogla da loguje grešku (npr. preko UART-a u debug modu) i preduzme odgovarajuću akciju (npr. restartuje modul, upali LED za grešku, itd.).

## 4. Blokirajuća kašnjenja (`HAL_Delay`)

Korišćenje `HAL_Delay()` blokira izvršavanje ostatka koda. Ovo je posebno problematično u `jalousie_control` i `handle_buttons` funkcijama, jer zaustavlja ceo `while(1)` loop.

**Primer:**
```c
HAL_GPIO_WritePin(jalousie_ports_down[i], jalousie_pins_down[i], GPIO_PIN_RESET);
HAL_Delay(20); // Blokira sve na 20ms
HAL_GPIO_WritePin(jalousie_ports_up[i], jalousie_pins_up[i], GPIO_PIN_SET);
```

**Preporuka:**
Zameniti `HAL_Delay()` sa neblokirajućim mehanizmima koji koriste tajmere. Na primer, umesto `HAL_Delay(20)`, može se postaviti tajmer koji će nakon 20ms pozvati callback funkciju ili postaviti flag koji će u sledećoj iteraciji glavne petlje izvršiti potrebnu akciju. Ovo omogućava da mikrokontroler obavlja druge zadatke (npr. proverava RS485 komunikaciju) dok čeka.

## 5. Pojednostavljenje koda

Postoje delovi koda koji se mogu pojednostaviti.

**Primer:**
U `jalousie_control` funkciji, pokretanje tajmera se ponavlja:
```c
if (jalousie[i] == 1) {
    // ...
    if (jalousie_tmr[i] == 0) {
        jalousie_tmr[i] = elapsed_ms;
    }
}
else if (jalousie[i] == 2) {
    // ...
    if (jalousie_tmr[i] == 0) {
        jalousie_tmr[i] = elapsed_ms;
    }
}
```
**Preporuka:**
Ovaj deo se može izvući van `if-else` bloka:
```c
if (jalousie[i] == JALOUSIE_STATE_UP || jalousie[i] == JALOUSIE_STATE_DOWN) {
    if (jalousie_tmr[i] == 0) {
        jalousie_tmr[i] = elapsed_ms;
    }
    // ... ostatak logike
} else {
    // ...
}
```

## 6. Konzistentnost u komentarima i imenovanju

Komentari su mešavina engleskog i srpskog/hrvatskog/bosanskog jezika. Takođe, neka imena varijabli nisu dovoljno deskriptivna.

**Primer:**
*   `eesta` - nije jasno šta znači. Bolje bi bilo `isEepromError` ili `eeprom_status`.
*   `rec` - bolje bi bilo `uart_rx_byte`.
*   Komentari poput `// sačuvaj sve timoute u eeprom` i `// softverski restart modula žaluzina` bi trebali biti na istom jeziku kao i ostatak koda.

**Preporuka:**
Odabrati jedan jezik (preporučljivo engleski) i držati ga se kroz ceo projekat, kako u komentarima, tako i u imenovanju varijabli i funkcija.

## 7. Korišćenje `struct` za organizaciju podataka

Podaci vezani za jednu žaluzinu su razbacani po različitim nizovima (`jalousie`, `jalousie_tmr`, `jalousie_timeout`, `jalousie_ports_up`, itd.).

**Preporuka:**
Kreirati `struct` koji će sadržati sve podatke za jednu žaluzinu:
```c
typedef struct {
    JalousieState_t state;
    uint32_t timer;
    uint32_t timeout;
    GPIO_TypeDef* port_up;
    uint16_t pin_up;
    GPIO_TypeDef* port_down;
    uint16_t pin_down;
} Jalousie_t;

Jalousie_t jalousies[JALOUSIE_COUNT];
```
Ovo čini kod mnogo organizovanijim, lakšim za čitanje i smanjuje mogućnost grešaka pri pristupu podacima.
