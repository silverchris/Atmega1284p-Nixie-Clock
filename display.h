#ifndef _DISPLAY_H_
#define _DISPLAY_H_

void display(int8_t *);
void display_init(void);
void utc_digits(struct tm *tm_struct, uint8_t array[]);
#endif