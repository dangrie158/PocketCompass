#!/usr/bin/env python3

import math

NUM_LEDS = 36
RADIUS = 15
START_ANGLE = 0
CENTER = RADIUS + 2, RADIUS + 2
ATTRIBUTES = {	'RED' : {'JLC': 'KT-0603R', 'LCSC': 'C2286'},
		'YELLOW': {'JLC': '19-213/Y2C-CQ2R2L/3T(CY)', 'LCSC': 'C72038'}
}

print('CHANGE DISPLAY OFF;')
for led in range(1, NUM_LEDS + 1):
	angle = START_ANGLE + led * (360 // NUM_LEDS)
	if led % 2 == 0:
		angle += 180
	theta = (math.pi * 2) * (led / NUM_LEDS)
	x, y = RADIUS * math.cos(theta) + CENTER[0], RADIUS * math.sin(theta) + CENTER[1]
	color = 'YELLOW' if led % (NUM_LEDS // 4) == 0 else 'RED'
	attributes = ATTRIBUTES[color]

	print(f'ROTATE =R{angle:d} D{led}')
	print(f'MOVE D{led} ({x:06.4f} {y:06.4f})')
	print(f'VALUE D{led} {color}')
	for k, v in attributes.items():
		print(f'ATTRIBUTE D{led} {k} DELETE')
		print(f"ATTRIBUTE D{led} {k} '{v}';")

