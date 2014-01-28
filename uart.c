#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "uart.h"
#include "buffer.h"
#include "xbootapi.h"

CircularBuffer uart0_rx_buffer;
CircularBuffer uart0_tx_buffer;

static void init_uart0(void){
    PRR0 &= ~(1 << PRUSART0); 
    #define BAUD 115200
    #include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    #if USE_2X
    UCSR0A |= (1 << U2X0);
    #else
    UCSR0A &= ~(1 << U2X0);
    #endif
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(0<<UCSZ02)|(1<<RXCIE0);
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}

static void init_uart1(void){
    PRR0 &= ~(1 << PRUSART1); 
    #define BAUD 9600
    #include <util/setbaud.h>
    UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;
    #if USE_2X
    UCSR1A |= (1 << U2X1);
    #else
    UCSR1A &= ~(1 << U2X1);
    #endif
}

void uart0_tx(void){
    char tx;
    if(UCSR0A & (1<<UDRE0)){
        if(cbIsEmpty(&uart0_tx_buffer)){
            UCSR0B &= ~(1<<UDRIE0);
        }
        else{
            cbRead(&uart0_tx_buffer, &tx);
            UDR0 = tx;
        }
    }
}

ISR(USART0_RX_vect){
    char rx = UDR0;
    if (rx == 0x1B) {
        xboot_reset();
    }
    cbWrite(&uart0_rx_buffer, &rx);
}

ISR(USART0_UDRE_vect){
    uart0_tx();
}

void setup_uarts(void){
    cbInit(&uart0_rx_buffer, UART_BUFFER_SIZE);
    cbInit(&uart0_tx_buffer, UART_BUFFER_SIZE);    
    init_uart0();
}

int uart_putchar(char c, FILE *stream){
    if(c == '\n'){
        char r = '\r';
        cbWrite(&uart0_tx_buffer, &r);
    }
    cbWrite(&uart0_tx_buffer, &c);
    UCSR0B |= (1<<UDRIE0);
    return 0;
}

int uart_getchar(FILE *stream){
    char c = 0;
    if(!cbIsEmpty(&uart0_rx_buffer)){
        cbRead(&uart0_tx_buffer, &c);
    }
    return c;
}
