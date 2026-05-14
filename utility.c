/* utility.c
 *this file contains general purpose utilities such as setting port pins
 *
 */

#include "utility.h"
#include <util/parity.h>


#ifndef PORT
//if pin is hi-z, toggle ddr fist to transition through low, port first to transition through pull-up
void output_high(volatile uint8_t *port, uint8_t pin){	//set an io high
	volatile uint8_t *ddr = port-1;
	*port |= _BV(pin);	//update port reg first (transition through pull up)
	*ddr |= _BV(pin);	//update ddr reg
}

//if pin is pulled up, toggle ddr fist to transition through high, port first to transition through hi-z
void output_low(volatile uint8_t *port, uint8_t pin){		//set an io low
	volatile uint8_t * ddr=port-1;
	*ddr |=_BV(pin);
	*port &= ~_BV(pin);
}

void output_toggle(volatile uint8_t *port, uint8_t pin){	//toggle io
	volatile uint8_t * ddr=port-1;
	*ddr |=_BV(pin);
	*port ^= _BV(pin);
}

//if pin is high, toggle ddr fist to transition through pull-up, port first to transition through low
void output_float(volatile uint8_t *port, uint8_t pin){		//set an io to hi-Z
	volatile uint8_t *ddr = port-1;
	*ddr &= ~_BV(pin);
	*port &= ~_BV(pin);
}

//if pin is low, toggle ddr first to transition through hi-z, port first to transition through high
void output_pulled_up(volatile uint8_t *port, uint8_t pin){		//output pulled up 
	volatile uint8_t *ddr = port-1;
	*ddr &= ~_BV(pin);
	*port |= _BV(pin);
}


bool input(volatile uint8_t *port, uint8_t pin){			//get pin input
	bool temp;
	volatile uint8_t *ddr = port-1;
	volatile uint8_t *pinr = port-2;
	*ddr &= ~ _BV(pin);				//set direction to input
	temp = (*pinr & _BV(pin));
	return temp;
}

#else
#define DDR	DDRB
#define PIN PINB

/**************** for parts with only one port, functions require only pin number ***************/

//if pin is hi-z, toggle ddr fist to transition through low, port first to transition through pull-up
void output_high(uint8_t pin){	//set an io high
	PORT |= _BV(pin);	//update port reg first (transition through pull up)
	DDR |= _BV(pin);	//update ddr reg
}

//if pin is pulled up, toggle ddr fist to transition through high, port first to transition through hi-z
void output_low(uint8_t pin){		//set an io low
	DDR |=_BV(pin);
	PORT &= ~_BV(pin);
}

//void output_toggle(uint8_t pin){	//toggle io
//	DDR |=_BV(pin);
//	PORT ^= _BV(pin);
//}

//if pin is high, toggle ddr fist to transition through pull-up, port first to transition through low
void output_float(uint8_t pin){		//set an io to hi-Z
	DDR &= ~_BV(pin);
	PORT &= ~_BV(pin);
}

//if pin is low, toggle ddr first to transition through hi-z, port first to transition through high
void output_pulled_up(uint8_t pin){		//output pulled up 
	DDR &= ~_BV(pin);
	PORT |= _BV(pin);
}


bool input(uint8_t pin){			//get pin input
	bool temp;
	DDR &= ~ _BV(pin);				//set direction to input
	temp = (PIN & _BV(pin));
	return temp;
}

#endif


// calculate parity of an 8 bit argument (1=odd, 0=even)
bool calcParity(uint8_t arg)
{
 return parity_even_bit(arg); // defined in util/parity.h 
}


#if 0
// this function returns the hex digit of the least sig nibble of the passed argument
char toHexChar(uint8_t arg){
	// 0x30 = '0', 0x41 = 'A'
	char retValue;
	arg &= 0x0F;	// mask off upper bits
	retValue = arg + 0x30;
	if (arg > 9)
		retValue += 7;	// if A-F, add 7 to get those chars
	return retValue;
}
#endif


// millisecond delay routine
void delay_ms(uint16_t arg)
{
 while(arg-- > 0)
 {
  delay_us(998); // assumes 2us loop overhead
 }
}


// 16 bit integer argument
// this MUST be modified if F osc != 8 MHz
void delay_us(uint16_t arg)
{
 do
 {
  __asm__ volatile ("NOP");
  __asm__ volatile ("NOP");
  __asm__ volatile ("NOP");
  __asm__ volatile ("NOP");
 } while(--arg); // 2 clocks for this loop overhead
}
