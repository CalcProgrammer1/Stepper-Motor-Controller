###############################################################################
# Makefile for the project L297-L298-Stepper-Driver
###############################################################################

## General Flags
PROJECT = L297-L298-Stepper-Driver
MCU = attiny2313
TARGET = L297-L298-Stepper-Driver.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=L297-L298-Stepper-Driver.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = L297-L298-Stepper-Driver.o serial.o usi_i2c_master.o USI_TWI_Master.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) L297-L298-Stepper-Driver.hex L297-L298-Stepper-Driver.eep L297-L298-Stepper-Driver.lss size

## Compile
L297-L298-Stepper-Driver.o: ../L297-L298-Stepper-Driver.c
	$(CPP) $(INCLUDES) $(CFLAGS) -c  $<

serial.o: ../serial.c
	$(CPP) $(INCLUDES) $(CFLAGS) -c  $<

usi_i2c_master.o: ../usi_i2c_master.c
	$(CPP) $(INCLUDES) $(CFLAGS) -c  $<

USI_TWI_Master.o: ../USI_TWI_Master.cpp
	$(CPP) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CPP) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

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
	-rm -rf $(OBJECTS) L297-L298-Stepper-Driver.elf dep/* L297-L298-Stepper-Driver.hex L297-L298-Stepper-Driver.eep L297-L298-Stepper-Driver.lss L297-L298-Stepper-Driver.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

