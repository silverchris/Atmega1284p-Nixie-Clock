#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/cpufunc.h>

#include "main.h"
#include "uart.h"
#include "xbootapi.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

int main(void){
    setup_uarts();
    stdout = stdin = &uart_str;
    sei();
    printf("Version: %s\n", VERSION);
    uint32_t count = 0;
    DDRA |= (1<<PA4);
    while(1){
        count++;
        if(count == 1000000){
            PORTA |= (1<<PA4);
        }
        if(count == 2000000){
            count = 0;
            PORTA &= ~(1<<PA4);
        }
    }
}