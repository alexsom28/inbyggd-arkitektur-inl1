#include "app.h"
#include "config.h"
#include "gpio.h"
#include "pins.h"
#include "uart.h"
#include "millis.h"
#include "keypad.h"
#include <string.h>
#include <avr/eeprom.h>

#define PIN_LENGTH        4
#define INPUT_TIMEOUT_MS  5000
#define ACCESS_DURATION   3000
#define BLINK_INTERVAL    250
#define FLASH_INTERVAL    80
#define CMD_BUFFER_SIZE   48
#define EEPROM_MAGIC      0xAB

static uint8_t  EEMEM ee_magic;
static char     EEMEM ee_pin[PIN_LENGTH + 1];

typedef enum
{
    STATE_IDLE,
    STATE_INPUT_AWAIT,
    STATE_ACCESS_GRANTED
} state_t;

static state_t  current_state;
static char     pin_code[PIN_LENGTH + 1] = "1772";
static char     input_buffer[PIN_LENGTH + 1];
static uint8_t  input_index;
static millis_t state_enter_time;
static millis_t last_blink_time;
static bool     red_led_on_flag;
static char     cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t  cmd_index;

static void red_on(void)    { gpio_pin_high(&RED_LED_PORT, RED_LED_PIN);   red_led_on_flag = true;  }
static void red_off(void)   { gpio_pin_low(&RED_LED_PORT, RED_LED_PIN);    red_led_on_flag = false; }
static void green_on(void)  { gpio_pin_high(&GREEN_LED_PORT, GREEN_LED_PIN); }
static void green_off(void) { gpio_pin_low(&GREEN_LED_PORT, GREEN_LED_PIN);  }

static bool button_pressed(void)
{
    return !(BTN_INPUT & (1U << BTN_PIN));
}

static void enter_state(state_t new_state)
{
    current_state    = new_state;
    state_enter_time = millis_get();
    last_blink_time  = state_enter_time;
    input_index      = 0;

    switch (new_state)
    {
        case STATE_IDLE:
            red_on();
            green_off();
            uart_write_string("State: IDLE\n");
            break;

        case STATE_INPUT_AWAIT:
            red_led_on_flag = true;
            red_on();
            green_off();
            uart_write_string("State: INPUT_AWAIT - enter PIN\n");
            break;

        case STATE_ACCESS_GRANTED:
            red_off();
            green_on();
            uart_write_string("State: ACCESS_GRANTED\n");
            break;
    }
}

static void flash_green(void)
{
    green_on();
    millis_t start = millis_get();
    while ((millis_get() - start) < FLASH_INTERVAL)
        ;
    green_off();
}

static void load_pin_from_eeprom(void)
{
    uint8_t magic = eeprom_read_byte(&ee_magic);
    if (magic == EEPROM_MAGIC)
    {
        eeprom_read_block(pin_code, ee_pin, PIN_LENGTH);
        pin_code[PIN_LENGTH] = '\0';
        uart_write_string("PIN loaded from EEPROM\n");
    }
    else
    {
        eeprom_update_byte(&ee_magic, EEPROM_MAGIC);
        eeprom_update_block("1772", ee_pin, PIN_LENGTH + 1);
        uart_write_string("Default PIN stored in EEPROM\n");
    }
}

static void save_pin_to_eeprom(void)
{
    eeprom_update_byte(&ee_magic, EEPROM_MAGIC);
    eeprom_update_block(pin_code, ee_pin, PIN_LENGTH + 1);
}

static void process_command(const char *cmd)
{
    if (current_state != STATE_IDLE)
    {
        uart_write_string("Command rejected - not in IDLE state\n");
        return;
    }

    if (strncmp(cmd, "NEW PIN;", 8) != 0)
    {
        uart_write_string("Unknown command. Use: NEW PIN;<old>;<new>\n");
        return;
    }

    const char *old_start = cmd + 8;
    const char *sep = strchr(old_start, ';');
    if (!sep)
    {
        uart_write_string("Bad format. Use: NEW PIN;<old>;<new>\n");
        return;
    }

    uint8_t old_len = (uint8_t)(sep - old_start);
    const char *new_start = sep + 1;
    uint8_t new_len = (uint8_t)strlen(new_start);

    if (old_len != PIN_LENGTH || new_len != PIN_LENGTH)
    {
        uart_write_string("PIN must be 4 digits\n");
        return;
    }

    if (strncmp(old_start, pin_code, PIN_LENGTH) != 0)
    {
        uart_write_string("Old PIN incorrect\n");
        return;
    }

    memcpy(pin_code, new_start, PIN_LENGTH);
    pin_code[PIN_LENGTH] = '\0';
    save_pin_to_eeprom();
    uart_write_string("PIN updated successfully!\n");
}

void app_init(void)
{
    gpio_pin_output(&RED_LED_DDR, RED_LED_PIN);
    gpio_pin_output(&GREEN_LED_DDR, GREEN_LED_PIN);
    gpio_pin_input_pullup(&BTN_DDR, &BTN_PORT, BTN_PIN);
    keypad_init();
    millis_init();
    uart_init(UART_BAUDRATE);
    load_pin_from_eeprom();
    uart_write_string("System ready\n");
    uart_write_string("Commands: NEW PIN;<old>;<new>\n");
    enter_state(STATE_IDLE);
}

void app_run(void)
{
    millis_t now = millis_get();

    {
        char c;
        while (uart_read_char(&c))
        {
            uart_write_char(c);

            if (c == '\r' || c == '\n')
            {
                uart_write_string("\n");
                if (cmd_index > 0)
                {
                    cmd_buffer[cmd_index] = '\0';
                    process_command(cmd_buffer);
                    cmd_index = 0;
                }
            }
            else if (cmd_index < (CMD_BUFFER_SIZE - 1))
            {
                cmd_buffer[cmd_index++] = c;
            }
            else
            {
                cmd_index = 0;
                uart_write_string("\nCommand too long\n");
            }
        }
    }

    switch (current_state)
    {
        case STATE_IDLE:
        {
            if (button_pressed())
            {
                millis_t db = millis_get();
                while ((millis_get() - db) < 50)
                    ;
                if (button_pressed())
                {
                    while (button_pressed())
                        ;
                    enter_state(STATE_INPUT_AWAIT);
                }
            }
            break;
        }

        case STATE_INPUT_AWAIT:
        {
            if ((now - last_blink_time) >= BLINK_INTERVAL)
            {
                last_blink_time = now;
                if (red_led_on_flag)
                    red_off();
                else
                    red_on();
            }

            if ((now - state_enter_time) >= INPUT_TIMEOUT_MS)
            {
                uart_write_string("Timeout!\n");
                enter_state(STATE_IDLE);
                break;
            }

            char key = keypad_scan();
            if (key != KEYPAD_NO_KEY)
            {
                flash_green();
                input_buffer[input_index++] = key;
                uart_write_char(key);

                if (input_index >= PIN_LENGTH)
                {
                    input_buffer[PIN_LENGTH] = '\0';
                    uart_write_string("\n");

                    if (strcmp(input_buffer, pin_code) == 0)
                    {
                        enter_state(STATE_ACCESS_GRANTED);
                    }
                    else
                    {
                        uart_write_string("Wrong PIN!\n");
                        enter_state(STATE_IDLE);
                    }
                }
            }
            break;
        }

        case STATE_ACCESS_GRANTED:
        {
            if ((now - state_enter_time) >= ACCESS_DURATION)
            {
                uart_write_string("Session ended\n");
                enter_state(STATE_IDLE);
            }
            break;
        }
    }
}
