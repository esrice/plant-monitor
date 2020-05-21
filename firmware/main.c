#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TIMEOUT 32000
#define PULSES 41

/**** port definitions ****/
// LED for some simple feedback
#define LED_BIT PB3
#define LED_PORT PORTB
#define LED_DDR DDRB
// DHT22
#define DHT_BIT PC5
#define DHT_PORT PORTC
#define DHT_PIN PINC
#define DHT_DDR DDRC

volatile uint8_t i = 0;
volatile uint16_t count = 0;

void setup() {
    LED_DDR |= (1 << LED_BIT); // set LED pin as output
    DHT_DDR &= ~(1 << DHT_BIT); // start DHT data pin off as an input pin
    DHT_PORT |= (1 << DHT_BIT); // enable pull-up on DHT data pin
}

void blink_quickly() {
    int i;
    for (i = 0; i < 20; i++)
    {
        LED_PORT ^= (1 << LED_BIT);
        _delay_ms(100);
    }
}

void blink_slowly() {
    int i;
    for (i = 0; i < 10; i++)
    {
        LED_PORT ^= (1 << LED_BIT);
        _delay_ms(500);
    }
}

void blink_long() {
    LED_PORT |= (1 << LED_BIT);
    _delay_ms(1500);
    LED_PORT &= ~(1 << LED_BIT);
}

int get_temp() {
    uint16_t i; // loop counter
    uint16_t counts[PULSES*2] = {0};
    uint8_t humidity_int, humidity_dec, temperature_int, temperature_dec;
    uint8_t test, checksum;
    uint32_t sum, mean_low_pulse_count;
    uint64_t all_bits;

    // Send DHT22 the start signal
    DHT_DDR |= (1 << DHT_BIT); // set data pin to output mode
    DHT_PORT &= ~(1 << DHT_BIT); // drive data pin low
    _delay_ms(2); // give DHT22 a moment to notice the start signal

    // Receive the DHT22's "I'm about to transmit" signal
    DHT_DDR &= ~(1 << DHT_BIT); // set data pin to input mode
    DHT_PORT |= (1 << DHT_BIT); // enable pullup

    // wait for sensor to pull low
    for (i = 0; DHT_PIN & (1 << DHT_BIT); i++)
    {
        if (i > TIMEOUT) return -1;
    }

    // count the number of iterations each high and low pulse lasts
    for (i = 0; i < PULSES*2; i += 2)
    {
        while (!(DHT_PIN & (1 << DHT_BIT))) // even i is low
            if (++counts[i] > TIMEOUT) return -2;
        while (DHT_PIN & (1 << DHT_BIT)) // odd i is high
            if (++counts[i+1] > TIMEOUT) return -2;
    }

    // calculate the mean low pulse width
    sum = 0;
    for (i = 2; i < PULSES*2; i += 2) sum += counts[i];
    mean_low_pulse_count = sum / (PULSES - 1);

    // use the mean low pulse width as a threshold to determine whether each
    // high pulse is a 1 (long) or a 0 (short) and shift it into all_bits
    all_bits = 0;
    for (i = 0; i < PULSES; i++)
    {
        all_bits <<= 1;
        if (counts[i*2 + 1] >= mean_low_pulse_count)
            all_bits |= 1;
    }

    // shift the five values the sensor is giving us out of all_bits
    humidity_int = (all_bits >> 32) & 0xFF;
    humidity_dec = (all_bits >> 24) & 0xFF;
    temperature_int = (all_bits >> 16) & 0xFF;
    temperature_dec = (all_bits >> 8) & 0xFF;
    checksum = (all_bits >> 0) & 0xFF;

    // compare the checksum to the actual sum
    test = humidity_int + humidity_dec + temperature_int + temperature_dec;
    if ((test & 0xFF) == checksum) return 1;
    else return 0;
}

int connected() {
    DHT_DDR &= ~(1 << DHT_BIT); // set data pin to input mode
    DHT_PORT |= (1 << DHT_BIT); // enable pullup
    return (DHT_PIN & (1 << DHT_BIT));
}

int main() {
    setup();

    while (1)
    {
        int temp = get_temp();
        if (temp == 1) blink_quickly();
        else if (temp == -1) blink_slowly();
        else if (temp == -2) blink_long();
        _delay_ms(5000);
    }
}
