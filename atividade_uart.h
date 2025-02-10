#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "inc/matrix_helpers.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "hardware/pio.h"
#include "hardware/adc.h"
#include "hardware/uart.h"

#include "pio_matrix.pio.h"

#ifndef _ATIVIDADE_UART_H
#define _ATIVIDADE_UART_H

//-------------------------------------------------------------------------

const uint BTN_A_PIN = 5;
const uint BTN_B_PIN = 6;

const uint LED_MATRIX_PIN = 7;
const uint LED_MATRIX_NUM_PINS = 25;

const uint LED_GREEN_PIN = 11;
const uint LED_BLUE_PIN = 12;

//-------------------------------------------------------------------------

static i2c_inst_t* const I2C_PORT = i2c1; // Porta I2C
static const uint I2C_SDA  = 14; // Pinos I2C SDA
static const uint I2C_SCL  = 15; // Pinos I2C SCL
static const uint I2C_SPEED = 400 * 1000;   // Operando à 400 kHz
static const uint OLED_ADDR = 0x3C; // Endereço do display OLED

static ssd1306_t ssd;  // Controller do display OLED
const char *msg_headline = "ATIVIDADE-U4C6"; // Mensagem de cabeçalho
const char *msg_body = "Press. BTN A/B"; // Mensagem de corpo
const char *msg_footer = "P/ continuar"; // Mensagem de rodapé

static uart_inst_t* const UART_ID = uart1;
static const uint BAUD_RATE = 115200;

static const uint UART_TX_PIN = 4; // Pino TX
static const uint UART_RX_PIN = 5; // Pino RX



//-------------------------------------------------------------------------

void initialize_gpio_pins();
void gpio_irq_handler_cb(uint gpio, uint32_t events);
uint32_t matrix_rgb(double b, double r, double g);
void display_digit(int digit, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b);
void init_i2c();
void init_display(ssd1306_t *ssd);
void update_display(ssd1306_t *ssd, bool *color_flag);
void init_uart();

//-------------------------------------------------------------------------

volatile uint digit = 0;
volatile bool digit_changed = false;
volatile absolute_time_t last_interrupt_time = {0};

//-------------------------------------------------------------------------

#endif // _MAIN_H
