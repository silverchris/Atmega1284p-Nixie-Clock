#ifndef _SYSCLK_H_
#define _SYSCLK_H_
#include <time.h>

void sysclk_setup(void);
time_t sys_seconds;
void sysclk_adj(int16_t adj);

#endif