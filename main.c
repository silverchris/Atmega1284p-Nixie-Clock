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
#include "display.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

extern int ui_flag;
extern int gps_flag;
extern int second_flag;

int8_t utc_toggle;

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
    pps_enable();
    while(1){
        if(second_flag){
            second_flag = 0;
            pps_filter();
            //Display stuff temp
            int8_t array[6];
            struct tm tm_struct;
            time_t seconds = time(NULL);
            if(!(seconds%10)){
                if(utc_toggle == 1){
                    utc_toggle = 0;
                }
                else{
                    utc_toggle = 1;
                }
            }
            if(utc_toggle == 0){
                gmtime_r(&seconds, &tm_struct);
            }
            else{
                localtime_r(&seconds, &tm_struct);
            }
            utc_digits(&tm_struct, &array[0]);
            display(&array[0]);
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