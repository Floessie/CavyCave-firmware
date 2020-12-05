/*
	CavyCave - A temperature controlled box for guinea pigs and other
		small animals kept outside in winter

	Copyright (C) 2020 Flössie <floessie.mail@gmail.com>

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

/*
	CavyCave - A temperature controlled box for guinea pigs and other
		small animals kept outside in winter

	Copyright (C) 2020 Flössie <floessie.mail@gmail.com>

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

#include "Led.hpp"

#include "Pins.hpp"

Led::Led() :
	color(Color::GREEN)
{
	pinMode(Pin::LED_R, OUTPUT);
	pinMode(Pin::LED_G, OUTPUT);
}

void Led::begin()
{
	setColor(Color::GREEN);
}

Led::Color Led::getColor() const
{
	return color;
}

void Led::setColor(Color value)
{
	color = value;

	switch (value) {
		case Color::GREEN: {
			digitalWrite(Pin::LED_R, false);
			digitalWrite(Pin::LED_G, true);
			break;
		}

		case Color::YELLOW: {
			digitalWrite(Pin::LED_R, true);
			digitalWrite(Pin::LED_G, true);
			break;
		}

		case Color::RED: {
			digitalWrite(Pin::LED_R, true);
			digitalWrite(Pin::LED_G, false);
			break;
		}
	}
}

