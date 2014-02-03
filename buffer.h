typedef struct {
    int16_t         size;   /* maximum number of elements           */
    int16_t         start;  /* index of oldest element              */
    int16_t         end;    /* index at which to write new element  */
    char        *elems;  /* vector of elements                   */
} CircularBuffer;

void cbInit(CircularBuffer *cb, int16_t size);

void cbFree(CircularBuffer *cb);

int8_t cbIsFull(CircularBuffer *cb);

int8_t cbIsEmpty(CircularBuffer *cb);

void cbWrite(CircularBuffer *cb, char *elem);

void cbRead(CircularBuffer *cb, char *elem);