//  To configure FDRS:
//  
//  Uncomment the code corresponding to the unit you are configuring,
//   then uncomment the code corresponding to the unit you would like
//   to be previous and next in the line of communication.
//  Be sure that all unused lines are commented out. 

// THIS UNIT
//#define UNIT_MAC 0x00 // Terminal
//#define UNIT_MAC 0x01 // Relay 0
#define UNIT_MAC 0x02 // Relay 1
//#define UNIT_MAC 0x03 // Gateway
//#define UNIT_MAC 0x04 // 

// PREVIOUS UNIT
//#define PREV_MAC 0x00 // Terminal
#define PREV_MAC 0x01 // Relay 0
//#define PREV_MAC 0x02 // Relay 1
//#define PREV_MAC 0x03 // Gateway
//#define PREV_MAC 0x04 // 

// NEXT UNIT
//#define NEXT_MAC 0x00 // Terminal
//#define NEXT_MAC 0x01 // Relay 0
//#define NEXT_MAC 0x02 // Relay 1
#define NEXT_MAC 0x03 // Gateway
//#define NEXT_MAC 0x04 // 
