/*
	CavyCave - A temperature controlled box for guinea pigs and other
		small animals kept outside in winter

	Copyright (C) 2020-2021 Flössie <floessie.mail@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <Arduino.h>

#include "Fan.hpp"

#include "Pins.hpp"

Fan::Fan() :
	speed(Speed::OFF),
	low_speed(140),
	high_speed(180)
{
	pinMode(Pin::FAN, OUTPUT);
	analogWrite(Pin::FAN, 0);
}

void Fan::begin()
{
	// Set frequency divisor to 1024
	TCCR1B = TCCR1B & ~0x07 | 0x05;
}

Fan::Speed Fan::getSpeed() const
{
	return speed;
}

void Fan::setSpeed(Speed value)
{
	speed = value;

	switch (value) {
		case Speed::OFF: {
			analogWrite(Pin::FAN, 0);
			break;
		}

		case Speed::LOW: {
			analogWrite(Pin::FAN, low_speed);
			break;
		}

		case Speed::HIGH: {
			analogWrite(Pin::FAN, high_speed);
			break;
		}
	}
}

void Fan::setLowSpeed(uint8_t value)
{
	low_speed = value;
	setSpeed(speed);
}

void Fan::setHighSpeed(uint8_t value)
{
	high_speed = value;
	setSpeed(speed);
}
