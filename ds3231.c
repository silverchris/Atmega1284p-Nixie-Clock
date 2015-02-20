/* Author: Nicholas Nell
   Email: nico.nell@gmail.com 

   DS3231 Interface 
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "twi_master.h"
#include "ds3231.h"



static const uint8_t ds3231_init_seq[] PROGMEM = {
    0x00, // seconds
    0x00, // minutes
    0x00, // hours, 12/24 config
    0x01, // Day (1-7)
    0x01, // date (1-31)
    0x01, // month & century
    0x00, // year (00 - 99)
    0x00, // alarm1 seconds
    0x00, // alarm1 minutes
    0x00, // alarm1 hours
    0x01, // alarm1 date
    0x00, // alarm2 minutes
    0x00, // alarm2 hours
    0x01, // alarm2 date
    0x00, // control
    0x08, // control/status
    0x00 // aging offset
};

volatile uint8_t led = 0;

/* decimal to binary coded decimal helper */
static inline uint8_t dectobcd(uint8_t k) {
    return((k/10)*16 + (k%10));
}

static inline uint8_t BCDToDecimal (uint8_t bcdByte){
  return (((bcdByte & 0xF0) >> 4) * 10) + (bcdByte & 0x0F);
}
 


/* Initialize the DS3231 chip. TWI_init() must have been called for
   this to work. */
uint8_t ds3231_init(void) {
    uint8_t i = 0;

    while(TWI_busy){};

    TWI_buffer_out[0] = 0x0F;    
    TWI_master_start_write_then_read(DS3231_ADDR, 1, 1);
    while(TWI_busy){};
    if((0x80 & TWI_buffer_in[0])){
        printf("RTC STOPPED, Reinitializing!\n");
        /* word address for config start */
        TWI_buffer_out[0] = 0x00;

        /* call me only after TWI_init has been called */
        for (i = 0; i < sizeof(ds3231_init_seq); i++) {
            TWI_buffer_out[i + 1] = pgm_read_byte(&ds3231_init_seq[i]);
        }

        /* write config to chip */
        TWI_master_start_write(DS3231_ADDR, sizeof(ds3231_init_seq) + 1);

        while(TWI_busy){};
    }
    else{
        printf("RTC Valid\n");
        struct tm tm_struct;
        ds3231_get(&tm_struct);
        char time[25];
        strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S\n", &tm_struct);
        printf(time);
//         printf("Years: 2%03u ", tm_struct.tm_year);
//         printf("Month: %u ", tm_struct.tm_mon);
//         printf("Day: %u ", tm_struct.tm_mday);
//         printf("%02u:", tm_struct.tm_hour);
//         printf("%02u:", tm_struct.tm_min);
//         printf("%02u\n", tm_struct.tm_sec);
        ds3231_temperature temp;
        ds3231_get_temp(&temp);
        printf("DS3231 Temperature: %d.%dC\n", temp.temperature, temp.fraction);
        
    }
    return 0;
}


/* Set ds3231 time */
void ds3231_set(struct tm *tm_struct) {
    //TODO: Fix so that we actually set the full year in the RTC
    while(TWI_busy){};

    /* word address for time set start */
    TWI_buffer_out[0] = 0x00;
    TWI_buffer_out[1] = dectobcd(tm_struct->tm_sec);
    TWI_buffer_out[2] = dectobcd(tm_struct->tm_min);
    /* maintain 24-hour setting */
    TWI_buffer_out[3] = (~(0xc0) & dectobcd(tm_struct->tm_hour));
    /* Set Date*/
    TWI_buffer_out[4] = 0x00;//skip this reg
    TWI_buffer_out[5] = (0x3f & dectobcd(tm_struct->tm_mday));
    TWI_buffer_out[6] = (0x1f & dectobcd(tm_struct->tm_mon+1));
    TWI_buffer_out[7] = (dectobcd(tm_struct->tm_year-100));
    /* write (UTC) time to chip */
    TWI_master_start_write(DS3231_ADDR, 8);
    
    while(TWI_busy){};
    return;
}

/* Get time from ds3231*/
void ds3231_get(struct tm *tm_struct){
    //TODO: Fix so that we actually get the whole year from the rtc
    while(TWI_busy){};
    
    TWI_buffer_out[0] = 0x00;
    
    TWI_master_start_write_then_read(DS3231_ADDR, 1, 8);
    
    while(TWI_busy) {};
    
    tm_struct->tm_sec = BCDToDecimal(TWI_buffer_in[0]);
    tm_struct->tm_min = BCDToDecimal(TWI_buffer_in[1]);
    tm_struct->tm_hour = BCDToDecimal(~(0xc0)&TWI_buffer_in[2]);

    tm_struct->tm_mday = BCDToDecimal(0x3f & TWI_buffer_in[4]);
    tm_struct->tm_mon = BCDToDecimal(0x1f & TWI_buffer_in[5])-1;
    tm_struct->tm_year = BCDToDecimal(TWI_buffer_in[6])+100;
    
    tm_struct->tm_wday = -1;
    tm_struct->tm_yday = -1;
    return;
}

/* Retrieve temperature from DS3231 and return as a float */
void ds3231_get_temp(ds3231_temperature *temp) {
    while(TWI_busy) {};
    TWI_buffer_out[0] = 0x11;
    TWI_master_start_write_then_read(DS3231_ADDR, 1, 2);
    while(TWI_busy) {};

    temp->temperature = TWI_buffer_in[0];
    temp->fraction = (TWI_buffer_in[1]>>6)*25;
    return;
}