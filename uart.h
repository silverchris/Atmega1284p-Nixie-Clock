#ifndef _UART_H
#define _UART_H

#define UART_BUFFER_SIZE 200

void setup_uarts(void);
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);
#endif