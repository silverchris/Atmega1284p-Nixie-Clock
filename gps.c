#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <avr/interrupt.h>
#include <avr/cpufunc.h>


#include "buffer.h"
#include "gps.h"

extern CircularBuffer uart1_rx_buffer;

int gps_flag;
time_t gps_seconds;

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
    struct tm tm_struct;
    gmtime_r(&gps_seconds, &tm_struct);
    printf("%s\n", asctime(&tm_struct));
}

void pps_enable(void){
    //TODO: Probably take into account the timestamp/last message is for the previous second
    EICRA = (1<<ISC11)|(1<<ISC10);
    EIMSK = (1<<INT1);
}

ISR(INT1_vect){
    printf("PPS\n");
}
