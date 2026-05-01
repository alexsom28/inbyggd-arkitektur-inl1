#include "uart.h"
#include "ring_buffer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static ring_buffer_t rx_buffer;

static uint16_t uart_baud_to_ubrr(uint32_t baudrate)
{
    return (uint16_t)((F_CPU / (16UL * baudrate)) - 1UL);
}

void uart_init(uint32_t baudrate)
{
    uint16_t ubrr = uart_baud_to_ubrr(baudrate);

    ring_buffer_init(&rx_buffer);

    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);

    UCSR0A = 0;
    UCSR0B = (1U << RXEN0) | (1U << TXEN0) | (1U << RXCIE0);
    UCSR0C = (1U << UCSZ01) | (1U << UCSZ00);
}

void uart_write_char(char c)
{
    while (!(UCSR0A & (1U << UDRE0)))
    {
    }

    UDR0 = c;
}

void uart_write_string(const char *str)
{
    while (*str)
    {
        if (*str == '\n')
        {
            uart_write_char('\r');
        }
        uart_write_char(*str++);
    }
}

bool uart_read_char(char *c)
{
    bool ok;

    cli();
    ok = ring_buffer_get(&rx_buffer, c);
    sei();

    return ok;
}

uint8_t uart_available(void)
{
    uint8_t count;

    cli();
    count = rx_buffer.count;
    sei();

    return count;
}

ISR(USART_RX_vect)
{
    char c = (char)UDR0;
    (void)ring_buffer_put(&rx_buffer, c);
}
