DEVICE = atmega328p
CLOCK = 1000000
PROGRAMMER = usbtiny
MAIN = main

all: burn

%.o: %.c
	avr-gcc -g -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -o $@ $<

%.hex: %.o
	avr-objcopy -j .text -j .data -O ihex $< $@

burn: $(MAIN).hex
	avrdude -c $(PROGRAMMER) -p $(DEVICE) -U flash:w:$<

clean:
	rm *.hex *.o
