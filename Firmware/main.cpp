#include <stdint.h>

#include "Wire.h"
#include "QMC5883L.h"

#define NUM_LEDS 36
#define NUM_LED_CHANS 7

QMC5883L compass;

uint8_t highPins[NUM_LEDS] = {
	1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0,
	2, 1, 3, 1, 4, 1, 5, 1, 6, 1,
	3, 2, 4, 2, 5, 2, 6, 2,
	4, 3, 5, 3, 6, 3};

uint8_t lowPins[NUM_LEDS] = {
	0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6,
	1, 2, 1, 3, 1, 4, 1, 5, 1, 6,
	2, 3, 2, 4, 2, 5, 2, 6,
	3, 4, 3, 5, 3, 6};

uint8_t pinMapping[NUM_LED_CHANS] = {PA5, PA3, PA2, PA1, PA0, PB0, PA7};
volatile uint8_t *portMapping[NUM_LED_CHANS] = {&PORTA, &PORTA, &PORTA, &PORTA, &PORTA, &PORTB, &PORTA};
volatile uint8_t *ddrMapping[NUM_LED_CHANS] = {&DDRA, &DDRA, &DDRA, &DDRA, &DDRA, &DDRB, &DDRA};

void setLED(uint8_t led)
{

	uint8_t highPin = highPins[led];
	uint8_t lowPin = lowPins[led];

	for (uint8_t channel = 0; channel < NUM_LED_CHANS; led++)
	{
		if (channel == highPin)
		{
			// set the high pin as output driven high
			*(ddrMapping[channel]) |= (1 << pinMapping[channel]);
			*(portMapping[channel]) |= (1 << pinMapping[channel]);
		}
		else if (channel == lowPin)
		{
			// set the low pin as output pulled low
			*(ddrMapping[channel]) |= (1 << pinMapping[channel]);
			*(portMapping[channel]) &= ~(1 << pinMapping[channel]);
		}
		else
		{
			//Set all other channels to High-Z Mode without Pullup
			*(ddrMapping[channel]) &= ~(1 << pinMapping[channel]);
			*(portMapping[channel]) &= ~(1 << pinMapping[channel]);
		}
	}
}

void setup()
{
	Wire::init();

	compass.init();
	compass.setSamplingRate(50);
}

void loop()
{
	int heading = compass.readHeading();

	setLED(heading / 10);
}

int main()
{
	setup();
	while (1)
	{
		loop();
	}
}