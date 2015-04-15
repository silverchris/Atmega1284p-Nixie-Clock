#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include "spi.h"
#include "main.h"
#include <time.h>

const uint8_t hdigit[10] PROGMEM = {0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
const uint8_t ldigit[10] PROGMEM = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x00};

void display(int8_t array[]){
    PORTA |=  (1<<PA2); //Disable the display
    
    PORTA &= ~(1<<PA0); //Reset The shift register
    PORTA |=  (1<<PA0);
    
    int x;
    for(x = 0; x < 6; x++){
        if(array[x] < 0){
            //Turn on LHDP if number if number is negative
            array[x] = array[x]*-1;
            spi_write(pgm_read_byte((&hdigit[array[x]]))|128);
        }
        else{
            spi_write(pgm_read_byte((&hdigit[array[x]])));
        }
        spi_write(pgm_read_byte(&ldigit[array[x]]));
    }
    
    PORTA |=  (1<<PA1); //Latch the Data 
    PORTA &= ~(1<<PA1);
    
    PORTA &= ~(1<<PA2); //Renable the display
}


void display_init(void){
    DDRA |= (1<<PA0)|(1<<PA1)|(1<<PA2);
    PORTA &= ~(1<<PA0); //Reset The shift register
    PORTA |=  (1<<PA0);
}

void utc_digits(struct tm *tm_struct, int8_t array[]){
    array[5] = tm_struct->tm_hour/10;
    array[4] = tm_struct->tm_hour%10;
    array[3] = (tm_struct->tm_min/10)*-1;
    array[2] = tm_struct->tm_min%10;
    array[1] = (tm_struct->tm_sec/10)*-1;
    array[0] = tm_struct->tm_sec%10;
}