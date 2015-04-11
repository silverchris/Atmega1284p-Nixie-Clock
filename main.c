#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/cpufunc.h>
#include <time.h>

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
#include "gps.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

extern int ui_flag;
extern int gps_flag;
extern int second_flag;

extern uint16_t pps;
extern uint16_t pps_last;
extern uint16_t pps_tcnt_last;
extern uint16_t pps_icr;
extern time_t gps_seconds;
extern time_t seconds;

int main(void){
    setup_uarts();
    stdout = stdin = &uart_str;
    sei();
    printf("Software Version: %s\n", VERSION);
    uint16_t xbootver;
    xboot_get_version(&xbootver);
    printf("xboot Version:    %d.%d\n", xbootver>>8, xbootver&0xFF);
//     TWI_init();
//     ds3231_init();
    sysclk_setup();
    tz_init();
//     spi_init();
//     display_init();
    setup_ui();
    pps_enable();
    while(1){
        if(second_flag){
            printf("PPS at: %u\n", pps);
            printf("Diff: %u-%u=%i\n", pps_last, pps, pps_last-pps);
            sysclk_adj(pps_filter());
            second_flag = 0;
            printf("ICR1: %u\n", pps_icr);
            pps_last = pps;
            printf("Sys vs GPS: %lu-%lu = %li\n", time(NULL), gps_seconds, time(NULL)-gps_seconds);
        }
        if(gps_flag){
            run_gps();
        }
        if(ui_flag){
            run_ui();
        }
        else{
            _NOP();
        }
    }
}