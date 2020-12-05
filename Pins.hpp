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

#pragma once

#include <stdint.h>

#include <Arduino.h>

struct Pin {
	enum : uint8_t {
		DS_B = PIN_PB0,
		FAN = PIN_PB1,
		DS_PWR = PIN_PB2,
		HEAT_A = PIN_PC0,
		HEAT_B = PIN_PC1,
		CSN = PIN_PC2,
		CE = PIN_PC3,
		DHT_A = PIN_PC4,
		DHT_B = PIN_PC5,
		IRQ = PIN_PD2,
		LED_B = PIN_PD3,
		DHT_PWR = PIN_PD4,
		LED_R = PIN_PD5,
		LED_G = PIN_PD6,
		DS_A = PIN_PD7
	};
};
