#ifndef __FDRS_TYPES_H__
#define __FDRS_TYPES_H__

#include <stdint.h>

typedef struct __attribute__((packed)) DataReading_t {
    float data;
    uint16_t id;
    uint8_t type;
} DataReading_t;

#endif