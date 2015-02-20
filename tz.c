#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <avr/pgmspace.h>

#include <time.h>
#include "tz.h"

#include "main.h"
#include <util/delay.h>


extern uint_farptr_t _binary_tz_names_start;
extern uint_farptr_t _binary_tz_names_end;
extern uint_farptr_t _binary_tz_names_size;

extern uint_farptr_t _binary_tz_offset_start;
extern uint_farptr_t _binary_tz_offset_end;
extern uint_farptr_t _binary_tz_offset_size;

extern uint_farptr_t _binary_tz_zones_start;
extern uint_farptr_t _binary_tz_zones_end;
extern uint_farptr_t _binary_tz_zones_size;

extern uint_farptr_t _binary_tz_rules_start;
extern uint_farptr_t _binary_tz_rules_end;
extern uint_farptr_t _binary_tz_rules_size;

extern time_t sys_seconds;

extern long __utc_offset;

uint16_t TZ;
uint16_t LAST_DST;

time_t LAST_DST_UPDATE;

char wday[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

uint8_t week_day(int16_t year, uint8_t month, uint8_t d){
    //TODO Optimize for our dates, check if this works with an 8bit type to 2100
    uint8_t y = year;
    uint8_t m = month+1;
    return (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7;
}

void wdays_in_month(uint8_t (*days)[5], int16_t year, uint8_t month, uint8_t day){
    uint8_t first = week_day(year, month, 1);
    uint8_t last_day = month_length(year, month);
    uint8_t i;
    uint8_t c = 0;
    for(i = 1;i<=last_day; i++){
        if(week_day(year, month, i) == day){
            (*days)[c] = i;
            c++;
        }
    }
}

uint8_t day_to_int(char day[3]){
    uint8_t i;
    for(i=0;i<=6;i++){
        if(!strcmp(day, wday[i])){
            break;
        }
    }
    return i;
}

uint8_t last(uint8_t (*days)[5]){
    uint8_t highest = 0;
    uint8_t i;
    for(i=0;i<=4;i++){
        if((*days)[i] > highest){
            highest = (*days)[i];
//             printf("%i\n", (*days)[i]);
        }
    }
    return highest;
}

uint8_t decode_day(tz_rule *rule, uint8_t year){
    uint8_t rvalue;
    uint8_t days[5] = {0, 0, 0, 0};
    if(strlen(rule->day) <= 2){
        rvalue = atoi(rule->day);
    }
    if( ((uint8_t)rule->day[0] >= 65) && ((uint8_t)rule->day[0] <= 90) ){
        char day[4];
        strncpy(day, rule->day, 3);
        day[3] = '\0';
        char op[3];
        strncpy(op, rule->day+3, 2);
        op[2] = '\0';
        uint8_t day_num = atoi(rule->day+5);
        if(!strcmp(op, ">=")){
            wdays_in_month(&days, year, rule->month, day_to_int(day));
            int i;
            for(i=0;i<=4;i++){
                if(days[i] >= day_num){
                    rvalue = days[i];
                    break;
                }
            }
        }
        if(!strcmp(op, "<=")){
            wdays_in_month(&days, year, rule->month, day_to_int(day));
            uint8_t i;
            for(i=4;i<=0;i--){
                if(days[i] <= day_num){
                    rvalue = days[i];
                    break;
                }
            }
        }
    }
    if(!strncmp(rule->day, "last", 4)){
        char day[4];
        strcpy(day, rule->day+4);
        wdays_in_month(&days, year, rule->month, day_to_int(day));
//         int i = 0;
//         for(i=0;i<=4;i++){
//             printf("days: %i\n", days[i]);
//         }
        rvalue = last(&days);
    }
    return rvalue;
}

void read_zone(uint16_t location, tz_zone *zone){
    char temp[18];
    char *ptr = &temp[0];
    uint_farptr_t addr = &_binary_tz_zones_start;
    addr = addr+(location*18);
    memcpy_PF(&temp, addr, 18);
    zone->offset = *(int8_t *)ptr;
    ptr++;
    zone->rule = *(uint16_t *)ptr;
    ptr = ptr+2;
    strncpy(zone->format, ptr, 9);
    ptr = ptr+10;
    zone->until = *(uint32_t *)ptr;
    ptr = ptr+4;
    zone->flags = *ptr;
}

void read_rule(uint16_t location, tz_rule *rule){
    char temp[15];
    char *ptr = &temp;
    uint_farptr_t addr = &_binary_tz_rules_start;
    addr = addr+(location*15);
    memcpy_PF(&temp, addr, 15);
    rule->from = *(uint8_t *)ptr;
    ptr++;
    rule->to = *(uint8_t *)ptr;
    ptr++;
    rule->month = *(uint8_t *)ptr;
    ptr++;
    strncpy(rule->day, ptr, 9);
    ptr = ptr+8;
    rule->at = *(uint8_t *)ptr; //15 minute resolution
    ptr++;
    rule->save = *(int8_t *)ptr; //15 minute resolution
    ptr++;
    rule->letter = *(char *)ptr;
    ptr++;
    rule->flags = *ptr;
    rule->flags = rule->flags|0x01;
}

void print_rule(tz_rule *rule){
    printf("From: %u ", rule->from);
    printf("To: %u ", rule->to);
    printf("Month: %u ", rule->month);
    printf("Day: %s ", rule->day);
    printf("At: %u ", rule->at);
    printf("Save: %i ", rule->save);
    printf("Letter: %c ", rule->letter);
    printf("Flags: %u\n", rule->flags);
    decode_day(rule, 15);
}

void print_zone(tz_zone *zone){
    printf("Offset: %i ", zone->offset);
    printf("Rule: %hu ", zone->rule);
    printf("%s ", zone->format);
    printf("Until: %4lu ", zone->until);
    printf("Flags: %i\n", zone->flags);
}

const static uint8_t months[2][12] = {
    {31,28,31,30,31,30,31,31,30,31,30,31},
    {31,29,31,30,31,30,31,31,30,31,30,31}
};

#define secondsinyear 31556926

time_t time_subtract(time_t a, time_t b){
    time_t temp = a-b;
    if(temp > a){
        return 0;
    }
    return temp;
}

int get_dst(const time_t * timer, int32_t * z){
    uint16_t location = TZ;
    time_t t;
    t = *timer;
    struct tm tm_struct;
    gmtime_r(&sys_seconds, &tm_struct);
    uint8_t year = tm_struct.tm_year-100;
#ifdef DEBUG
    printf("Year: %u\n", year);
#endif
    tz_zone zone;
    tz_rule rule;
    tz_rule last_rule = (const tz_rule){ 0 };
    struct tm rule_time;
    time_t temp;
    time_t last_rule_time = 0;
    uint8_t flag = 1;
    while(flag){
        read_zone(location, &zone);
//         print_zone(&zone);
        if(zone.until >= t){
            flag = 0;
        }
        if((zone.flags&0x80)>>7){
            flag = 0;
        }
        location++;
    }
    location = zone.rule;
    t = t+(*z+LAST_DST); //Make UTC into localtime
    while(1){
        rule_time = (const struct tm){ 0 };
        read_rule(location, &rule);
        if(year >= rule.from){
#ifdef DEBUG1
            printf("Last Time %lu\n", last_rule_time);
             print_rule(&rule);
#endif
            if(rule.to <= year){
                rule_time.tm_year = rule.to+100;
            }
            else{
                rule_time.tm_year = year+100;
            }
            rule_time.tm_mon = rule.month;
            rule_time.tm_mday = decode_day(&rule, year);
#ifdef DEBUG1
            printf("%s\n", rule.day);
            printf("day %i\n", rule_time.tm_mday);
#endif
            rule_time.tm_hour = (rule.at*15)/60;
            rule_time.tm_min = (rule.at*15)%60;
            temp = mk_gmtime(&rule_time);
#ifdef DEBUG1
            printf("%lu\n", t);             
            printf("%lu\n", temp);
            printf("%lu\n", time_subtract(temp,secondsinyear));
#endif 
            if(last_rule_time == 0){
                if(rule.from < year){
                    last_rule_time = time_subtract(temp,secondsinyear);
                }
                else{
                    last_rule_time = temp;
                }
            }
#ifdef DEBUG1
            printf("%lu\n", last_rule_time);
#endif
            if(t >= time_subtract(temp,secondsinyear) && time_subtract(temp,secondsinyear) >= last_rule_time && rule.from < year){
//                      print_rule(&rule);
                last_rule_time = time_subtract(temp,secondsinyear);
                last_rule = rule;
            }
            if(t >= temp && temp >= last_rule_time){// && temp > last_rule_time+secondsinyear){
                last_rule_time = temp;
                last_rule = rule;
            }
            if(last_rule_time > t){
                read_rule(0, &last_rule);
            }
//          print_rule(&last_rule);
        }
        if((rule.flags&0x80)>>7){
            break;
        }
        location++;
    }
#ifdef DEBUG
    printf("Selected Rule ");
    print_rule(&last_rule);
#endif
    int dst = (last_rule.save*15)*60;
    if(dst != LAST_DST && LAST_DST_UPDATE+ONE_HOUR < sys_seconds){
        LAST_DST = dst;
        LAST_DST_UPDATE = sys_seconds;
    }
    return dst;
}

int32_t get_offset(time_t t){
    uint16_t location = TZ;
    tz_zone zone;
    uint8_t flag = 1;
    while(flag){
        read_zone(TZ, &zone);
        if(zone.until >= t){
            flag = 0;
        }
        if((zone.flags&0x80)>>7){
            flag = 0;
        }
        location++;
    }
    return (zone.offset*15)*60;
}

void get_name(char string[30], int count){
    uint_farptr_t ptr = &_binary_tz_names_start;
    uint16_t i;
    for(i=0;i<=count;i++){
        if(!i==0){
            ptr = ptr+(strlen_PF(ptr)+1);
        }
    }
    strncpy_PF(string, ptr, 29);
}

uint16_t zone_by_name(char *zone_name){
    uint_farptr_t ptr = &_binary_tz_offset_start;
    uint16_t count = (((uint16_t)&_binary_tz_offset_size)/2)-1;
    uint16_t i;
    char name[30];
    for(i=0;i<=count;i++){
        get_name(name, i);
        if(!strcmp(zone_name, name)){
            return pgm_read_word(ptr+(2*i));
        }
    }
    return 0;
}

void list_zones(void){
    uint16_t count = (((uint16_t)&_binary_tz_offset_size)/2)-1;
    uint16_t i;
    char name[30];
    for(i=0;i<=count;i++){
        get_name(name, i);
        printf("%s\n", name);
        if(i%10 == 0){
            _delay_ms(10);
        }
    }
}
            

void tz_init(void){
    TZ = 453;
    LAST_DST_UPDATE = 0;
    set_zone(get_offset(sys_seconds));
    set_dst(get_dst);
    get_dst(&sys_seconds, &__utc_offset);
}

// void test(void){
//     uint_farptr_t ptr = &_binary_tz_offset_start;
//     char name[30];
//     uint16_t i;
//     uint16_t count = (((uint16_t)&_binary_tz_offset_size)/2)-1;
//     tz_zone zone;
//     int flag = 1;
//     time_t seconds = 189406800;
//     TZ = 453;
//     printf("%i\n", get_offset(sys_seconds));
// //     for(i=0;i<=count;i++){
// //         get_name(name, i);
// //         TZ =  pgm_read_word(ptr+(2*i));
// // //         printf("%s:%i\n", name, get_offset(seconds));
// //         printf("%s: %u\n",name, TZ);
// // //         flag = 1;
// // //         read_zone(location, &zone);
// // //         print_zone(&zone);
// // //         while(flag){
// // //             read_zone(location, &zone);
// // //             print_zone(&zone);
// // //             if((zone.flags&0x80)>>7){
// // //                 flag = 0;
// // //             }
// // //             location++;
// // //             _delay_ms(20);
// // //         }
// //         _delay_ms(100);
// //     }
// }