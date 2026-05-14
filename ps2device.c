/*ps2device.c
 *this module emulates a ps2 AT type keyboard
 *signals (male 6 pin mini din)
 *
 *        /|__|\
 *       /5 [] 6\	
 *       |3    4|
 *       \1____2/
 *        
 *1 keyboard data
 *2 na
 *3 grd
 *4 +5V (300ma)
 *5 clk
 *6 na
 *
 *The sync serial frame format is as follows:
 *all interface lines open collector, keyboard responsible for pullups
 *data and clk quiescent state is high
 *11 bit frame: 1 start bit, 8 bit data lsb first, 1 bit parity (odd), stop bit
 *
 *start			lsb	data	msb  parity		stop
 *0 			x x x x x x x x   	x         1
 *
 *clock period aprox 80us
 *protocol: key press scan code, repeats code at typmatic rate, key release code
 *example: 'A' key is scan code 1C
 *frame 0 00111000 0 1
 *if using this module with external pu on clock and data, set quiescent state output_float()
 *internal pu may be used by using output_pull_up(), beware of transition issue from low to pull up state
 *
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>


#include "utility.h"
#include "ps2device.h"

// define io for port b pins pb3 and pb4
#define PS2PORT	PORTB
#define DATAP 	3
#define CLOCKP 	4

// file scope variable declarations
uint8_t lastSent = 0xFE;	// init as code to send if no code sent


/* define utility functions ********************************************************************/
// this function assumes that clock has been high for at least 5us, and exits with clock high
void sendBit(bool bitVal){	// this function manipulates the IO data and clock to send one bit
	delay_us(15);	
	if (bitVal)	// set data 
		output_pulled_up(DATAP);
	else
		output_low(DATAP);
	delay_us(15);	
	output_low(CLOCKP);				// toggle clock low to load data
	delay_us(38);
	output_pulled_up(CLOCKP);		// toggle clock high to setup next bit 
}

// get a data bit from host while driving clock, device reads while clock is high
// set ack argument true to drive the ack signal on the data pin (end of frame bit 10)
bool getBit(bool ack){
	bool bitVal;
	delay_us(15);
	bitVal=input(DATAP);			// read data value
	delay_us(15);		
	output_low(CLOCKP);				// toggle clock low 
	if (ack)						// if ack request (bitVal should also be high) 
		output_low(DATAP);			// drive data line low to ack 

	delay_us(38);
	output_pulled_up(CLOCKP);		// toggle clock high to setup next bit 
	if (ack){
		delay_us(10);
		output_pulled_up(DATAP);
	}
	return bitVal;
}


/*data	clock	ATBusStateType
 *	0	1		HOST_RTS=1
 *	1	0		INHIBIT=2
 *	1	1		IDLE=3
 */
enum ATBusStateType getBusState(){
	enum ATBusStateType state = 0;
	if(input(CLOCKP))
		bit_set(state,0);
	if(input(DATAP))
		bit_set(state,1);
	return state;
}

// decodes a scan code frame from the PC, returns true if successful
// passes 8 bit command byte by reference via function argument *codeRef
bool getScanCode(uint8_t *codeRef){
	bool ss;
	bool parity;
	bool good = true;
	uint8_t i;
	uint8_t code = 0;
	cli();									//disable interrupts
	ss=getBit(false);						//start bit
	if(ss==true) good=false;
	for(i=0;i<8;i++){						//get each bit, lsb first
		if(getBit(false)) bit_set(code,i);
	}
	parity=getBit(false);					//parity
	getBit(true);							//send ack pulse
	sei();									//enable interrupts
	if(parity == calcParity(code))			//check parity
		good = false;						//parity check failed
	*codeRef = code;						//setup to return value by ref
	return good;							//return true if scancode is good
}



/* public function definitions ****************************************************************/
// initialize io lines
void device_init(void){
	output_pulled_up(CLOCKP);			//emulate hardwire pullup at powerup 
	output_pulled_up(DATAP);			//emulate hardwire pullup at powerup 
}

#if 0
// these fuctions are used to send scancode data to the PC
void keyPress(uint8_t key){
	keyDown(key);						// send key down sequence
	keyRelease(key);					// send key release sequence
}

void keyDown(uint8_t key){
	sendScanCode(key);					// send key code
}

void keyRelease(uint8_t key){
	sendScanCode (0xF0);				// release code byte 1
	sendScanCode (key);					// release code byte 2
}

void keyPressExt(uint8_t key){
	keyDownExt(key);					// send key down extended sequence
	keyReleaseExt(key);					// send key release sequence
}

void keyDownExt(uint8_t key){
	sendScanCode(0xE0);					// send extended code
	keyDown(key);						// send key down
}

void keyReleaseExt(uint8_t key){
	sendScanCode(0xE0);					// send extended code
	keyRelease(key);					// send key release
}

void sendLastCode(void){
	sendScanCode(lastSent);
}
#endif

//sends a scan code frame to the PC
void sendScanCode(uint8_t code){
	bool parity;
	uint8_t i=0;
	parity = !(calcParity(code));

	do{									// loop until bus in idle state and stable
		if(getBusState() == IDLE)
			++i;						// increment count
		else
			i=0;						// reset count
	}while (i<5);						// loop until bus is idle for at leat 100 us
	
	cli();								// disable interrupts
	sendBit(0);							// start bit
	for(i=0;i<8;i++){					// send each bit, lsb first
		sendBit(bit_test(code,i));
	}
	sendBit(parity);					// send parity
	sendBit(1);							// stop bit
	sei();								// enable interrupts
	lastSent = code;

}


// this function waits for the rst bus condition, gets a code, verifies it, and if not valid, retries
// returns a valid 8 bit scan code 
uint8_t receiveScanCode(void){
	uint8_t code;
	uint8_t i=0;
	bool good;
	do{
		do{
			if(getBusState() == HOST_RTS)
				i++;					// increment count
			else
				i=0;					// reset count
		}while (i<4);					// loop until bus is RTS for at leat 50 us

		good=getScanCode(&code);

		if (good)
			break;						// break out of loop
		delay_ms(1);
		sendScanCode(0xFE);				// send retry command 
	}while(1);

	
	return code;
}


