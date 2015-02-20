#include <time.h>

#ifndef _DS3231_H_
#define _DS3231_H_

#define DS3231_ADDR 0x68
#define DS3231_NREG 19 // number of registers in memory
#define DS3231_DATETIME_NREG 7 // number of registers for time
#define DS3231_TIME_NREG 3 // number of registers for time
#define DS3231_DATE_NREG 3 // number of registers for date

/* Lots of DS3231 registers read out the same way... */
#define _ds3231_get_seconds ds3231_get_reg_as_int
#define _ds3231_get_minutes ds3231_get_reg_as_int
#define _ds3231_get_date ds3231_get_reg_as_int
#define _ds3231_get_year ds3231_get_reg_as_int

typedef struct{
    int8_t temperature; //Temperature, excluding fraction
    int8_t fraction; //Fraction of temperature in .25C steps
} ds3231_temperature;

uint8_t ds3231_init(void);
void ds3231_set(struct tm *);
void ds3231_get(struct tm *);
float ds3231_convert_temp(uint8_t, uint8_t);
void ds3231_get_temp(ds3231_temperature *);
#endif
