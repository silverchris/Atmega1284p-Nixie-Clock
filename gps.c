#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <avr/interrupt.h>
#include <avr/cpufunc.h>


#include "buffer.h"
#include "gps.h"
#include "sysclk.h"
#include "ds3231.h"

extern CircularBuffer uart1_rx_buffer;

int gps_flag;
time_t gps_seconds;
extern uint16_t sys_milli;


uint16_t pps;
uint16_t pps_last;
uint16_t pps_tcnt_last;
uint16_t pps_icr;


#define FILTER_LENGTH 3
int16_t PPS_FILTER_ARRAY[FILTER_LENGTH];
uint8_t PPS_FILTER_LOC;


uint8_t milli_reset;
uint8_t pps_count;

uint8_t PPS_DEBUG;

extern int16_t sysclk_adj;
extern int adj_ready;

time_t zda(char *data){
    //TODO: Get local offset from GPS data
    struct tm tm_struct;
    char *token = &data[0];
    int i;
    int32_t local_offset = 0;
    token = strtok(token+7, ",");//+7 to eat the first bit
    for(i=1;i<=6;i++){
        switch(i){
            case 1:
                sscanf(token, "%02" SCNd8 "%02" SCNd8 "%02" SCNd8 , &tm_struct.tm_hour,
                       &tm_struct.tm_min, &tm_struct.tm_sec);
                break;
            case 2:
                tm_struct.tm_mday = atoi(token)-1;
                break;
            case 3:
                tm_struct.tm_mon = atoi(token)-1;
                break;
            case 4:
                tm_struct.tm_year = atoi(token)-1900;
                break;
            case 5:
                local_offset = atoi(token)*3600;
                break;
            case 6:
                if(local_offset < 0){
                    local_offset = local_offset-(atoi(token)*60);
                }
                else{
                    local_offset = local_offset+(atoi(token)*60);
                }
                break;
        }
        token = strtok(NULL, ",");
    }
    return mk_gmtime(&tm_struct);
}

time_t gga(char *data){
    struct tm tm_struct;
    time_t seconds;
    time(&seconds);
    gmtime_r(&seconds, &tm_struct);
    char *token = &data[0];
    int i;
    uint8_t valid = 0;
    token = strtok(token+7, ",");//+7 to eat the first bit
    for(i=1;i<=6;i++){
        switch(i){
            case 1:
                sscanf(token, "%02" SCNd8 "%02" SCNd8 "%02" SCNd8 , &tm_struct.tm_hour,
                       &tm_struct.tm_min, &tm_struct.tm_sec);
                break;
            case 6:
                valid = atoi(token);
                break;
        }
        token = strtok(NULL, ",");
    }
    if(valid){
        return mk_gmtime(&tm_struct);
    }
    else{
        return 0;
    }
}

time_t rmc(char *data){
    struct tm tm_struct;
    time_t seconds;
    time(&seconds);
    gmtime_r(&seconds, &tm_struct);
    char *token = &data[0];
    int i;
    uint8_t valid = 0;
    token = strtok(token+7, ",");//+7 to eat the first bit
    for(i=1;i<=6;i++){
        switch(i){
            case 1:
                sscanf(token, "%02" SCNd8 "%02" SCNd8 "%02" SCNd8 , &tm_struct.tm_hour,
                       &tm_struct.tm_min, &tm_struct.tm_sec);
                break;
            case 2:
                if(token[0] == 'A'){
                    valid = 1;
                }
                else{
                    valid = 0;
                }
                break;
        }
        token = strtok(NULL, ",");
    }
    if(valid){
        return mk_gmtime(&tm_struct);
    }
    else{
        return 0;
    }
}

uint8_t check_crc(char *data){
    /* Returns 0 if CRC correct */
    uint8_t length = strlen(data);
    int i;
    uint8_t crc = 0;
    for(i=1;i<=length-4;i++){
        crc^=data[i];
    }
    return crc-(uint8_t)strtol(&data[length-2], NULL, 16);
}

void run_gps(void){
    gps_flag = 0;
    char data[100] = {0};
    uint8_t l = 0;
    while(!cbIsEmpty(uart1_rx_buffer)){
        l = strlen(data);
        if(l<100){
            cbRead(&uart1_rx_buffer, &data[l]);
        }
        if(data[l] == '\n'){
            break;
        }
    }
    data[strlen(data)-2] = '\0';
    if(check_crc(data)){
        printf("Invalid NMEA Sentence\n");
        return;
    }
    char *token;
    token = strtok(data, ",");
    if(!strcmp(token, "$GPGGA")){
        gps_seconds = gga(token);
    }
    else if(!strcmp(token, "$GPZDA")){
        gps_seconds = zda(token);
    }
    else if(!strcmp(token, "$GPRMC")){
        gps_seconds = rmc(token);
    }
}

void pps_enable(void){
    PPS_DEBUG = 1;
    PPS_FILTER_LOC = 0;
    //TODO: Probably take into account the timestamp/last message is for the previous second
    TIMSK1 |= (1 << ICIE1);
}


void pps_filter(void){
    int16_t ts = 0;
    int16_t result;
    int i;
    if(pps_tcnt_last > pps_icr){
        ts = (pps_tcnt_last-pps_icr)*-1;
    }
    else if(pps_icr > pps_tcnt_last){
        ts = (pps_icr-pps_tcnt_last);
    }
    else if(pps_icr == pps_tcnt_last){
        ts = 0;
    }
//     printf("Counter Diff: %u-%u=%i\n", pps_icr, pps_tcnt_last, ts);
    PPS_FILTER_ARRAY[PPS_FILTER_LOC] = ts;
    result = 0;
    for(i=0;i<=FILTER_LENGTH-1;i++){
        result += PPS_FILTER_ARRAY[i];
    }
//     result = result/FILTER_LENGTH;
    if((result/FILTER_LENGTH) > 200 || (result/FILTER_LENGTH) < -200){
        result = 0;
    }
    pps_tcnt_last = pps_icr;
    PPS_FILTER_LOC++;
    if(PPS_FILTER_LOC == FILTER_LENGTH){
        PPS_FILTER_LOC = 0;
//         return resuDlt;
    }
//     else{
//         return 0;
//     }
    sysclk_adj += result/FILTER_LENGTH;
    adj_ready = 1;
    if(PPS_DEBUG == 1){
            printf("%u,", pps);
            printf("%f,", (float)result/FILTER_LENGTH);
            printf("%u,", sysclk_adj);
            printf("%u,", pps_icr);
            ds3231_temperature temperature;
            ds3231_get_temp(&temperature);
            printf("%lu,%lu,%i.%i\n", time(NULL), gps_seconds, temperature.temperature, temperature.fraction);
    }
}

ISR(TIMER1_CAPT_vect){
    pps_icr = ICR1;
    pps = sys_milli;
    if(!milli_reset && pps_count == 5){
        sysclk_adj = OCR1A_VAL;
    }
    if(!milli_reset && pps_count == 60){
        sys_milli = 0;
        TCNT1 = 0;
        set_system_time(gps_seconds);
        milli_reset = 1;
    }
    pps_count++;
}
