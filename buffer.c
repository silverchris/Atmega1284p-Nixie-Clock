/*Implementation shamelessly borrowed from
http://en.wikipedia.org/wiki/Circular_buffer */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "buffer.h"
 
void cbInit(CircularBuffer *cb, int8_t size) {
    cb->size  = size + 1; /* include empty elem */
    cb->start = 0;
    cb->end   = 0;
    cb->elems = (char *)calloc(cb->size, sizeof(char));
}
 
void cbFree(CircularBuffer *cb) {
    free(cb->elems); /* OK if null */
}
 
int8_t cbIsFull(CircularBuffer *cb) {
    return (cb->end + 1) % cb->size == cb->start;
}
 
int8_t cbIsEmpty(CircularBuffer *cb) {
    return (cb->end == cb->start);
}
 
/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking cbIsFull(). */
void cbWrite(CircularBuffer *cb, char *elem) {
    cb->elems[cb->end] = *elem;
    cb->end = (cb->end + 1) % cb->size;
    if (cb->end == cb->start)
        cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
}
 
/* Read oldest element. App must ensure !cbIsEmpty() first. */
void cbRead(CircularBuffer *cb, char *elem) {
    *elem = cb->elems[cb->start];
    cb->start = (cb->start + 1) % cb->size;
}