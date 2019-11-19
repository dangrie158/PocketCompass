#include <stdint.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include "Wire.h"
#include "QMC5883L.h"

#define NUM_LEDS 36
#define NUM_LED_CHANS 7
#define ALL_OFF -1

QMC5883L compass;

const uint8_t highPins[NUM_LEDS] = {
	1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0,
	2, 1, 3, 1, 4, 1, 5, 1, 6, 1,
	3, 2, 4, 2, 5, 2, 6, 2,
	4, 3, 5, 3, 6, 3};

const uint8_t lowPins[NUM_LEDS] = {
	0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6,
	1, 2, 1, 3, 1, 4, 1, 5, 1, 6,
	2, 3, 2, 4, 2, 5, 2, 6,
	3, 4, 3, 5, 3, 6};

const uint8_t pinMapping[NUM_LED_CHANS] = {PA5, PA3, PA2, PA1, PA0, PB0, PA7};
volatile uint8_t *portMapping[NUM_LED_CHANS] = {&PORTA, &PORTA, &PORTA, &PORTA, &PORTA, &PORTB, &PORTA};
volatile uint8_t *ddrMapping[NUM_LED_CHANS] = {&DDRA, &DDRA, &DDRA, &DDRA, &DDRA, &DDRB, &DDRA};

volatile uint8_t *switchDDR = &DDRB;
volatile uint8_t *switchOR = &PORTB;
volatile uint8_t *switchIR = &PINB;
uint8_t switchPin = PB2;
uint8_t switchInt = INT0;

const uint8_t quarterLeds[4] = {8, 17, 26, 35};

void setLED(int8_t led)
{

	uint8_t highPin, lowPin;
	if (led == ALL_OFF)
	{
		highPin = NUM_LEDS + 1;
		lowPin = NUM_LEDS + 1;
	}
	else
	{
		highPin = highPins[led];
		lowPin = lowPins[led];
	}

	//Set all channels to High-Z Mode without Pullup
	for (uint8_t channel = 0; channel < NUM_LED_CHANS; channel++)
	{
		*(ddrMapping[channel]) &= ~(1 << pinMapping[channel]);
		*(portMapping[channel]) &= ~(1 << pinMapping[channel]);
	}
	// set the high pin as output driven high
	*(ddrMapping[highPin]) |= (1 << pinMapping[highPin]);
	*(portMapping[highPin]) |= (1 << pinMapping[highPin]);

	// set the low pin as output pulled low
	*(ddrMapping[lowPin]) |= (1 << pinMapping[lowPin]);
	*(portMapping[lowPin]) &= ~(1 << pinMapping[lowPin]);
}

void setup()
{
	//Input with pull-up enabled for switch input
	*switchDDR &= ~(1 << switchPin);
	*switchOR |= (1 << switchPin);

	// enable interrupts
	sei();

	Wire::init();

	compass.init();
	compass.setSamplingRate(50);
	compass.setRange(QMC5883L_CONFIG_8GAUSS);
}

ISR(INT0_vect)
{
	//do nothing, only to wakeup
}

void loop()
{
	if ((*switchIR & (1 << switchPin)))
	{
		setLED(ALL_OFF);
		//enable level triggered interrupt on INT0
		GIMSK |= (1 << INT0);
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
	}

	// disable interrupt
	GIMSK &= ~(1 << INT0);

	for (uint8_t i = 0; i < 4; i++)
	{
		setLED(quarterLeds[i]);
		_delay_ms(1);
	}

	int heading = compass.readHeading();

	while (heading < 0)
	{
		heading += 360;
	}
	while (heading >= 360)
	{
		heading -= 360;
	}
	setLED((heading / 10) % 36);
	_delay_ms(3);
}

int main()
{
	setup();
	while (1)
	{
		loop();
	}
}
