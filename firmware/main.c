#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/**** DHT constants ****/
#define TIMEOUT 32000
#define PULSES 41

/**** commands from master ****/
#define CHECKSUM 1
#define TEMP_INT 2
#define TEMP_DEC 3
#define HUM_INT 4
#define HUM_DEC 5
#define MOISTURE_HIGH 6
#define MOISTURE_LOW 7

/**** port definitions ****/
// LED for some simple feedback
#define LED_BIT PD0
#define LED_PORT PORTD
#define LED_DDR DDRD
// DHT22
#define DHT_BIT PC5
#define DHT_PORT PORTC
#define DHT_PIN PINC
#define DHT_DDR DDRC
// soil sensor
#define SOIL_POWER_BIT PD1
#define SOIL_POWER_PORT PORTD
#define SOIL_POWER_DDR DDRD
#define SOIL_ADC 4
// SPI
#define DDR_SPI DDRB
#define DD_MISO PB4

typedef struct {
    uint8_t status;
    uint8_t checksum;
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temperature_int;
    uint8_t temperature_dec;
} dht_reading;

volatile uint8_t i = 0;
volatile uint16_t count = 0;
volatile dht_reading result;
volatile uint8_t moisture_high, moisture_low;
volatile uint8_t command;

void setup() {
    // set up LED
    LED_DDR |= (1 << LED_BIT); // set LED pin as output

    // set up DHT22 temperature/humidity sensor
    DHT_DDR &= ~(1 << DHT_BIT); // start DHT data pin off as an input pin
    DHT_PORT |= (1 << DHT_BIT); // enable pull-up on DHT data pin

    // set up soil sensor
    // set soil sensor pin as output and start it out low
    SOIL_POWER_DDR |= (1 << SOIL_POWER_BIT);
    SOIL_POWER_PORT &= ~(1 << SOIL_POWER_BIT);
    //DIDR0 |= (1 << ADC4D); // disable digital use of ADC5 to save power
    ADCSRA |= (1<<ADEN);
    ADMUX = (1<<REFS0); // set reference voltage to AVcc

    // set up SPI
    DDR_SPI = (1 << DD_MISO); // set MISO to output
    SPCR = (1 << SPIE) | (1 << SPE); // enable SPI in slave mode w/ interrupts

    // set up timer interrupt
    OCR1A = 0x231F; // set counter
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC and scale 1024
    TIMSK1 |= (1 << OCIE1A); // enable interrupt

    sei(); // enable interrupts
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

uint16_t read_ADC(uint8_t port) {
    //PRR &= ~(1 << PRADC); // turn off ADC power saving
    ADMUX = (ADMUX & 0xF0) | (port & 0x0F);
    ADCSRA |= (1 << ADEN) | (1 << ADSC); // start conversion
    while (ADCSRA & (1 << ADSC)); // wait until conversion is complete
    //PRR |= (1 << PRADC); // turn ADC power saving back on

    return ADC;
}

uint16_t get_soil_moisture() {
    SOIL_POWER_PORT |= (1 << SOIL_POWER_BIT); // turn soil sensor voltage on
    _delay_ms(10);
    uint16_t reading = read_ADC(SOIL_ADC);
    SOIL_POWER_PORT &= ~(1 << SOIL_POWER_BIT); // turn soil sensor voltage off
    return reading;
}

dht_reading get_temp() {
    uint16_t i; // loop counter
    uint16_t counts[PULSES*2] = {0};
    uint8_t humidity_int, humidity_dec, temperature_int, temperature_dec;
    uint8_t test, checksum;
    uint32_t sum, mean_low_pulse_count;
    uint64_t all_bits;
    dht_reading result;

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
        if (i > TIMEOUT)
        {
            result.status = -1;
            return result;
        }
    }

    // count the number of iterations each high and low pulse lasts
    for (i = 0; i < PULSES*2; i += 2)
    {
        while (!(DHT_PIN & (1 << DHT_BIT))) { // even i is low
            if (++counts[i] > TIMEOUT)
            {
                result.status = -1;
                return result;
            }
        }
        while (DHT_PIN & (1 << DHT_BIT)) { // odd i is high
            if (++counts[i+1] > TIMEOUT)
            {
                result.status = -2;
                return result;
            }
        }
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
    result.status = ((test & 0xFF) == checksum) ? 1 : 0;
    result.checksum = checksum;
    result.humidity_int = humidity_int;
    result.humidity_dec = humidity_dec;
    result.temperature_int = temperature_int;
    result.temperature_dec = temperature_dec;

    return result;
}

int connected() {
    DHT_DDR &= ~(1 << DHT_BIT); // set data pin to input mode
    DHT_PORT |= (1 << DHT_BIT); // enable pullup
    return (DHT_PIN & (1 << DHT_BIT));
}

int main() {
    setup();

    while (1) { }
}

// get temperature and store in global variale each timer cycle
ISR(TIMER1_COMPA_vect) {
    result = get_temp();

    uint16_t moisture = get_soil_moisture();
    moisture_high = moisture >> 8;
    moisture_low = moisture & 0xFF;

    blink_quickly();
}

// when slave select is triggered, send the requested infoz
ISR (SPI_STC_vect) {
    command = SPDR;
    switch (command) {
        case CHECKSUM: SPDR = result.checksum; break;
        case TEMP_INT: SPDR = result.temperature_int; break;
        case TEMP_DEC: SPDR = result.temperature_dec; break;
        case HUM_INT: SPDR = result.humidity_int; break;
        case HUM_DEC: SPDR = result.humidity_dec; break;
        case MOISTURE_HIGH: SPDR = moisture_high; break;
        case MOISTURE_LOW: SPDR = moisture_low; break;
    }
}
