#ifndef PS2DEVICE_H
#define PS2DEVICE_H

#include <avr/io.h>
#include <stdbool.h>


/*data	clock
 *	0	1	Host RTS
 *	1	0	Inhibit
 *	1	1	Idle
 */

enum ATBusStateType{
	HOST_RTS = 1,
	INHIBIT = 2,
	IDLE = 3
	};

void device_init(void);


//this function sends a key press sequence for a given (non extended) key
void keyPress(uint8_t key);

//this function sends a key down
void keyDown(uint8_t key);

//this function sends a key released
void keyRelease(uint8_t key);

//this function sends a key press sequence for a given extended key
void keyPressExt(uint8_t key);

//this function sends a key down (extended)
void keyDownExt(uint8_t key);

//this function sends a key released (extended)
void keyReleaseExt(uint8_t key);


//this function resends the last code sent to the host
void sendLastCode();

//this function sends a scan code to the host
void sendScanCode(uint8_t code);

//this function detects that the host is requesting to send, this is signaled by a data pulled low, clock high
//condition.  The function only returns true if this condition is recorded on 3 consecutive calls.	
bool hostRequestDet(void);

//this fuction is called after request to send detect to retrieve host command
uint8_t receiveScanCode(void);

enum ATBusStateType getBusState();

//this function should be called periodically to sense host rts and handle response internally
void keyboardHandler(void);

//this function should be called periodically to sense host rts and passes codes to attached keyboard
void keyboardPassthrough(void);

#endif
