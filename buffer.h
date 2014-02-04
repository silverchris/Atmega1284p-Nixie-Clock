typedef struct {
    uint8_t         size;   /* maximum number of elements           */
    uint8_t         start;  /* index of oldest element              */
    uint8_t         end;    /* index at which to write new element  */
    char        *elems;  /* vector of elements                   */
} CircularBuffer;

void cbInit(CircularBuffer *cb, uint8_t size);

void cbFree(CircularBuffer *cb);

int8_t cbIsFull(CircularBuffer *cb);

#define cbIsEmpty(cb) (cb.end == cb.start)

void cbWrite(CircularBuffer *cb, char *elem);

void cbRead(CircularBuffer *cb, volatile char *elem);