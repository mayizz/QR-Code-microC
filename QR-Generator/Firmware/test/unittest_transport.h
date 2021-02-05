#ifndef UNITTEST_TRANSPORT_H
#define UNITTEST_TRANSPORT_H

#include <avr/io.h>
#include <stdint.h>
#include "uart.h"

void unittest_uart_begin()
{
    // Set Baudrate according to datasheet (16MHz -> 9600 Baud -> 103)
    UBRR0 = 103;
    UCSR0B |= (1<<TXEN0);

    // Reset Complete-Flags
    UCSR0A = (1 << RXC0) | (1 << TXC0);
}

void unittest_uart_putchar(char c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

void unittest_uart_flush()
{
}

void unittest_uart_end()
{
}

#endif