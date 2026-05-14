/*

Keyboard-Twister

Rudolph Riedel aka Shadowolf/Paralyze
Some changes by 1570


scancon.c

An on-the-fly scan-code exchanger for PS/2 keyboards
based on "irKeyboard", circuit-cellar
"Atmel AVR Design Contest 2006" Entry # AT3296 by Steven Savage. 

The motivation behind this was to make use of european keyboards
with the C64DTV which uses an US layout even in the PAL version.


V0.1 2007-05-06: first test-release in keyboard-twister thread on forum64.de
V0.2 2007-05-13: swapped left and right shift, initial 0xaa is supressed,
                 added fix for F7, ALT-GR no longer acts as SHIFT-Commodore
V0.3 2007-05-21: finished ALT-GR/Q -> @ transcoding
V0.4 2007-05-27: added ALT-GR/8 -> [ and ALT-GR/9 -> ]
                 cleanup, ok, somewhat..
V0.5 2008-01-02: Shift was borked after '@', '+' and '#'.
V0.6 2008-01-02: More delays. Less trouble with '/'.
V0.7 2008-01-06: Changed sending of key combinations. Should be a bit more robust.

*/

// http://www.computer-engineering.org/ps2keyboard/scancodes2.html

#include <avr/interrupt.h>

#include "ps2device.h"
#include "ps2host.h"
#include "application.h"
#include "handler2.h"
#include "avr\io.h"


#define JOY_UP 0 // define joy up as pb0

void sendBytes(char *b){
 while(b[0]!=0)
 {
  sendScanCode(b[0]);
  delay_ms(15);
  b++;
 }
}


// this is the main application that runs continuously and provides high level functionality 
int main(void)
{
uint8_t shift=0;    // indicator if SHIFT was pressed
uint8_t brk=0;      // indicator if break-code was sent
uint8_t altgr=0;      // indicator if ALTGR was pressed
uint8_t suppress=0; // indicator if sending scan-code out is to be avoided
uint8_t keyCode;


volatile bool JOY_PRESS=false; //tracks if joy up is pressed 
output_pulled_up(JOY_UP); // set Joy up as input and turn on pull up
PORTB&= ~_BV(PB5);//PORTB pin5 low
DDRB|=_BV(PB5);//set PORTB pin5 to one as output

device_init(); // initialize keyboard emulation hardware
host_init();   // initialize pc host hardware
sei();         // enable interrupts
delay_ms(900); // give dtv some time to boot

while(1) // just wait untill the keyboard responds
{
 delay_ms(3); // look for a keyboard POST response every 3 ms
 if(AThit()) break; // keyboard is connected, continue
}

// endless loop
while(1)
{
 processPCtoKeyboard(); // check pc to keyboard (or handle if no attached keyboard) coms

 if ((!(input(JOY_UP)))&& (!(JOY_PRESS)))// Read joy up pin if  pressed 
  {
   sendScanCode(0x66); // delete
   PORTB|=_BV(PB5);//PORTB pin5 high
   JOY_PRESS=true;
  }
 if ((input(JOY_UP)) && (JOY_PRESS))// Read joy up pin if released
  {
   sendScanCode(0xF0); // release code byte 1
   sendScanCode(0x66); // release code
   PORTB&=~_BV(PB5);//PORTB pin5 low
   JOY_PRESS=false;
  }
 if (AThit())
 {
  inhibitBus(true); // prevent keyboard transmit while decoding current command 
  if(getATCode(&keyCode)) // keyboard sent a code?
  {
   switch (keyCode)
   {
    case 0xaa: // power-up status, might confuse the dtv
              sendBytes("\x42"); // send k to dtv so it goes to basic
		  delay_ms(1000); 
		  sendBytes("\xf0\x42");  // send release code for k
              delay_ms(15);       // give DTV some time
              sendBytes("\x66");  // send back space
              delay_ms(15);       // give DTV some time
		  sendBytes("\xf0\x66"); // send release code back space
              suppress = 1;
              brk = 0;
              break;

    case 0x1e: // key for '2' - SHIFT-2 is " on german keyboards
              if(shift == 1)
              {
               keyCode = 0x52; // output '"' on SHIFT-2
              }
              brk = 0;
              break;

    case 0x36: // key for '6' - SHIFT-6 is & on german keyboards
              if(shift == 1)
              {
               keyCode = 0x3d; // output '&' on SHIFT-6
              }
              brk = 0;
              break;


    case 0x3e: // key for '8'
              if(shift == 1)
              {
               keyCode = 0x46; // output '(' on SHIFT-8
              }
              if(altgr == 1)
              {
               keyCode = 0x54; // output '[' on ALTGR-8
              }
              brk = 0;
              break;

    case 0x46: // key for '9'
              if(shift == 1)
              {
               keyCode = 0x45; // output ')' on SHIFT-9
              }
              if(altgr == 1)
              {
               keyCode = 0x5b; // output ']' on ALTGR-9
              }
              brk = 0;
              break;

    case 0x83: // scan-code for F7
              sendScanCode(0xe0); // make this 0xe0 0x03 to fix
              keyCode = 0x03;     // a bug in the DTV
              brk = 0;
              break;

    case 0xf0: // break-code
              brk = 1;
              break;

    case 0x12: // lshift
              keyCode = 0x59; // make this the correct l-shift for C64
              if(brk == 1)
              {
               shift = 0;
              }
              else
              {
               shift = 1;
              }
              brk = 0;
              break;

    case 0x58: // shift-lock
              if(brk == 1)
              {
               shift = 0;
              }
              else
              {
               shift = 1;
              }
              brk = 0;
              break;

    case 0x59: // rshift
              keyCode = 0x12; // make this the correct r-shift for C64
              if(brk == 1)
              {
               shift = 0;
              }
              else
              {
               shift = 1;
              }
              brk = 0;
              break;

    case 0xe0: // we have a combined key-code
               inhibitBus(false); // enable keyboard transmit
               while(!AThit()); // wait for next code to finish
               inhibitBus(true); // prevent keyboard transmit while decoding current command 
               getATCode(&keyCode); // get next code

               if(keyCode == 0xf0) // we have a break-code
               {
                brk = 1;
                inhibitBus(false); // enable keyboard transmit
                while(!AThit()); // wait for next code to finish
                inhibitBus(true); // prevent keyboard transmit while decoding current command 
                getATCode(&keyCode); // get next code
               }

               if(keyCode == 0x11) // ALTGR
               {
                if(brk == 1)
                {
                 altgr = 0;
                }
                else
                {
                 altgr = 1;
                }
                suppress = 1;
                brk = 0;
                break;
               }

               sendScanCode(0xe0); // send special make
               if(brk == 1)
               {
                sendScanCode(0xf0); // send break-code
               }
               delay_ms(15);       // give DTV some time

               brk = 0;
               break;

    default:
              suppress = 0;
              brk = 0;
              break;
   }

   if(suppress == 0)
   {
    sendScanCode(keyCode); // send the code to the pc
   }
   else
   {
    suppress = 0; 
   }

  } // end of if

 inhibitBus(false); // enable keyboard transmit
 } // end of if

}//end while

}
