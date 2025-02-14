/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Definicije pinova
#define LED_ROWS 3
#define LED_COLS 3

// Definicije pinova
#define LED_R1_Pin GPIO_PIN_4
#define LED_R1_GPIO_Port GPIOA
#define LED_R2_Pin GPIO_PIN_9
#define LED_R2_GPIO_Port GPIOB
#define LED_R3_Pin GPIO_PIN_8
#define LED_R3_GPIO_Port GPIOB

#define LED_C1_Pin GPIO_PIN_7
#define LED_C1_GPIO_Port GPIOB
#define LED_C2_Pin GPIO_PIN_6
#define LED_C2_GPIO_Port GPIOB
#define LED_C3_Pin GPIO_PIN_5
#define LED_C3_GPIO_Port GPIOB

#define JALOUSIE_COUNT 8
#define TIMEOUT_DEFAULT 30000  // 30 sekundi u milisekundama
#define NUMBER_OF_JALOUSIE 8
#define LED_COUNT 9
#define SNAKE_LENGTH 43 // Du�ina zmije
#define SNAKE_SPEED 20   // Brzina zmije (veca vrednost = sporije)
#define MENU_TIMEOUT    10000 // 10 sekundi timeout
#define BUTTON_PAUSE    200 // dodatni debouncing
#define TOGGLE_INTERVAL 50 // Period treptanja LED
#define EEPROM_I2C_ADDR  0xA0  // Prilagodi prema tvojoj EEPROM adresi
#define EEPROM_TIMEOUT   100   // Timeout za I2C operacije
#define EEPROM_START_ADDR 0x00 // Početna adresa u EEPROM-u
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

uint8_t jalousie[JALOUSIE_COUNT] = {0};           // Status žaluzina
uint32_t jalousie_tmr[JALOUSIE_COUNT] = {0};      // Tajmeri žaluzina
uint32_t jalousie_timeout[JALOUSIE_COUNT] = {TIMEOUT_DEFAULT, TIMEOUT_DEFAULT, TIMEOUT_DEFAULT, TIMEOUT_DEFAULT,
                                             TIMEOUT_DEFAULT, TIMEOUT_DEFAULT, TIMEOUT_DEFAULT, TIMEOUT_DEFAULT
                                            };

// Definicije GPIO izlaza za UP i DOWN
GPIO_TypeDef* jalousie_ports_up[JALOUSIE_COUNT] = {JAL0_UP_GPIO_Port, JAL1_UP_GPIO_Port, JAL2_UP_GPIO_Port, JAL3_UP_GPIO_Port,
                                                   JAL4_UP_GPIO_Port, JAL5_UP_GPIO_Port, JAL6_UP_GPIO_Port, JAL7_UP_GPIO_Port
                                                  };
uint16_t jalousie_pins_up[JALOUSIE_COUNT] = {JAL0_UP_Pin, JAL1_UP_Pin, JAL2_UP_Pin, JAL3_UP_Pin,
                                             JAL4_UP_Pin, JAL5_UP_Pin, JAL6_UP_Pin, JAL7_UP_Pin
                                            };

GPIO_TypeDef* jalousie_ports_down[JALOUSIE_COUNT] = {JAL0_DN_GPIO_Port, JAL1_DN_GPIO_Port, JAL2_DN_GPIO_Port, JAL3_DN_GPIO_Port,
                                                     JAL4_DN_GPIO_Port, JAL5_DN_GPIO_Port, JAL6_DN_GPIO_Port, JAL7_DN_GPIO_Port
                                                    };
uint16_t jalousie_pins_down[JALOUSIE_COUNT] = {JAL0_DN_Pin, JAL1_DN_Pin, JAL2_DN_Pin, JAL3_DN_Pin,
                                               JAL4_DN_Pin, JAL5_DN_Pin, JAL6_DN_Pin, JAL7_DN_Pin
                                              };

uint8_t led_state[LED_ROWS * LED_COLS] = {0};
uint8_t active_row = 0;

static uint8_t snake_head = 0;  // Trenutna glava zmije
static uint8_t snake_counter = 0;  // Brojac za usporavanje


volatile uint32_t menu_timer = 0;
volatile uint32_t last_toggle_time = 0;
volatile uint8_t toggle_state = 0;
volatile uint8_t menu_activ = 0;
volatile uint8_t selected_channel = NUMBER_OF_JALOUSIE - 1;
uint8_t button_select_prev = 0, button_up_prev = 0, button_down_prev = 0;
static uint8_t my_address = 0;
static uint8_t first_toggle = 1;
static uint16_t jalousie_start = 0;
static uint16_t jalousie_end = 0;
static uint8_t eesta = 0;
bool ee_save_flag = false;
uint8_t rec;
TinyFrame tfapp;
bool init_tf = false;
bool reset = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_I2C1_Init(void);
static void MX_IWDG_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM14_Init(void);
/* USER CODE BEGIN PFP */
static void RS485_Init(void);
static uint8_t ee_save(void);
static uint8_t ee_load(void);
static void ee_check(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  sačuvaj sve timoute u eeprom
  * @param
  * @retval 0 = uspješno / 1 = neuspješno, ako je neuspješno i eesta flag se setuje 
  */
static uint8_t ee_save(void)
{
    uint8_t temp_value;

    for (uint8_t i = 0; i < JALOUSIE_COUNT; i++) {
        temp_value = (uint8_t)(jalousie_timeout[i] / 1000); // Konverzija u sekunde
        if (HAL_I2C_Mem_Write(&hi2c1, EEPROM_I2C_ADDR, EEPROM_START_ADDR + i, I2C_MEMADD_SIZE_8BIT, &temp_value, 1, EEPROM_TIMEOUT) != HAL_OK) {
            eesta = 1; // Greška pri upisu,nevalja eeprom, neki log ili retry mehanizam
            return 1;
        }
        HAL_Delay(10); // EEPROM često zahteva kratku pauzu nakon pisanja
    }
    return 0;
}
/**
  * @brief  kopiraj timout-e iz memorisanih u eeprom-u
  * @param
  * @retval 0 = uspješno / 1 = neuspješno, ako je neuspješno i eesta flag se setuje 
  */
static uint8_t ee_load(void)
{
    uint8_t temp_value;

    for (uint8_t i = 0; i < JALOUSIE_COUNT; i++) {
        if (HAL_I2C_Mem_Read(&hi2c1, EEPROM_I2C_ADDR, EEPROM_START_ADDR + i, I2C_MEMADD_SIZE_8BIT, &temp_value, 1, EEPROM_TIMEOUT) == HAL_OK) {
            jalousie_timeout[i] = temp_value * 1000; // Konverzija u milisekunde
        } else {
            jalousie_timeout[i] = 0; // Ako čitanje ne uspije, postavi na 0
            eesta = 1; // nevalja eeprom
            return 1;
        }
    }
    return 0;
}
/**
  * @brief  periodična provjera eeproma zbog odgovora u tinyframe upitu
  * @param
  * @retval nema povratnog člana ali ostavlja eesta flag prema stanju eeproma
  */
static void ee_check(void)
{
    static uint32_t last_eeprom_check = 0;

    if(ee_save_flag ==  true)
    {
        ee_save();
        ee_save_flag = false;
    }
    // Na svakih 10 sekundi provjeri stanje eeproma
    if (HAL_GetTick() - last_eeprom_check > 10000) {
        ee_load();
        last_eeprom_check = HAL_GetTick();
    }
    // za slučaj neispravnog eeproma koristi defaultne vrijednosti timeouta
    if(eesta)
    {
        for (int i = 0; i < JALOUSIE_COUNT; i++)
        {
            jalousie_timeout[i] = 255 * 1000;
        }
    }
}
/**
  * @brief  podesi rs485 bus adresu uređaja i startnu i zadnju adresu žaluzine
  * @param
  * @retval nema
  */
void get_address(void)
{
    my_address = 0;
    if ((HAL_GPIO_ReadPin(ADDRESS0_GPIO_Port, ADDRESS0_Pin) == GPIO_PIN_RESET))   my_address |= 0x01;
    if ((HAL_GPIO_ReadPin(ADDRESS1_GPIO_Port, ADDRESS1_Pin) == GPIO_PIN_RESET))   my_address |= 0x02;
    if ((HAL_GPIO_ReadPin(ADDRESS2_GPIO_Port, ADDRESS2_Pin) == GPIO_PIN_RESET))   my_address |= 0x04;
    if ((HAL_GPIO_ReadPin(ADDRESS3_GPIO_Port, ADDRESS3_Pin) == GPIO_PIN_RESET))   my_address |= 0x08;
    if ((HAL_GPIO_ReadPin(ADDRESS4_GPIO_Port, ADDRESS4_Pin) == GPIO_PIN_RESET))   my_address |= 0x10;
    if(my_address)
    {
        jalousie_start = (NUMBER_OF_JALOUSIE*(my_address-1))+1;
        jalousie_end = jalousie_start + NUMBER_OF_JALOUSIE - 1;
    }
}
/**
  * @brief LED efekt za aktivan terminal meni
  * @param
  * @retval nema
  */
void update_snake(void)
{
    if (++snake_counter < SNAKE_SPEED) {
        return;  // Cekamo dok ne prode dovoljno ciklusa
    }
    snake_counter = 0;  // Reset brojaca

    // Ocisti prethodno stanje LED-ova
    for (uint8_t i = 0; i < 16; i++) {
        led_state[i] = 0;
    }

    // Podesi svetlece LED-ove
    for (uint8_t i = 0; i < SNAKE_LENGTH; i++) {
        int8_t pos = snake_head - i;
        if (pos < 0) pos += 16;  // Omogucava kru�no kretanje po matrici
        led_state[pos] = 1;  // Upali LED
    }

    // Pomeraj zmiju napred
    snake_head = (snake_head + 1) % 16;
}
/**
  * @brief LED efekt za start modula
  * @param
  * @retval nema
  */
void start_effect(void) {
    const uint8_t order[][2] = {
        {3, 4}, {2, 5}, {1, 6}, {0, 7}, // Paljenje (-1 poravnanje)
        {3, 4}, {2, 5}, {1, 6}, {0, 7}  // Gašenje (-1 poravnanje)
    };

    HAL_Delay(300);  // Početno kašnjenje

    // Sekvenca paljenja i gašenja LED-ova
    for (int i = 0; i < 8; i++) {
        // Uzimanje stanja iz led_state[] za svaki LED
        led_state[order[i][0]] = (i < 4) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        led_state[order[i][1]] = (i < 4) ? GPIO_PIN_SET : GPIO_PIN_RESET;

        // Ovdje se stanje direktno postavlja u led_state[], bez GPIO poziva
        // Logika LED paljenja/gašenja prema led_state

        HAL_Delay(50);  // Čekanje za tajming efekta

#ifdef USE_WATCHDOG
        HAL_IWDG_Refresh(&hiwdg);  // Osvežavanje watchdog-a
#endif
    }
}

/**
  * @brief Logika žaluzina i tajmera sa timout-ima
  * @param
  * @retval nema
  */
void jalousie_control(void)
{
    uint32_t elapsed_ms = HAL_GetTick(); // Trenutni sistemski brojač (ms)

    for (uint8_t i = 0; i < JALOUSIE_COUNT; i++) {
        if (jalousie[i] == 1) {
            // Aktiviraj UP, deaktiviraj DOWN
            HAL_GPIO_WritePin(jalousie_ports_down[i], jalousie_pins_down[i], GPIO_PIN_RESET);
            HAL_Delay(20); // Pauza da se prethodni triak potpuno ugasi
            HAL_GPIO_WritePin(jalousie_ports_up[i], jalousie_pins_up[i], GPIO_PIN_SET);
            
            if (jalousie_tmr[i] == 0) { // Pokreni timer ako nije već pokrenut
                jalousie_tmr[i] = elapsed_ms;
            }
        }
        else if (jalousie[i] == 2) {
            // Aktiviraj DOWN, deaktiviraj UP
            HAL_GPIO_WritePin(jalousie_ports_up[i], jalousie_pins_up[i], GPIO_PIN_RESET);
            HAL_Delay(20); // Pauza da se prethodni triak potpuno ugasi
            HAL_GPIO_WritePin(jalousie_ports_down[i], jalousie_pins_down[i], GPIO_PIN_SET);

            if (jalousie_tmr[i] == 0) { // Pokreni timer ako nije već pokrenut
                jalousie_tmr[i] = elapsed_ms;
            }
        }
        else {
            // Isključi oba izlaza ne treba nikakva pauza
            HAL_GPIO_WritePin(jalousie_ports_up[i], jalousie_pins_up[i], GPIO_PIN_RESET);
            HAL_GPIO_WritePin(jalousie_ports_down[i], jalousie_pins_down[i], GPIO_PIN_RESET);
            jalousie_tmr[i] = 0; // Resetuj timer
        }

        // Provjeri timeout
        if (jalousie_tmr[i] && jalousie_timeout[i] && (elapsed_ms - jalousie_tmr[i] >= jalousie_timeout[i])) {
            jalousie[i] = 0; // Isključi žaluzinu
            jalousie_tmr[i] = 0; // Resetuj timer
        }
        if(jalousie_timeout[i] == 0) jalousie_tmr[i] = 0; // Resetuj timer ako je timeout setovan za on/off kontrolu
    }
}
/**
  * @brief Kopiraj izlaze triaka na LED-ove prednjeg panela
  * @param
  * @retval nema
  */
void refresh_led(void)
{
    // Samo kopiraj izlaze na LED
    for (uint8_t i = 0; i < NUMBER_OF_JALOUSIE; i++)
    {
        led_state[i] = jalousie[i];
    }
}
/**
  * @brief Funkcija za osvježavanje LED matrice svakih ~5ms
  * @param
  * @retval nema
  */
void refresh_led_matrix(void)
{
    // Isključi trenutno aktivni red
    HAL_GPIO_WritePin(LED_R1_GPIO_Port, LED_R1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_R2_GPIO_Port, LED_R2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_R3_GPIO_Port, LED_R3_Pin, GPIO_PIN_RESET);

    // Isključi sve kolone prije uključivanja reda
    HAL_GPIO_WritePin(LED_C1_GPIO_Port, LED_C1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_C2_GPIO_Port, LED_C2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_C3_GPIO_Port, LED_C3_Pin, GPIO_PIN_SET);

    // Aktiviraj sledeći red
    switch (active_row) {
    case 0:
        HAL_GPIO_WritePin(LED_R1_GPIO_Port, LED_R1_Pin, GPIO_PIN_SET);
        break;
    case 1:
        HAL_GPIO_WritePin(LED_R2_GPIO_Port, LED_R2_Pin, GPIO_PIN_SET);
        break;
    case 2:
        HAL_GPIO_WritePin(LED_R3_GPIO_Port, LED_R3_Pin, GPIO_PIN_SET);
        break;
    }

    // Postavi kolone na osnovu led_state[]
    for (uint8_t col = 0; col < LED_COLS; col++) {
        uint8_t index = active_row * LED_COLS + col;
        if (led_state[index]) {
            switch (col) {
            case 0:
                HAL_GPIO_WritePin(LED_C1_GPIO_Port, LED_C1_Pin, GPIO_PIN_RESET);
                break;
            case 1:
                HAL_GPIO_WritePin(LED_C2_GPIO_Port, LED_C2_Pin, GPIO_PIN_RESET);
                break;
            case 2:
                HAL_GPIO_WritePin(LED_C3_GPIO_Port, LED_C3_Pin, GPIO_PIN_RESET);
                break;
            }
        }
    }

    // Prebaci na sledeći red
    active_row = (active_row + 1) % LED_ROWS;
}
/**
  * @brief Funkcija za procesiranje tastera prednjeg panela
  * @param
  * @retval nema
  */
void handle_buttons(void)
{
    uint32_t current_time = HAL_GetTick();
    uint8_t button_select = HAL_GPIO_ReadPin(BTN_MOVE_GPIO_Port, BTN_MOVE_Pin);
    uint8_t button_up = HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin);
    uint8_t button_down = HAL_GPIO_ReadPin(BTN_DN_GPIO_Port, BTN_DN_Pin);

    if (button_select == GPIO_PIN_RESET) {
        if (!button_select_prev) {
            button_select_prev = 1; // lock press time load
            menu_activ = 1;
            toggle_state = 0;
            first_toggle = 1;  // Resetuj first_toggle ako se resetuje meni
            if(++selected_channel >= NUMBER_OF_JALOUSIE) selected_channel = 0;
            menu_timer = current_time + MENU_TIMEOUT;
            refresh_led();
            get_address();
            HAL_Delay(BUTTON_PAUSE);
        }
    }
    else if ((button_select == GPIO_PIN_SET) && button_select_prev)
    {
        button_select_prev = 0;
        HAL_Delay(BUTTON_PAUSE);
        refresh_led();
    }

    if (menu_activ && (button_up == GPIO_PIN_RESET)) {
        if (!button_up_prev) {
            button_up_prev = 1; // lock press time load
            jalousie[selected_channel] = (jalousie[selected_channel] == 0) ? 1 : 0;
            toggle_state = jalousie[selected_channel]; // Ažuriraj toggle_state
            HAL_Delay(BUTTON_PAUSE);
        }
        menu_timer = current_time + MENU_TIMEOUT;
    }
    else if ((button_up == GPIO_PIN_SET) && button_up_prev)
    {
        button_up_prev = 0; // release button press
        HAL_Delay(BUTTON_PAUSE);
    }

    if (menu_activ && (button_down == GPIO_PIN_RESET)) {
        if (!button_down_prev) {
            button_down_prev = 1; // lock press time load
            jalousie[selected_channel] = (jalousie[selected_channel] == 0) ? 2 : 0;
            toggle_state = jalousie[selected_channel]; // Ažuriraj toggle_state
            HAL_Delay(BUTTON_PAUSE);
        }
        menu_timer = current_time + MENU_TIMEOUT;
    }
    else if ((button_down == GPIO_PIN_SET) && button_down_prev)
    {
        button_down_prev = 0; // release button press
        HAL_Delay(BUTTON_PAUSE);
    }
}
/**
  * @brief  Jednostavan meni za podešavanje stanja izlaza sa razlicitim odnosom pauze i signala
  *         odabrane LED prema stanju izlaza kraci signal duža pauza za iskljucen izlaz
  * @retval None
  */
void menu_logic(void)
{
    uint32_t current_time = HAL_GetTick();

    // Resetujemo meni ako istekne vrijeme
    if (current_time > menu_timer) {
        selected_channel = NUMBER_OF_JALOUSIE - 1;
        menu_activ = 0;
        toggle_state = 0;
        first_toggle = 1;  // Resetuj first_toggle ako se resetuje meni
        refresh_led();
    }

    // Definiši vremenske intervale zavisno od jalousie_states
    uint32_t on_time, off_time;
    if (jalousie[selected_channel] == 1) {
        on_time = TOGGLE_INTERVAL * 10;  // LED ON duže
        off_time = TOGGLE_INTERVAL;     // LED OFF kraće
    } else if (jalousie[selected_channel] == 2) {
        on_time = TOGGLE_INTERVAL;      // LED ON duže
        off_time = TOGGLE_INTERVAL * 10; // LED OFF kraće
    }  else {
        on_time = TOGGLE_INTERVAL;      // LED ON jednako
        off_time = TOGGLE_INTERVAL; // LED OFF jednako
    }

    // Pri prvom ulasku postavi početno stanje
    if (first_toggle) {
        toggle_state = jalousie[selected_channel]; // Ako je triac_state 1, LED počinje upaljena
        last_toggle_time = current_time;  // Resetujemo timer
        first_toggle = 0; // Obilježimo da više nije prvi ulazak
    }

    // Odredi trenutno trajanje na osnovu toggle_state
    uint32_t toggle_period = toggle_state ? on_time : off_time;

    // Ako je prošao period, promjeni stanje
    if (current_time - last_toggle_time >= toggle_period) {
        toggle_state = !toggle_state;
        led_state[selected_channel] = toggle_state;  // Direktno ažuriranje LED stanja
        last_toggle_time = current_time;
    }

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC_Init();
    MX_I2C1_Init();
    MX_IWDG_Init();
    MX_USART1_UART_Init();
    MX_TIM14_Init();
    /* USER CODE BEGIN 2 */
    RS485_Init();
    eesta = 0; // Reset statusa EEPROM-a
    ee_load(); // Prvo učitavanje
    get_address();
    start_effect();
    start_effect();
    start_effect();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        if(reset == true) Error_Handler(); // reset je iskan učini tako
        handle_buttons();
        menu_logic();
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        jalousie_control();
        ee_check(); // refrešuj eeprom
        
#ifdef USE_WATCHDOG
        HAL_IWDG_Refresh(&hiwdg);
#endif
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14
                                       |RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.HSI14CalibrationValue = 16;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
    RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

    /* USER CODE BEGIN ADC_Init 0 */

    /* USER CODE END ADC_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC_Init 1 */

    /* USER CODE END ADC_Init 1 */

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.LowPowerAutoWait = DISABLE;
    hadc.Init.LowPowerAutoPowerOff = DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(&hadc) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure for the selected ADC regular channel to be converted.
    */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC_Init 2 */

    /* USER CODE END ADC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

    /* USER CODE BEGIN I2C1_Init 0 */

    /* USER CODE END I2C1_Init 0 */

    /* USER CODE BEGIN I2C1_Init 1 */

    /* USER CODE END I2C1_Init 1 */
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x0010020A;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C1_Init 2 */

    /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

    /* USER CODE BEGIN IWDG_Init 0 */
#ifdef USE_WATCHDOG
    /* USER CODE END IWDG_Init 0 */

    /* USER CODE BEGIN IWDG_Init 1 */

    /* USER CODE END IWDG_Init 1 */
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
    hiwdg.Init.Window = 4095;
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN IWDG_Init 2 */
#endif
    /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

    /* USER CODE BEGIN TIM14_Init 0 */

    /* USER CODE END TIM14_Init 0 */

    /* USER CODE BEGIN TIM14_Init 1 */

    /* USER CODE END TIM14_Init 1 */
    htim14.Instance = TIM14;
    htim14.Init.Prescaler = 48000 - 1;
    htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim14.Init.Period = 5 - 1;
    htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM14_Init 2 */
    HAL_TIM_Base_Start_IT(&htim14);
    /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_RS485Ex_Init(&huart1, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART1_Init 2 */

    /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* USER CODE BEGIN MX_GPIO_Init_1 */
    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, LED_R1_Pin|JAL1_DN_Pin|JAL1_UP_Pin|JAL0_DN_Pin
                      |JAL3_UP_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, JAL7_DN_Pin|JAL7_UP_Pin|JAL6_DN_Pin|JAL6_UP_Pin
                      |JAL5_DN_Pin|JAL5_UP_Pin|JAL4_DN_Pin|JAL4_UP_Pin
                      |RELAY_Pin|JAL2_DN_Pin|JAL2_UP_Pin|LED_C3_Pin
                      |LED_C2_Pin|LED_C1_Pin|LED_R3_Pin|LED_R2_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOF, JAL0_UP_Pin|JAL3_DN_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : ADDRESS0_Pin ADDRESS1_Pin ADDRESS2_Pin */
    GPIO_InitStruct.Pin = ADDRESS0_Pin|ADDRESS1_Pin|ADDRESS2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : ADDRESS3_Pin ADDRESS4_Pin */
    GPIO_InitStruct.Pin = ADDRESS3_Pin|ADDRESS4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /*Configure GPIO pins : LED_R1_Pin JAL1_DN_Pin JAL1_UP_Pin JAL0_DN_Pin
                             JAL3_UP_Pin */
    GPIO_InitStruct.Pin = LED_R1_Pin|JAL1_DN_Pin|JAL1_UP_Pin|JAL0_DN_Pin
                          |JAL3_UP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : BTN_DN_Pin BTN_UP_Pin BTN_MOVE_Pin */
    GPIO_InitStruct.Pin = BTN_DN_Pin|BTN_UP_Pin|BTN_MOVE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : JAL7_DN_Pin JAL7_UP_Pin JAL6_DN_Pin JAL6_UP_Pin
                             JAL5_DN_Pin JAL5_UP_Pin JAL4_DN_Pin JAL4_UP_Pin
                             RELAY_Pin JAL2_DN_Pin JAL2_UP_Pin LED_C3_Pin
                             LED_C2_Pin LED_C1_Pin LED_R3_Pin LED_R2_Pin */
    GPIO_InitStruct.Pin = JAL7_DN_Pin|JAL7_UP_Pin|JAL6_DN_Pin|JAL6_UP_Pin
                          |JAL5_DN_Pin|JAL5_UP_Pin|JAL4_DN_Pin|JAL4_UP_Pin
                          |RELAY_Pin|JAL2_DN_Pin|JAL2_UP_Pin|LED_C3_Pin
                          |LED_C2_Pin|LED_C1_Pin|LED_R3_Pin|LED_R2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : JAL0_UP_Pin JAL3_DN_Pin */
    GPIO_InitStruct.Pin = JAL0_UP_Pin|JAL3_DN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* USER CODE BEGIN MX_GPIO_Init_2 */
    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// Timer interrupt callback
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM14) {
//        if(start_effect == true) update_snake();  // Ažuriraj stanje zmije ako je mod podešavanja aktivan
        refresh_led_matrix();
    }
}
/**
  * @brief  podesi timout žaluzine trajno
  * @param
  * @retval
  */
TF_Result JALOUSIE_SETUP_Listener(TinyFrame *tf, TF_Msg *msg)
{
    uint16_t adr = (uint16_t)(msg->data[0] << 8) | msg->data[1];  // Kombinacija dva bajta u adresu
    uint8_t idx = adr - jalousie_start;  // Izračunavanje indeksa žaluzine
    uint8_t ret[4]; // Niz za vracanje stanja

    // Proveravamo da li je adresa unutar opsega jalousie_start i jalousie_end
    if (adr && my_address && (adr >= jalousie_start) && (adr <= jalousie_end)) {
        if(eesta == 0) { // provjeri stanje eeproma 
            jalousie_timeout[idx] = (uint32_t)msg->data[2] * 1000; // podesi timeout u milisekundama
            ee_save_flag = true; // setuj flag za upis eeproma
        } else jalousie_timeout[idx] = 255 * 1000; // defaultna vrijednost ako eeprom ima problem
        ret[0] = msg->data[0];
        ret[1] = msg->data[1];
        ret[2] = jalousie_timeout[idx] / 1000;  // vrati timeout u sekundama
        ret[3] = (eesta == 0) ? ACK : NAK; // odgovor ovisi od zadnje provjere stanja eeproma
        msg->data = ret;  // Postavljamo novi niz za odgovor
        msg->len = 4;  // Dužina odgovora je 4 bajta
        TF_Respond(tf, msg);  // Odgovaramo sa novim stanjem
    }
    return TF_STAY;  // Održavanje trenutnog stanja
}
/**
  * @brief Podesi novo stanje žaluzine
  * @param
  * @retval stanje žaluzine, timeout žaluzine, ACK
  */
TF_Result JALOUSIE_SET_Listener(TinyFrame *tf, TF_Msg *msg)
{
    uint16_t adr = (uint16_t)(msg->data[0] << 8) | msg->data[1];  // Kombinacija dva bajta u adresu
    uint8_t idx = adr - jalousie_start;  // Izračunavanje indeksa žaluzine
    uint8_t ret[4]; // Niz za vracanje stanja

    // Proveravamo da li je adresa unutar opsega rel_start i rel_end
    if (adr && my_address && (adr >= jalousie_start) && (adr <= jalousie_end)) {
        jalousie[idx] = (msg->data[2] < 3) ? msg->data[2] : 0;  // Novo stanje izlaza žaluzine
        refresh_led(); // pokaži i na front panelu
        ret[0] = msg->data[0];
        ret[1] = msg->data[1];
        ret[2] = jalousie[idx]; // Vracamo stanje žaluzine na toj adresi
        ret[3] = (msg->data[2] < 3) ? ACK : NAK; // Reaguj na pogrešan podatak
        msg->data = ret;  // Postavljamo novi niz za odgovor
        msg->len = 4;  // Dužina odgovora je 4 bajta
        TF_Respond(tf, msg);  // Odgovaramo sa novim stanjem
    }

    return TF_STAY;  // Održavanje trenutnog stanja
}
/**
  * @brief  Odgovori na zahtjev za stanje žaluzine
  * @param
  * @retval
  */
TF_Result JALOUSIE_GET_Listener(TinyFrame *tf, TF_Msg *msg)
{
    uint16_t adr = (uint16_t)(msg->data[0] << 8) | msg->data[1];  // Kombinacija dva bajta u adresu
    uint8_t idx = adr - jalousie_start;  // Izračunavanje indeksa žaluzine
    uint8_t ret[4]; // Niz za vracanje stanja

    // Provjeravamo da li je adresa unutar opsega jalousie_start i jalousie_end
    if (adr && my_address && (adr >= jalousie_start) && (adr <= jalousie_end)) {
        ret[0] = msg->data[0];
        ret[1] = msg->data[1];
        ret[2] = jalousie[idx]; // Vracamo stanje žaluzine na toj adresi
        ret[3] = jalousie_timeout[idx] / 1000;  // vrati timeout u sekundama
        msg->data = ret;  // Postavljamo novi niz za odgovor
        msg->len = 4;  // Dužina odgovora je 4 bajta
        TF_Respond(tf, msg);  // Odgovaramo sa novim stanjem
    }

    return TF_STAY;  // Održavanje trenutnog stanja
}
/**
  * @brief  // softverski restart modula žaluzina čiji je adresirani izlaza žaluzine 
  * @param
  * @retval
  */
TF_Result JALOUSIE_RESET_Listener(TinyFrame *tf, TF_Msg *msg)
{
    uint16_t adr = (uint16_t)(msg->data[0] << 8) | msg->data[1];  // Kombinacija dva bajta u adresu
    uint8_t idx = adr - jalousie_start;  // Izračunavanje indeksa žaluzine
    uint8_t ret[3]; // Niz za vracanje stanja

    // Proveravamo da li je adresa unutar opsega jalousie_start i jalousie_end
    if (adr && my_address && (adr >= jalousie_start) && (adr <= jalousie_end)) {
        reset = true;  // koristimo flag za reset umjesto da odmah ovdje restartujemo modul bez odgovora na komandu
        ret[0] = msg->data[0];
        ret[1] = msg->data[1];
        ret[2] = ACK; // odgovor ovisi od zadnje provjere stanja eeproma
        msg->data = ret;  // Postavljamo novi niz za odgovor
        msg->len = 3;  // Dužina odgovora je 3 bajta
        TF_Respond(tf, msg);  // Odgovaramo sa novim stanjem
    }
    return TF_STAY;  // Održavanje trenutnog stanja
}
/**
  * @brief
  * @param
  * @retval
  */
void RS485_Init(void)
{
    if(!init_tf) {
        init_tf = TF_InitStatic(&tfapp, TF_SLAVE);
        TF_AddTypeListener(&tfapp, JALOUSIE_GET, JALOUSIE_GET_Listener);
        TF_AddTypeListener(&tfapp, JALOUSIE_SET, JALOUSIE_SET_Listener);
        TF_AddTypeListener(&tfapp, JALOUSIE_SETUP, JALOUSIE_SETUP_Listener);
        TF_AddTypeListener(&tfapp, JALOUSIE_RESET, JALOUSIE_RESET_Listener);
    }
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief
  * @param
  * @retval
  */
void RS485_Tick(void)
{
    if (init_tf == true) {
        TF_Tick(&tfapp);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    HAL_UART_Transmit(&huart1,(uint8_t*)buff, len, RESP_TOUT);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    TF_AcceptChar(&tfapp, rec);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    __HAL_UART_CLEAR_PEFLAG(&huart1);
    __HAL_UART_CLEAR_FEFLAG(&huart1);
    __HAL_UART_CLEAR_NEFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    HAL_UART_AbortReceive(&huart1);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    HAL_ADC_MspDeInit(&hadc);
    HAL_I2C_MspDeInit(&hi2c1);
    HAL_TIM_Base_MspDeInit(&htim14);
    HAL_UART_MspDeInit(&huart1);
    while (1)
    {
        HAL_NVIC_SystemReset();
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
