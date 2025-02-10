#include "atividade_uart.h"

int main()
{
    stdio_init_all();

    PIO pio = pio0;
    bool ok;
    uint16_t i;
    uint32_t valor_led;
    double r = 0.01, b = 0.0, g = 0.0;

    ok = set_sys_clock_khz(128000, false);

    // Inicializa os pinos GPIO
    initialize_gpio_pins();

    // Inicializa I2C e OLED display
    init_i2c();
    init_display(&ssd);

    // Inicializa UART
    init_uart();

    bool color_flag = true;

    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, LED_MATRIX_PIN);

    display_digit(digit, valor_led, pio, sm, r, g, b);



    while (true)
    {
        
        int c = getchar_timeout_us(0); 
        
        if (c != PICO_ERROR_TIMEOUT)
        {
            // Display character on SSD1306
            msg_body = "Retorno " + c;

            // If character is a number between 0 and 9, display on WS2812
            if (c == '0'){
                digit = 0;
                digit_changed = true;
            } else if (c == '1'){
                digit = 1;
                digit_changed = true;
            } else if (c == '2'){
                digit = 2;
                digit_changed = true;
            } else if (c == '3'){
                digit = 3;
                digit_changed = true;
            } else if (c == '4'){
                digit = 4;
                digit_changed = true;
            } else if (c == '5'){
                digit = 5;
                digit_changed = true;
            } else if (c == '6'){
                digit = 6;
                digit_changed = true;
            } else if (c == '7'){
                digit = 7;
                digit_changed = true;
            } else if (c == '8'){
                digit = 8;
                digit_changed = true;
            } else if (c == '9'){
                digit = 9;
                digit_changed = true;
            }
        }

        if (digit_changed)
        {
            display_digit(digit, valor_led, pio, sm, r, g, b);
            digit_changed = false;
        }

        update_display(&ssd, &color_flag);
        sleep_ms(1000);
    }

    return 0;
}

//-------------------------------------------------------------------------

void initialize_gpio_pins()
{
    gpio_init(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_init(LED_MATRIX_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);

    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);

    gpio_set_dir(LED_MATRIX_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);

    gpio_pull_up(BTN_A_PIN);
    gpio_pull_up(BTN_B_PIN);

    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler_cb);
    gpio_set_irq_enabled_with_callback(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler_cb);
}

void gpio_irq_handler_cb(uint gpio, uint32_t events)
{
    absolute_time_t now =  to_us_since_boot(get_absolute_time());;
    if (absolute_time_diff_us(last_interrupt_time, now) < 200000)
    {
        return;
    }

    last_interrupt_time = now;

    // Pressionar o botão A deve alternar o estado do LED RGB Verde (ligado/desligado)
    if (gpio == BTN_A_PIN)
    {
        bool green_led_new_state = !green_led_last_state;

        gpio_put(LED_BLUE_PIN, 0);
        gpio_put(LED_GREEN_PIN, green_led_new_state);
        
        // Uma mensagem informativa sobre o estado do LED deve ser exibida no display SSD1306
        msg_body = green_led_new_state ? "LED VERDE ON" : "LED VERDE OFF";

        printf("%s\n", msg_body);

        green_led_last_state = green_led_new_state;

    }
    // Pressionar o botão B deve alternar o estado do LED RGB Azul (ligado/desligado).
    else if (gpio == BTN_B_PIN)
    {
        bool blue_led_new_state = !blue_led_last_state;
        
        gpio_put(LED_GREEN_PIN, 0);
        gpio_put(LED_BLUE_PIN, blue_led_new_state);

        // Uma mensagem informativa sobre o estado do LED deve ser exibida no display SSD1306
        msg_body = blue_led_new_state ? "LED AZUL ON" : "LED AZUL OFF";
        
        printf("%s\n", msg_body);

        blue_led_last_state = blue_led_new_state;
    }
}


uint32_t matrix_rgb(double b, double r, double g)
{
    uint8_t R = (uint8_t)(r * 255);
    uint8_t G = (uint8_t)(g * 255);
    uint8_t B = (uint8_t)(b * 255);
    return (G << 24) | (R << 16) | (B << 8);
}

void display_digit(int digit, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b)
{
    double *digitsMap[] = {digit0, digit1, digit2, digit3, digit4, digit5, digit6, digit7, digit8, digit9};
    double *desenho = digitsMap[digit];

    for (int i = 0; i < LED_MATRIX_NUM_PINS; i++)
    {
        int n = NORMALIZED_ARRAY_LOCATION[i];
        valor_led = matrix_rgb(0, desenho[n] * r, 0);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// I2C relacionado
void init_i2c()
{
    i2c_init(I2C_PORT, I2C_SPEED);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

// Inicializa o OLED display com as configurações padrão
void init_display(ssd1306_t *ssd)
{
    ssd1306_init(ssd, WIDTH, HEIGHT, false, OLED_ADDR, I2C_PORT);
    ssd1306_config(ssd);
    ssd1306_send_data(ssd);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

// Atualiza o display com um retângulo e textos
void update_display(ssd1306_t *ssd, bool *color_flag)
{
    *color_flag = !(*color_flag);
    ssd1306_fill(ssd, !(*color_flag));
    ssd1306_rect(ssd, 3, 3, 122, 58, *color_flag, !(*color_flag));
    ssd1306_draw_string(ssd, msg_headline, 8, 10);
    ssd1306_draw_string(ssd, msg_body, 8, 30);
    ssd1306_draw_string(ssd, msg_footer, 8, 48);
    ssd1306_send_data(ssd);
}

void init_uart()
{
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

void uart_rx_handler(int c) {
    if (c) {
        printf("Foi retornado %c", c);
        msg_body = "Recebido: " + c;
    }

    // If character is a number between 0 and 9, display on WS2812
    if (c >= '0' && c <= '9') {
        digit = c - '0';
        digit_changed = true;
    }
}



