#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "time.h"

tm gmtime(time_t *time){
    time_t t = *time;
    tm tm_struct;
    
    time_t day = t%86400; //Take our day out
    tm_struct.tm_hour = day/3600;
    day -= tm_struct.tm_hour*3600;
    tm_struct.tm_min = day/60;
    day -= tm_struct.tm_min*60;
    tm_struct.tm_sec = day;
        
    uint16_t days = t/86400;
    tm_struct.tm_year = days/365;
    uint8_t leap_days = ((tm_struct.tm_year-1)/4)-((tm_struct.tm_year-1) / 100);
    if(tm_struct.tm_year > 0){
        leap_days += 1;
    }
    printf("Leap Years: %u\n", leap_days);
    tm_struct.tm_yday = (days%365)-leap_days;
    if(tm_struct.tm_yday <= 0){
        tm_struct.tm_year--;
        if(IsLeapYear(tm_struct.tm_year)){
            tm_struct.tm_yday = 366+tm_struct.tm_yday;
        }
        else{
            tm_struct.tm_yday = 365+tm_struct.tm_yday;
        }
    }
    return tm_struct;
}

time_t mktime(tm *tm_struct){
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

// int main(void){
//     tm tm_struct;
//     time_t seconds;
//     tm_struct.tm_sec = 10;
//     tm_struct.tm_min = 10;
//     tm_struct.tm_hour = 10;
//     tm_struct.tm_yday = 365;
//     tm_struct.tm_year = 112;
//     printf("Years: 2%d ", tm_struct.tm_year);
//     printf("Days since January 1st: %d ", tm_struct.tm_yday);
//     printf("Hours: %d ", tm_struct.tm_hour);
//     printf("Minutes: %d ", tm_struct.tm_min);
//     printf("Seconds: %d\n", tm_struct.tm_sec);
//     seconds = mktime(&tm_struct);
//     printf("Seconds Since 01/01/2000: %u\n", seconds);
//     tm_struct = gmtime(&seconds);
//     printf("Years: 2%d ", tm_struct.tm_year);
//     printf("Days since January 1st: %d ", tm_struct.tm_yday);
//     printf("Hours: %d ", tm_struct.tm_hour);
//     printf("Minutes: %d ", tm_struct.tm_min);
//     printf("Seconds: %d\n", tm_struct.tm_sec);
//     
// }