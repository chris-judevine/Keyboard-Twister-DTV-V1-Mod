/*handler2.c
 * This module controls communications between the PC and attached keyboard 
 *
 */

#include <avr/io.h>
#include <stdbool.h>

#include <avr/pgmspace.h>

#include "utility.h"
#include "ps2device.h"
#include "ps2host.h"

//static bool passthrough = false; // pass through communications to attached keyboard

#if 0
// returns true if keyboard sends POST before timeout, otherwise send POST locally
bool resetHandler(void)
{
 uint8_t i=200;

 do
 {
  delay_ms(3); // look for a keyboard POST response every 3 ms
  if(AThit()) return true; // keyboard is connected, let keyboardHandler process it
 } while(--i); // POST must be sent between 500-750ms after powerup (3x240 = 720ms)

 // timed out, so  
 sendScanCode(0xAA); // send POST
 return false;
}
#endif


#if 0
// this function returns true if the keyboard has sent a code,
// the code is passed by reference via the function 
// argment pointer passed by the caller
bool checkKeyboardComs(uint8_t *scanCodePtr)
{
 bool result=false;

 if (AThit())
 {
  inhibitBus(true); // prevent keyboard transmit while decoding current command 

  if(getATCode(scanCodePtr)) // keyboard sent a code?
  {
   passthrough = true; // then passthrough commands
   sendScanCode(*scanCodePtr); // send the code to the pc
   result = true;
  } // end of if
  inhibitBus(false); // enable keyboard transmit

 } // end of if

 return result;
}
#endif


// This function handles comms from the pc to keyboard, or handles
// the protocol locally if no keyboard is present.
void processPCtoKeyboard(void)
{
 static uint8_t hrtsCount = 3 ; // host request to send count
// static uint8_t CodeSet= 2 ; // default to scan code set 2
 static bool inh = false;
 uint8_t scanCode;
// uint8_t temp;
 bool exitdo;

 // check what the PC is doing to the bus, and handle requests

 do // loop in host_rts (request to send) state
 {
  exitdo = true;

  switch (getBusState())
  {
   case HOST_RTS: // host request to send
            if(--hrtsCount) // ensure rts for some time by count down
            {   
             exitdo=false;
            }
            else // count down complete, process rts
            {
             scanCode=receiveScanCode(); // get the scan code

//             if (passthrough) // if keyboard is attached
//             {
              inhibitBus(true); // bus is released after sendATCommand
#if 0
              if(scanCode == 0xFF) // reset passthrough upon software reset req from pc
              {
               passthrough = false; // if keyboard responds, passthrough will enable
              }
#endif
              sendATCommand(scanCode); // relay PC message to attached keyboard
//             } // end if(passthrough)

#if 0
             else // handle pc communications locally, because keyboard is detached
             {
              if (scanCode < 0xED)
              {
               sendScanCode(0xFE); // invalid pc code
              }
              else
              {
               switch (scanCode)
               {
                case (0xEE): // echo
                         sendScanCode(0xEE); // send echo
                         break;
                case (0xED): // set LED's
                case (0xF3): // set typematic rate
                         sendScanCode(0xFA); // ack PC
                         temp = receiveScanCode(); // get the second byte and do nothing
                         sendScanCode(0xFA); // ack PC
                         break;
                case (0xF0): // set scan code set
                         sendScanCode(0xFA); // ack
                         temp=receiveScanCode(); // get the second byte
                         sendScanCode(0xFA); // ack
                         if(!temp) // byte = 0
                         {
                          sendScanCode(CodeSet); // return current code set
                         }
                         break;
                case (0xF2): // device id 
                         sendScanCode(0xFA); // ack
                         sendScanCode(0xAB); // id byte 1
                         sendScanCode(0x83); // id byte 2
                         break;
                case (0xFE): // resend last 
                         sendLastCode();
                         break;
                case (0xFF): // reset
                         sendATCommand(scanCode); // send reset command to kbd, but assume it wont respond
                         sendScanCode(0xFA);
                         resetHandler();
                         break;
                default:
                         sendScanCode(0xFA); // ack PC
                         break;
               } // end switch
              } // end if
             } // end if
#endif

            } // end if
            break;
   case INHIBIT:
            if (!inh)
            {
             inhibitBus(true);
             inh=true;
            }
            break;
   case IDLE:
            if (inh)
            {
             inhibitBus(false);
             inh=false;
            }
            break;

  } // end switch

 } while (!(exitdo)); // loop for some time while bus is rts state

}
