//  To configure your FDRS Gateway:
 
//   Each device in the system has a unique, one-byte address which 
//    is assigned to the last digit of its MAC address at startup.

//   Each device is configured to know what the previous and/or next
//    device in the line of communication is.

//   The gateway should usually be assigned the address 0x00.
//   The PREV_MAC is currently not used, as the gateway responds
//     to all packets in the same manner.



// THIS UNIT
#define UNIT_MAC 0x00
#define PREV_MAC 0x01

//MAC prefix:
#define MAC_PREFIX 0xAA, 0xBB, 0xCC, 0xDD, 0xEE
