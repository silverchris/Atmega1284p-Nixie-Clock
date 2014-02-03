#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <avr/pgmspace.h>


#include "time.h"

const static uint8_t months[2][12] PROGMEM = {
    {31,28,31,30,31,30,31,31,30,31,30,31},
    {31,29,31,30,31,30,31,31,30,31,30,31}
};

const static uint8_t wday[12] PROGMEM = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

tm *gmtime_r(time_t *time, tm *tm_struct){
    time_t t = *time;
    
    time_t day = t%86400; //Take our day out
    tm_struct->tm_hour = day/3600;
    day -= tm_struct->tm_hour*3600;
    tm_struct->tm_min = day/60;
    day -= tm_struct->tm_min*60;
    tm_struct->tm_sec = day;
        
    uint16_t days = t/86400;
    tm_struct->tm_year = days/365;
    uint8_t leap_days = ((tm_struct->tm_year-1)/4)-((tm_struct->tm_year-1) / 100);
    if(tm_struct->tm_year > 0){
        leap_days += 1;
    }
    tm_struct->tm_yday = (days%365)-leap_days;
    if(tm_struct->tm_yday < 0){
        tm_struct->tm_year--;
        if(IsLeapYear(tm_struct->tm_year)){
            tm_struct->tm_yday = 366+tm_struct->tm_yday;
        }
        else{
            tm_struct->tm_yday = 365+tm_struct->tm_yday;
        }
    }
    uint16_t monthday = tm_struct->tm_yday+1;
    uint8_t monthcount = 0;
    uint8_t leap = IsLeapYear(tm_struct->tm_year);
    while(monthday > pgm_read_byte(&months[leap][monthcount])){
        if(monthday > pgm_read_byte(&months[leap][monthcount])){
            monthday -= pgm_read_byte(&months[leap][monthcount]);
            monthcount += 1;
        }
    }
    tm_struct->tm_mon = monthcount;
    tm_struct->tm_mday = monthday;
    uint8_t y = tm_struct->tm_year;
    if(y == 0 && tm_struct->tm_mon < 2){
        y += 28;
    }
    y -= (tm_struct->tm_mon < 2);
    tm_struct->tm_wday = (y+(y/4)-(y/100)+pgm_read_byte(&wday[tm_struct->tm_mon])+tm_struct->tm_mday)%7;
    return tm_struct;
}

time_t timegm(tm *tm_struct){
    if(tm_struct->tm_yday == -1){
        uint8_t leap = IsLeapYear(tm_struct->tm_year);
        int c;
        for(c = 0; c < tm_struct->tm_mon; c++){
            tm_struct->tm_yday += pgm_read_byte(&months[leap][c]);
            printf("%d\n",pgm_read_byte(&months[leap][c]));
        }
        tm_struct->tm_yday += tm_struct->tm_mday;
    }
    time_t seconds;
    seconds = tm_struct->tm_sec;
    seconds += tm_struct->tm_min*60;
    seconds += tm_struct->tm_hour*3600;
    seconds += tm_struct->tm_yday*86400;
    seconds += (tm_struct->tm_year)*31536000;
    /* Leap year shit */
    seconds += ((tm_struct->tm_year-1)/4)*86400;
    
    if(tm_struct->tm_year >= 101){
        seconds -= 86400;
    }
    if(tm_struct->tm_year >= 1){
        seconds += 86400;
    }
    return seconds;
}

void utc_digits(tm *tm_struct, uint8_t array[]){
    array[5] = tm_struct->tm_hour/10;
    array[4] = tm_struct->tm_hour%10;
    array[3] = tm_struct->tm_min/10;
    array[2] = tm_struct->tm_min%10;
    array[1] = tm_struct->tm_sec/10;
    array[0] = tm_struct->tm_sec%10;
}

void printtime(tm *tm_struct){
    printf("Years: 2%03u ", tm_struct->tm_year);
    printf("Days since January 1st: %d ", tm_struct->tm_yday);
    printf("Month: %u ", tm_struct->tm_mon);
    printf("Day: %u ", tm_struct->tm_mday);
    printf("Weekday: %u\n", tm_struct->tm_wday);
    printf("%02u:", tm_struct->tm_hour);
    printf("%02u: ", tm_struct->tm_min);
    printf("%02u\n", tm_struct->tm_sec);
}