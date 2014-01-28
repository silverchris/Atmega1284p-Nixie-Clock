#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "time.h"

time_t rtc_seconds;//seconds since epoch, duh
uint16_t rtc_milli;//milliseconds inbetween seconds

ISR (TIMER1_COMPA_vect){
    if(rtc_milli == 1024){
        rtc_seconds++;
        rtc_milli = 0;
        tm tm_struct;
        gmtime_r(&rtc_seconds, &tm_struct);
        printtime(&tm_struct);
    }
    else{
        rtc_milli++;
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
}
