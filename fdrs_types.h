#ifndef __FDRS_TYPES_H__
#define __FDRS_TYPES_H__

#include <stdint.h>
#include <string.h>

typedef struct __attribute__((packed)) DataReading_t {
    float data;
    uint16_t id;
    uint8_t type;
    DataReading_t(): data(0.0),id(0),type(0){
    }
} DataReading_t;


typedef struct DataReadingBuffer_t{
    DataReading_t buffer[256];
    uint16_t len;

    DataReadingBuffer_t(){
        
    }

}DataReadingBuffer_t;


typedef struct Peer_t{

    uint8_t peer[6];
    Peer_t(){
        memset(peer,0,6);
    }

    void _copy(uint8_t p[6]){
        memcpy(peer,p,6);
    }
    uint8_t *_data(void){
        return peer;
    }

    bool operator==(Peer_t c){
        if(memcmp(this->peer,c.peer,6) == 0){
            return true;
        }
        return false;
    }

}Peer_t;


#endif