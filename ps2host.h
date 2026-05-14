/*ps2host.h
this module decodes a ps2 AT type keyboard
signals (male 6 pin mini din)

        /|__|\
       /5 [] 6\	
       |3    4|
       \1____2/
        
1 keyboard data
2 na
3 grd
4 +5V (300ma max)
5 clk
6 na

The sync serial frame format is as follows:
all interface lines open collector, keyboard responsible for pullups
data and clk quiescent state is high
11 bit frame: 1 start bit, 8 bit data lsb first, 1 bit parity (odd), stop bit

start		lsb	data	msb  	parity		stop
0 			x x x x x x x x   	x         1

clock period aprox 80us
protocol: key press scan code, repeats code at typematic rate, key release code
example: A key is scan code 1C
frame 0 00111000 0 1
connect clock line to an external interrupt line 0=B0, 1=B1, 2=B2
connect data line to pin defined below
*/

#ifndef PS2HOST_H
#define PS2HOST_H

#include <avr/io.h>
#include <stdbool.h>


//function prototypes

//call this to initialize hardware, enable global interrupts in mainline code
void host_init(void);

//this function returns true if the keyboard has sent a scancode
bool AThit();


bool getATCode(uint8_t *retVal);	//returns true if decoded successfully, value returned by reference if passed argument
void sendATCommand(uint8_t cmd);
void inhibitBus(bool inh);			//set to inhibit coms 
#endif
