//  To configure FDRS:
 
//   Each device in the system has a unique, one-byte address which 
//    is assigned to the last digit of its MAC address at startup.

//   Each relay receives data from its pre-programmed "PREV_MAC" device and
//    sends the packet verbatim to the address corresponding to "NEXT_MAC".

//   The gateway receives the data and outputs it as a json string over the serial port. 


// THIS UNIT
#define UNIT_MAC 0x01

// NEXT UNIT
#define NEXT_MAC 0x00 

// PREVIOUS UNIT
#define PREV_MAC 0x02 

//Packets from unknown MACs are sent here.
#define DEFT_MAC NEXT_MAC 
