#include <stdint.h>

uint32_t hash(char *str){
    uint32_t hash = 5381;
    int c;

    while((c = *str++)){
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}