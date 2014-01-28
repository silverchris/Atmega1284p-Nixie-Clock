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
#include "sysclk.h"

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
    ds3231_init();
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
    rtc_timer_setup();
    //ds3231_set(&tm_struct);
    while(1){
        _NOP();
    }
}