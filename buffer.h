typedef struct {
    int8_t         size;   /* maximum number of elements           */
    int8_t         start;  /* index of oldest element              */
    int8_t         end;    /* index at which to write new element  */
    char        *elems;  /* vector of elements                   */
} CircularBuffer;

void cbInit(CircularBuffer *cb, int8_t size);

void cbFree(CircularBuffer *cb);

int8_t cbIsFull(CircularBuffer *cb);

int8_t cbIsEmpty(CircularBuffer *cb);

void cbWrite(CircularBuffer *cb, char *elem);

void cbRead(CircularBuffer *cb, char *elem);