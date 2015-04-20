#ifndef _UART_H
#define _UART_H
#include "buffer.h"

#define UART_BUFFER_SIZE 250

CircularBuffer uart0_rx_buffer;

void setup_uarts(void);
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);
#endif