F_CPU = 1000000

CDEFS = -DF_CPU=$(F_CPU)UL

MMCU=attiny44
CC=avr-gcc
SIZE=avr-size
CFLAGS=-g -Os -Wall -mcall-prologues -mmcu=$(MMCU) $(CDEFS)
CPPFLAGS=$(CFLAGS) -x c++
OBJ2HEX=avr-objcopy
AVRD=avrdude
TARGET=main

build: main.hex size

program: build
	$(AVRD) -p $(MMCU) -c stk600 -P usb -v -v -U flash:w:$(TARGET).hex

firmware: $(TARGET).hex

%.obj: %.o
	$(CC) $(CFLAGS) $< -o $@

%.hex: %.obj
	$(OBJ2HEX) -R .eeprom -O ihex $< $@

%.o : %.c
	$(CC) -c $(CFLAGS) -std=c99 $< -o $@ 

%.o : %.cpp
	$(CC) -c $(CPPFLAGS) $< -o $@ 

clean:
	rm -f *.hex *.obj *.o *.elf

%.elf: %.obj
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

size: $(TARGET).elf
	$(SIZE) --mcu=$(MMCU) --format=avr $<

.PHONY: clean build program size