#ifndef __FDRS_TYPES_H__
#define __FDRS_TYPES_H__

#include <stdint.h>
#include <string.h>
#include <vector>

typedef struct __attribute__((packed)) DataReading_t {
    float data;
    uint16_t id;
    uint8_t type;
    DataReading_t(): data(0.0),id(0),type(0){
    }
} DataReading_t;

typedef struct Peer_t{

    uint8_t peer[6];
    std::vector<DataReading_t> buffer;
    Peer_t(){
        memset(peer,0,6);
        buffer.clear();
    }

    void _set_peer(uint8_t p[6]){
        memcpy(peer,p,6);
    }
    uint8_t *_get_peer(void){
        return peer;
    }

    bool operator==(Peer_t c){
        return (memcmp(this->peer,c.peer,6) == 0);
    }

}Peer_t;


#endif