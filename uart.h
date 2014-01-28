#ifndef _UART_H
#define _UART_H

#define UART_BUFFER_SIZE 100

void setup_uarts(void);
int8_t uart_putchar(int8_t c);
int8_t uart_getchar(void);
#endif