#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <avr/interrupt.h>
#include <avr/cpufunc.h>


#include "buffer.h"
#include "gps.h"
#include "sysclk.h"
#include "ds3231.h"

#define FIX_INVALID 0
#define FIX_ZDA 1
#define FIX_GGA 2
#define FIX_RMC 3

extern CircularBuffer uart1_rx_buffer;

int gps_flag;
time_t gps_seconds;
extern uint16_t sys_milli;


uint16_t pps;
uint16_t tcnt_last;
uint16_t tcnt;

uint8_t milli_reset;
uint8_t pps_count;

uint8_t PPS_DEBUG;

extern int16_t sysclk_adj;
extern int adj_ready;

uint8_t valid;

int8_t fix_type;


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
        printf("%u\n", uart1_rx_buffer.size);
        printf("%u\n", uart1_rx_buffer.start);
        printf("%u\n", uart1_rx_buffer.end);
        printf("%s\n", data);
        return;
    }
//     printf("%s\n", data);
    char *token;
    token = strtok(data, ",");
    if(!strcmp(token, "$GPGGA")){
        if(fix_type != FIX_ZDA){
            gps_seconds = gga(token);
            fix_type = FIX_GGA;
        }
        else{
            gga(token);
        }
    }
    else if(!strcmp(token, "$GPZDA")){
        gps_seconds = zda(token);
        fix_type = FIX_ZDA;
    }
    else if(!strcmp(token, "$GPRMC")){
        if(fix_type != FIX_ZDA){
            gps_seconds = rmc(token);
            fix_type = FIX_RMC;
        }
        else{
            rmc(token);
        }
    }
}

void pps_enable(void){
    valid = 0;
    fix_type = FIX_INVALID;
    PPS_DEBUG = 0;
    //TODO: Probably take into account the timestamp/last message is for the previous second
    TIMSK1 |= (1 << ICIE1);
}


uint32_t last_val;
float MovingAverage;

void pps_filter(void){
    if(valid == 0){
        return;
    }
    int32_t val = (1000000*pps)/100;
    val += tcnt;
    int32_t result = val-last_val;
    
    if(result >= (998*1000000)/100 && result <= (1000*1000000)/100){
        result -= (1000*1000000)/100;
    }
    if(result <= ((998*1000000)/100)*-1 && result >= ((1000*1000000)/100)*-1){
        result += (1000*1000000)/100;
    }    
    float AmplitudeFactor = .5;//1.0/2.0;
    float DecayFactor = .62;//1.0-AmplitudeFactor;
    MovingAverage *= DecayFactor;
    MovingAverage += AmplitudeFactor * result;
    if(milli_reset && pps_count > 20){
        sysclk_adj += round(MovingAverage);
        adj_ready = 1;
    }
    if(PPS_DEBUG == 1){
            printf("%" PRIi32 "\n", val);
            printf("%" PRIi32 "\n", last_val);
            printf("%" PRIi32 "\n", result);
            printf("%" PRIi32 "\n", (1*1000000)/100);
            printf("%f\n", MovingAverage);
            printf("%u,", pps);
            printf("%u,", tcnt);
            printf("%li,", result);
            printf("%f,", MovingAverage);
            printf("%u,", sysclk_adj);
            printf("%u,%u\n", valid, fix_type);
            printf("\n");
    }
    
    last_val = val;
}

ISR(TIMER1_CAPT_vect){
    tcnt = ICR1;
    pps = sys_milli;
    if(!milli_reset && pps_count == 60){
//         sys_milli = 0;
//         TCNT1 = 0;
        set_system_time(gps_seconds);
        milli_reset = 1;
        pps_count = 0;
    }
    if(pps_count<254){
        pps_count++;
    }
}
