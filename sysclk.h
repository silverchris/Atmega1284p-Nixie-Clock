#ifndef _SYSCLK_H_
#define _SYSCLK_H_
#include <time.h>

#define OCR1A_VAL 9999


void sysclk_setup(void);
time_t sys_seconds;

#endif