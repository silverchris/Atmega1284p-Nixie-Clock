#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/cpufunc.h>

#include "main.h"
#include "uart.h"
#include "xbootapi.h"
#include "ds3231.h"
#include "twi_master.h"
#include "time.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

int main(void){
    setup_uarts();
    stdout = stdin = &uart_str;
    sei();
    printf("Software Version: %s\n", VERSION);
    uint16_t xbootver;
    xboot_get_version(&xbootver);
    printf("xboot Version:    %d.%d\n", xbootver>>8, xbootver&0xFF);
    TWI_init();
    //ds3231_init();
    printf("DS3231 Temperature: %2.1f\n", ds3231_get_temp());
    uint32_t count = 0;
    tm tm_struct;
    tm_struct.tm_sec = 10;
    tm_struct.tm_min = 10;
    tm_struct.tm_hour = 18;
    tm_struct.tm_yday = 27;
    tm_struct.tm_year = 14;
    tm_struct.tm_mday = 27;
    tm_struct.tm_mon = 4;
    printtime(&tm_struct);
    //ds3231_set(&tm_struct);
    
    DDRA |= (1<<PA4);
    while(1){
        count++;
        if(count == 1000000){
            PORTA |= (1<<PA4);
        }
        if(count == 2000000){
            count = 0;
            PORTA &= ~(1<<PA4);
            ds3231_get(&tm_struct);
            printtime(&tm_struct);
        }
    }
}