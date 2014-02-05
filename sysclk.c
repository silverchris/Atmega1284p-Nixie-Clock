#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "time.h"
#include "display.h"

time_t sys_seconds;//seconds since power on
uint16_t sys_milli;//milliseconds inbetween seconds
uint8_t led;

ISR (TIMER1_COMPA_vect){
    if(sys_milli == 1024){
        sys_seconds++;
        sys_milli = 0;
        if(led == 0){
            PORTA |= (1<<PA4);
            led++;
        }
        else if(led == 1){
            led = 0;
            PORTA &= ~(1<<PA4);
        }
        uint8_t array[6];
        tm tm_struct;
        gmtime_r(&sys_seconds, &tm_struct);
        utc_digits(&tm_struct, &array[0]);
        display(&array[0]);
    }
    else{
        sys_milli++;
    }
}

void rtc_timer_setup(void){
    //set up  timer here to match at 32 and clear timer on match
    TCCR1A = 0x00;
    TCCR1B |= (1 << WGM12)|(1 << CS12)|(1 << CS11)|(1 << CS10);//Enable CTC mode and external clock source on rising edge
    TCCR1C = 0xFF;
    TCNT1 = 0;
    OCR1A = 32; 
    TIMSK1 |= (1 << OCIE1A);//setup Match interupt
    DDRA |= (1<<PA4); //set direction for blinking led
    led = 0;
}
