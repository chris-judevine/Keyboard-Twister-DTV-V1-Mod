<b>Keyboard Twister DTV V1 Mod</b><br> 

Keyboard Twister is an ATtiny45-based hardware interface designed to sit between a standard PS/2 keyboard and a C64 Direct-to-TV (DTV) unit. It corrects keyboard emulation issues, and includes custom boot macros to streamline the DTV experience.<br>

Features<br> 
•	Hardware Emulation Fixes: Resolves common scan code incompatibilities between modern keyboards and the DTV.<br> 
•	Auto-Boot to BASIC: Automatically sends the K scan code at startup, forcing the DTV to bypass the standard games menu and boot directly into Commodore BASIC.<br> 
•	Joystick Fix/Hack: Resolves the DTV V1 lack of a JOY 1 up input, spoofing it by sending the keycode for Delete and triggering up on JOY 2 at the same time.<br> 

Hardware Requirements<br> 
•	Microcontroller: ATtiny45<br> 
•	Programmer: STK200 (or any AVR ISP programmer like USBasp)<br> 
•	Target: C64 DTV (v2/v3)<br> 
Installation & Programming<br> 
1. Wiring
The ATtiny45 acts as a "Man-in-the-Middle" for the PS/2 signals.
•	Connect the Keyboard Clock/Data to the ATtiny inputs.
•	Connect the ATtiny outputs to the DTV keyboard Clock/Data pads.
2. Flashing the Firmware
The pre-compiled Intel Hex file is located in the main directory. If you are using Linux with an STK200 programmer on a parallel port, use the following command:
```bash
avrdude -p t45 -c stk200 -P /dev/parport0 -U flash:w:keyboardtwister.hex:i -U lfuse:w:0xE2:m
```
Note: The fuse setting 0xE2 is critical for correct timing as it sets the internal oscillator.<br> 
Development<br> 
The core logic of this fork modifies the standard Keyboard Twister behavior to automate DTV1-specific commands.<br> 
•	Boot Macro: Modified to send K immediately upon power-up.<br> 
•	Joy-Port Workaround: Modified to intercept Joy1 Up signals and inject the Delete scan code into the keyboard stream, and trigger Joy2 Up.<br> 
Credits & Resources<br> 
•	Original concept and Wiki: PicoBay DTV Wiki<br> 
http://picobay.com/dtv_wiki/index.php?title=Keyboard_Twister<br> 
•	Based on the ATtiny PS/2 translation framework.<br> 

