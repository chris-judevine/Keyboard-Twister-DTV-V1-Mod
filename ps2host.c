/*ps2host.c
 * this module decodes a ps2 AT type keyboard
 * signals (male 6 pin mini din)
 *
 *       /|__|\
 *      /5 [] 6\	
 *      |3    4|
 *      \1____2/
 *       
 * 1 keyboard data
 * 2 na
 * 3 grd
 * 4 +5V (300ma)
 * 5 clk
 * 6 na
 * 
 * The sync serial frame format is as follows:
 * all interface lines open collector, keyboard responsible for pullups
 * data and clk quiescent state is high
 * 11 bit frame: 1 start bit, 8 bit data lsb first, 1 bit parity (odd), stop bit
 * 
 * start		lsb	data	msb  	parity		stop
 * 0 			x x x x x x x x   	x         1
 * 
 * clock period aprox 80us
 * protocol: key press scan code, repeats code at typematic rate, key release code
 * example: A key is scan code 1C
 * frame 0 00111000 0 1
 * 
 * 
 * API:
 * interrupt handler builds the 11 bit frame.
 * int8 (ATStatus enum)  AThit() // call periodically to see if a frame has been received, return ATStatus enum
 * ATStatus enum
 * call AThit() to poll if a frame has been received.
 * next call AT_decode_frame() to decode the frame, returns an int8 ATerror (0
 * 
 * bit_test, bit_set .... are macros defined in utility.h
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>


#include "utility.h"
#include "ps2host.h"

//setup for timer 1
//TCNT1 is accumulated value, set to 0 to clear
//values to write into TCCR1 to turn timer on and off with appropriate settings
//0x07 = normal mode, divide by 64 (at 8 MHz, delay of 2.04ms)
//0x0B = normal mode, divide by 1024 (at 8 MHz, delay of 32.04ms)
#define t1_on_2() 	TCNT1 = 0; TCCR1=0x07
#define t1_on_32() 	TCNT1 = 0; TCCR1=0x0B
#define t1_off()  	TCCR1=0	

//pin change interrupt enable
//#define ir_on() 	bit_set(GIMSK,PCIE)
//#define ir_off()	bit_clear(GIMSK,PCIE)			


#define PS2PORT	PORTB
#define DATAP 1
//clock and interrupt should be same pin (external interrupt 0)
#define CLOCKP 2

	
union FrameCodeType
{
	uint16_t data;	//holds 11 bit frame
	uint8_t code;	//stores validated AT scan code (after decoding)
}ATFrame;

volatile bool ATh;					//tracks if a valid command has been received
volatile bool Transmit;				//tracks if transmitting or receiving
volatile uint8_t recBit;			//track the received bit number

//call this to initialize hardware, enable global interrupts in mainline code
void host_init(void){
	bit_set(MCUCR,ISC01);			//falling edge of int0 (PB2) generates interupt
	bit_set(GIMSK,INT0);			//enable external interrupt 0
	bit_set(TIMSK,TOIE1);			//enable timer 1 interrupt on overflow
	ATFrame.data=0;						
}

//timer 1 interrupt on overflow (transmit or receive timed out)
ISR(TIM1_OVF_vect){	
	t1_off();	
	recBit = 0;				//reset received bit counter
//	ir_on();				//enable ir interrupt
	Transmit = false;		//reset in transmit mem
//	inhibitBus(false);		//allow keyboard to transmit
}

//external interrupt 0 ISR (external keyboard clock line)
ISR(INT0_vect)	
{
	bool databit;
	if (!recBit){ 			//enable timer on first bit, have 2 ms to complete
		t1_on_2();			//timer on
		}
	if(Transmit){						//transmit byte
		if(recBit<9){ 					//edges 0-8, transmit data and parity bit
			databit=bit_test(ATFrame.data,recBit);
			if (databit) 				//setup data for next falling edge
				output_float(DATAP);				
			else
				output_low(DATAP);
		}
		else{							//edges 9 and 10, stop and ack from device
			output_float(DATAP);
			if ((recBit)==10){			//entire frame transmitted
				Transmit = false;
				t1_off();				//timer off
				recBit = 0xFF;			//setup bit count for next pass
				while (input(DATAP));	//wait for data to go lo (ack from device)

//				ir_on();				//enable IR remote  						
				while (!(input(DATAP)) && !(input(CLOCKP)));	//block until lines released
			}		
		
		}						
		++recBit;

	}
	else{			//receive byte
		if (ATh) {
			return;							//don't process another receive until last command is read
		}

//		ir_off();							//disable external interrupt 0 (IR remote)
  						
		databit = input(DATAP);	//get bit data from IO pin
		if (databit) 
			bit_set(ATFrame.data,recBit);
		else
			bit_clear(ATFrame.data,recBit);
		if ((recBit)==10){					//entire frame received
				t1_off();					//timer off
//				ir_on();					//enable IR remote  						
				ATh = true;					//set AT hit flag (received scan code)
				recBit = 0xFF;				//setup bit count for next pass

		}
		++recBit;

	}//end of else
}

//this function returns true if the keyboard has sent a scancode
bool AThit(){
	return ATh;
}


//this function returns true if decoded successfully, value returned by reference
bool getATCode(uint8_t *retVal){	
	bool parity;
	bool result=false;
	
	if(ATh){
		//decode frame (bit#) disc: (0)start (1-8)data (9) parity (10)stop
		//good frame: start bit =0, stop = 1
		//if sb=1 or stop=0 return with false
		if (!(bit_test(ATFrame.data,0)) && bit_test(ATFrame.data,10)){
			ATFrame.data = ATFrame.data>>1; //shift data bits (2) into lower byte (which is same location as ATcode	
			parity=calcParity(ATFrame.code);
			if (parity != bit_test(ATFrame.data,8)){	//8th bit of frame is parity bit
				*retVal = ATFrame.code;
				result = true;
			}
		}
		ATh=false;
	}
	return result;
}


//this function send a PC->keyboard command to the keyboard (such as turn on led's)
void sendATCommand(uint8_t cmd){
	while (Transmit);
	while (!(input(DATAP)) && !(input(CLOCKP)));	//block if either pin is low or currently transmitting			

//	ir_off();						//disable IR remote
  		
	inhibitBus(true);				//inhibit transmission
	delay_us(100);
	output_low(DATAP);				//pull down data pin
	
	ATFrame.code=cmd;
	//calc parity bit
	if (!(calcParity(cmd)))			//if 1, the code is odd 
		bit_set(ATFrame.data,8);	//set the parity bit	
	else
		bit_clear(ATFrame.data,8);	//clear the parity bit
		
	Transmit=true;					//data transmit handled in interrupt routine
	t1_on_32();						//transmit over time timer on

	inhibitBus(false);				//release clock
}


//this function inhibits bus communications by driving the clock line low
void inhibitBus(bool inh){				
	if (recBit!=0) return;

	if (inh){
		bit_clear(GIMSK,INT0);			//disable interrupt 0
		output_low(CLOCKP);				//drive clock low to inhibit coms
	}
	else{
		GIFR = _BV(INTF0);				//clear interrupt flag bit (write a 1) to prevent ISR entry upon interupt enable
		bit_set(GIMSK,INT0);			//enable interrupt 1, must enable global interupts in mainline
		output_float(CLOCKP);			//release clock to allow coms
	}
}
