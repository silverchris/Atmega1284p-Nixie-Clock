#define UART_BUFFER_SIZE 32

void setup_uarts(void);
int8_t uart_putchar(int8_t c);
int8_t uart_getchar(void);