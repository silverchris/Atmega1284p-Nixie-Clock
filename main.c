#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/cpufunc.h>

#include "main.h"
#include "uart.h"
#include "xbootapi.h"
#include "ds3231.h"
#include "twi_master.h"
#include "sysclk.h"
#include "spi.h"
#include "display.h"
#include "ui.h"
#include "buffer.h"
#include "tz.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

extern int ui_flag;

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
    sysclk_setup();
    tz_init();
    spi_init();
    display_init();
    setup_ui();
    while(1){
        if(ui_flag){
            run_ui();
        }
        else{
            _NOP();
        }
    }
}