###############################################################################
# Makefile for the project irKeyboard
###############################################################################

## General Flags
PROJECT = irKeyboard
MCU = attiny45
AVRDUDE = avrdude -c usbasp -p $(MCU)
TARGET = keyboardtwister.elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2                            -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=keyboardtwister.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = scancon.o ps2device.o ps2host.o utility.o handler2.o 

flash:	all
	$(AVRDUDE) -U flash:w:keyboardtwister.hex:i

fuse:
	$(AVRDUDE) -U lfuse:w:0xe2:m -U hfuse:w:0x5f:m -U efuse:w:0x01:m 





## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) keyboardtwister.hex keyboardtwister.eep keyboardtwister.lss size

## Compile
scancon.o: scancon.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ps2device.o: ps2device.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ps2host.o: ps2host.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

utility.o: utility.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

handler2.o: handler2.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) keyboardtwister.elf dep/* keyboardtwister.hex keyboardtwister.eep keyboardtwister.lss keyboardtwister.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

