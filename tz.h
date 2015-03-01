typedef struct{
    uint8_t from;
    uint8_t to;
    uint8_t  month;
    char     day[10];
    uint8_t  at; //15 minute resolution
    int8_t   save; //15 minute resolution
    char     letter;
    uint8_t  flags;
} tz_rule;

typedef struct{
    int8_t offset; //only accurate to 15 minutes
    uint16_t  rule; //Multiply by 17(length of rule) to get rule start position
    char     format[10];
    uint32_t until;
    uint8_t  flags;
} tz_zone;

void test(void);

void tz_init(void);

int32_t get_offset(time_t t);

uint16_t zone_by_name(char *zone_name);

uint16_t zone_by_hash(char *zone_name);

void list_zones(void);

uint16_t tz_update(char *zone_name);