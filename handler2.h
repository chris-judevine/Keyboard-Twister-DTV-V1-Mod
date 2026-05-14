#ifndef HANDLER2_H
#define HANDLER2_H

bool keyboardAttached(void);

//returns true if keyboard sends POST before timeout
bool resetHandler(void);


//this function returns true if the keyboard has sent a code, the code is passed by reference via the function 
//argument pointer passed by the caller.  
bool checkKeyboardComs(uint8_t *scanCodePtr);

//This function handles comms from the pc to keyboard, or handles
//the protocol locally if no keyboard is present.
void processPCtoKeyboard(void);


#endif
