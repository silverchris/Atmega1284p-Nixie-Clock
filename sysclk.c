#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"
#include <time.h>
#include "display.h"
#include "ds3231.h"
#include "sysclk.h"

#include <util/delay.h>



uint16_t sys_milli;//milliseconds inbetween seconds
uint8_t led;
int second_flag;
int16_t pending_adj;
int adj_ready;
int16_t sysclk_adj;

ISR (TIMER1_COMPA_vect){
    sys_milli++;
    if(sys_milli == 500){
        if(adj_ready){
            OCR1A = sysclk_adj;
            adj_ready = 0;
        }
    }
    else if (sys_milli == 501){
        OCR1A = OCR1A_VAL;
    }
    if(sys_milli >= 1000){
        system_tick();
        sys_milli = 0;
        
        //Turn on the LED, and set up timer2 to interrupt
        PORTA |= (1<<PA4);
        TCNT2 = 10;
        TIFR2 = 0xFF;
        TIMSK2 = (1 << TOIE2);
        second_flag = 1;
    }
//     _delay_us(10);
//     TCCR1C = (1 << FOC1A); //Clock debug shit
}

ISR (TIMER2_OVF_vect){
    //turn off LED
    TIMSK2 = 0;
    PORTA &= ~(1<<PA4);
}


void sysclk_setup(void){
    sysclk_adj = OCR1A_VAL;
    //Divide 10mhz by 8, and then again by 1250 == 1millisecond
    
    //Set up timer2 for turning the LED off
    TCCR2A = (1 << WGM20);
    TCCR2B = (1 << CS22)|(1 << CS21)|(1 << CS20);
    TCNT2 = 0;
    
    sys_milli = 0;
    TCCR1A = (1 << COM1A0);//= 0x00;
    TCCR1B = (1 << ICES1)|(0 << ICNC1)|(1 << WGM12)|(0 << CS12)|(0 << CS11)|(1 << CS10);//Enable CTC mode and clock from osc
    TCCR1C = 0x00;
    OCR1A = OCR1A_VAL;
    TCNT1 = 0;
    TIMSK1 |= (1 << OCIE1A);//setup Match interupt
    DDRA |= (1<<PA4); //set direction for blinking led
    DDRD |= (1<<PD5); //Set direction for OC1A, for sysclock testing
    led = 0;
    struct tm tm_struct;
//     ds3231_get(&tm_struct);
//     1424399080-UNIX_OFFSET; //mk_gmtime(&tm_struct);
//     1430697600-UNIX_OFFSET;
    set_system_time(1425797990-UNIX_OFFSET);//Just before DST for EST
//     1425945600-UNIX_OFFSET;
//     1446357590-UNIX_OFFSET; //JUST BEFORE DST ENDS
}
