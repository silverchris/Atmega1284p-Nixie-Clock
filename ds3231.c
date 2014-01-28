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

#include "twi_master.h"
#include "ds3231.h"
#include "time.h"


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
        tm tm_struct;
        ds3231_get(&tm_struct);
        printf("Years: 2%03u ", tm_struct.tm_year);
        printf("Month: %u ", tm_struct.tm_mon);
        printf("Day: %u ", tm_struct.tm_mday);
        printf("%02u:", tm_struct.tm_hour);
        printf("%02u:", tm_struct.tm_min);
        printf("%02u\n", tm_struct.tm_sec);
        printf("DS3231 Temperature: %2.1f\n", (double)ds3231_get_temp());
    }
    return 0;
}


/* Set ds3231 time */
void ds3231_set(tm *tm_struct) {
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
    TWI_buffer_out[7] = (dectobcd(tm_struct->tm_year));
    /* write (UTC) time to chip */
    TWI_master_start_write(DS3231_ADDR, 8);
    
    while(TWI_busy){};
    return;
}

/* Get time from ds3231*/
void ds3231_get(tm *tm_struct){
    while(TWI_busy){};
    
    TWI_buffer_out[0] = 0x00;
    
    TWI_master_start_write_then_read(DS3231_ADDR, 1, 8);
    
    while(TWI_busy) {};
    
    tm_struct->tm_sec = BCDToDecimal(TWI_buffer_in[0]);
    tm_struct->tm_min = BCDToDecimal(TWI_buffer_in[1]);
    tm_struct->tm_hour = BCDToDecimal(~(0xc0)&TWI_buffer_in[2]);

    tm_struct->tm_mday = BCDToDecimal(0x3f & TWI_buffer_in[4]);
    tm_struct->tm_mon = BCDToDecimal(0x1f & TWI_buffer_in[5])-1;
    tm_struct->tm_year = BCDToDecimal(TWI_buffer_in[6]);
    
    tm_struct->tm_wday = -1;
    tm_struct->tm_yday = -1;
    return;
}

/* Convert the two temperature registers into a float value. MSB is
   integer temp in degrees celsius. The highest two bits of the LSB
   are fractional temperature in quarters. */
float ds3231_convert_temp(uint8_t msb, uint8_t lsb) {
    /* the LSB of temp is stored at the MSB of the word for some
       reason */
    return((float)msb + (float)(lsb >> 6)/4.0);
}

/* Retrieve temperature from DS3231 and return as a float */
float ds3231_get_temp(void) {
    uint8_t t_msb;
    uint8_t t_lsb;

    while(TWI_busy) {};
    TWI_buffer_out[0] = 0x11;
    TWI_master_start_write_then_read(DS3231_ADDR, 1, 2);
    while(TWI_busy) {};

    t_msb = TWI_buffer_in[0];
    t_lsb = TWI_buffer_in[1];
    return(ds3231_convert_temp(t_msb, t_lsb));
}