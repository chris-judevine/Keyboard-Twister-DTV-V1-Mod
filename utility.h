/* utility.h
 * this file contains general purpose utility functions
 * such as for seting port pins, ect.
 *
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <avr/io.h>
#include <stdbool.h>


// define utility macros
//bit set macro
#define bit_set(addr,bit) ((addr) |= _BV((bit)))

//bit clear macro
#define bit_clear(addr,bit) ((addr) &= ~_BV((bit)))

//return a bit given an address and bit position macro
#define bit_test(addr,bit) (((addr) & _BV((bit)))?true:false)


//uncomment for parts with one IO port (tiny)
#define PORT PORTB


#ifndef PORT
// example usage output_float(&PORTD,PD2);	set port d pin 2 to floating (Hi Z)
void output_high(volatile uint8_t *port, uint8_t pin);	//set an io high

void output_low(volatile uint8_t *port, uint8_t pin);	//set an io low

void output_toggle(volatile uint8_t *port, uint8_t pin);	//invert io output

void output_float(volatile uint8_t *port, uint8_t pin);	//set an io to hi-Z

void output_pulled_up(volatile uint8_t *port, uint8_t pin);		//output pulled up 

bool input(volatile uint8_t *port, uint8_t pin);		//get pin input

#else

// example usage output_float(&PORTD,PD2);	set port d pin 2 to floating (Hi Z)
void output_high(uint8_t pin);	//set an io high

void output_low(uint8_t pin);	//set an io low

void output_toggle(uint8_t pin);	//invert io output

void output_float(uint8_t pin);	//set an io to hi-Z

void output_pulled_up(uint8_t pin);		//output pulled up 

bool input(uint8_t pin);		//get pin input
#endif

bool calcParity(uint8_t arg);	//calculate parity of argument (1=odd, 0=even);

//this function returns the hex digit of the least sig nibble of the passed argument
char toHexChar(uint8_t arg);

//call for long delays greater than what _delay_ms can support, note integer argument
void delay_ms(uint16_t arg);

//call for long delays greater than what _delay_ms can support, note integer argument
void delay_us(uint16_t arg);

#endif
