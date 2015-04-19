#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#include "main.h"
#include "sysclk.h"
#include "uart.h"
#include "tz.h"
#include "hash.h"
#include "ds3231.h"
// #include <util/delay.h>


static uint8_t ui_mode;
int ui_flag;

#define UI_MODE_MAIN 0
#define UI_MODE_END 1
#define UI_MODE_BUILD_LINE 2
#define UI_MODE_DISPLAY_TIME 11
#define UI_MODE_SET 12
#define UI_MODE_TZ 13
#define UI_MODE_TEST 14


void setup_ui(void){
    ui_flag = 0;
    ui_mode = UI_MODE_MAIN;
    printf("\nPlease Select an Option\n");
    printf("\t 1: Display time\n");
    printf("\t 2: Set Time\n");
    printf("\t 3: Set Timezone\n");
    printf("Option #>");
}

void display_time(void){
    struct tm tm_struct;
    time_t seconds;
    time(&seconds);
    gmtime_r(&seconds, &tm_struct);
    char time_str[31];
    strftime(time_str, sizeof(time_str), "UTC:   %Y-%m-%d %H:%M:%S\n", &tm_struct);
    printf(time_str);
    localtime_r(&seconds, &tm_struct);
    strftime(time_str, sizeof(time_str), "Local: %Y-%m-%d %H:%M:%S\n", &tm_struct);
    printf(time_str);
    printf("Seconds: %lu\n", seconds);
    ui_mode = UI_MODE_MAIN;
}

void ui_end(void){
    ui_mode = UI_MODE_END;
    ui_flag = 1;
}

void ds3231_debug(void){
    struct tm tm_struct;
    time_t seconds;
    char time_str[31];
    time(&seconds);
    gmtime_r(&seconds, &tm_struct);
    tm_struct.tm_year = 215;
    strftime(time_str, sizeof(time_str), "UTC:   %Y-%m-%d %H:%M:%S\n", &tm_struct);
    printf(time_str);
    ds3231_set(&tm_struct);
    tm_struct.tm_year = 0;
    ds3231_get(&tm_struct);
    strftime(time_str, sizeof(time_str), "UTC:   %Y-%m-%d %H:%M:%S\n", &tm_struct);
    printf(time_str);
}

void run_ui(void){
    ui_flag = 0;
    int i;
    char line[30];
    char name[30] = "";
    switch(ui_mode){
        case UI_MODE_MAIN:
            if(fgets(line, sizeof line - 1, stdin) == NULL){
                _NOP();//ui_mode = UI_MODE_BUILD_LINE;
            }
            printf("\n");
            if(sscanf(line, "%02d", &i)){
                i = i+10;
                if(i > 10 && i <= 14){
                    ui_mode = i;
                    ui_flag = 1;
                }
                else {
                    ui_end();
                }
            }
            else {
                ui_end();
            }
            break;
            
        case UI_MODE_END:
            setup_ui();
            break;
            
        case UI_MODE_BUILD_LINE:
            printf("\n%s\n", line);
            ui_mode = UI_MODE_MAIN;
            break;
            
        case UI_MODE_DISPLAY_TIME:
            display_time();
            ui_end();
            break;
        case UI_MODE_TZ:
            if(fgets(line, sizeof line - 1, stdin) == NULL){
                _NOP();//ui_mode = UI_MODE_BUILD_LINE;
            }
            if(sscanf(line, "%29s", name)){
                printf("%s\n", name);
                if(name[0] == '!'){
                    ui_end();
                }
                else if(name[0] == 'l'){
                    list_zones();
                    printf(">");
                }
                else if(strlen(name)>3){
                    if(tz_update(name)){
                        printf("Set %s\n", name);
                        ui_end();
                    }
                }
                else{
                    printf("\nPlease Enter a Timezone\nl - to list\n! - to exit\n>");
                }
            }
            break;
        case UI_MODE_TEST:
            ds3231_debug();
            ui_end();
            break;
    }
}


