//  To configure FDRS:

//   Uncomment the code corresponding to the unit you are configuring,
//    then uncomment the code corresponding to the unit you would like
//    to be previous and/or next in the line of communication.
 
//   Each device in the system has a unique, one-byte address which 
//    is assigned to the last digit of its MAC address at startup.

//   Each device is configured to know what the previous and/or next
//    device in the line of communication is.

//   The terminal is considered the "first" device, which can be addressed
//    to a relay or the gateway.

//   Each relay receives data from its pre-programmed "PREV_MAC" device and
//    sends the packet verbatim to the address corresponding to "NEXT_MAC".

//   The gateway receives the data and outputs it as a json string over the serial port. 

// THIS UNIT
#define UNIT_MAC 0x00 // Terminal
//#define UNIT_MAC 0x01 // Relay 0
//#define UNIT_MAC 0x02 // Relay 1
//#define UNIT_MAC 0x03 // Gateway
//#define UNIT_MAC 0x04 // 

// NEXT UNIT
//#define NEXT_MAC 0x00 // Terminal
#define NEXT_MAC 0x01 // Relay 0
//#define NEXT_MAC 0x02 // Relay 1
//#define NEXT_MAC 0x03 // Gateway
//#define NEXT_MAC 0x04 // 
