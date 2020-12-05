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

#include "Heating.hpp"

#include "Pins.hpp"

Heating::Heating() :
	lounge(false),
	vestibule(false)
{
	pinMode(Pin::HEAT_A, OUTPUT);
	pinMode(Pin::HEAT_B, OUTPUT);
	digitalWrite(Pin::HEAT_A, false);
	digitalWrite(Pin::HEAT_B, false);
}

void Heating::begin()
{
}

bool Heating::getLounge() const
{
	return lounge;
}

void Heating::setLounge(bool value)
{
	lounge = value;
	digitalWrite(Pin::HEAT_A, value);
}

bool Heating::getVestibule() const
{
	return vestibule;
}

void Heating::setVestibule(bool value)
{
	vestibule = value;
	digitalWrite(Pin::HEAT_B, value);
}

