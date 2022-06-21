#ifndef __FDRS_TYPES_H__
#define __FDRS_TYPES_H__

typedef struct __attribute__((packed)) DataReading {
    float data;
    uint16_t id;
    uint8_t type;
} DataReading;


typedef struct{
    DataReading buffer[256];
    uint16_t len = 0;
    uint32_t time = 0;
} DataReadingBuffer_t;

#endif