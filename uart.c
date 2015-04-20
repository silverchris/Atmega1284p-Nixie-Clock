#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "uart.h"
#include "buffer.h"
#include "xbootapi.h"
#include "ui.h"

#include <util/delay.h>


extern int ui_flag;
extern int gps_flag;

CircularBuffer uart0_rx_buffer;
CircularBuffer uart0_tx_buffer;

CircularBuffer uart1_rx_buffer;

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
    #define BAUD 4800
    #include <util/setbaud.h>
    UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;
    #if USE_2X
    UCSR1A |= (1 << U2X1);
    #else
    UCSR1A &= ~(1 << U2X1);
    #endif
    UCSR1B = (1<<RXEN1)|(0<<UCSZ12)|(1<<RXCIE1);
    UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);
}

ISR(USART0_RX_vect){
    char rx = UDR0;
    if (rx == 0x1B) {
        xboot_reset();
    }
    cbWrite(&uart0_rx_buffer, &rx);
    if(rx != 0x0d){
        printf("%c", rx);
    }
    if(rx == '\r'){
        ui_flag = 1;
    }
}

ISR(USART1_RX_vect){
    if(!cbIsFull(&uart1_rx_buffer)){
        char rx = UDR1;
        cbWrite(&uart1_rx_buffer, &rx);
        if(rx == '\n'){
            gps_flag = 1;
        }
    }
}

ISR(USART0_UDRE_vect){
    cbRead(&uart0_tx_buffer, (char *)&UDR0);
    if(cbIsEmpty(uart0_tx_buffer)){
        UCSR0B &= ~(1<<UDRIE0);
    }
}

void setup_uarts(void){
    cbInit(&uart0_rx_buffer, UART_BUFFER_SIZE);
    cbInit(&uart0_tx_buffer, UART_BUFFER_SIZE);
    cbInit(&uart1_rx_buffer, UART_BUFFER_SIZE); 
    init_uart0();
    init_uart1();
}

int uart_putchar(char c, FILE *stream){
    while(cbIsFull(&uart0_tx_buffer)){
        _delay_us(1);
    }
    if(c == '\n'){
        char r = '\r';
        cbWrite(&uart0_tx_buffer, &r);
    }
    cbWrite(&uart0_tx_buffer, &c);
    UCSR0B |= (1<<UDRIE0);
    return 0;
}

int uart_getchar(FILE *stream){
    char c = EOF;//-1;
    if(!cbIsEmpty(uart0_rx_buffer)){
        cbRead(&uart0_rx_buffer, &c);
    }
    return c;
}
