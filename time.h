#ifndef _TIME_H
#define _TIME_H
typedef uint32_t time_t;

typedef struct{
    uint8_t tm_sec;
    uint8_t tm_min;
    uint8_t tm_hour;
    uint8_t tm_mday;
    uint8_t tm_mon;
    uint8_t tm_year;
    uint8_t tm_wday;
    int16_t tm_yday;
    uint8_t tm_isdst;
} tm;

tm *gmtime_r(time_t *, tm *);
time_t timegm(tm *);
void printtime(tm *);
void utc_digits(tm *, uint8_t *);

#define IsLeapYear(x)   ((x % 4 == 0) && (x % 100 != 0 || x % 400 == 0))
#endif