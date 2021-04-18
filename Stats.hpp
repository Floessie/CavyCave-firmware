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

#pragma once

#include <stdint.h>

class Controller;

class Stats final
{
public:
	Stats(const Controller& _controller);

	void begin();

	void run();

	void dump() const;

	void reset();

	uint32_t getSecondsSinceReset() const;

	int16_t getMinRoomTemperature10thC() const;
	int16_t getMaxRoomTemperature10thC() const;

	int16_t getMinFloorTemperature10thC() const;
	int16_t getMaxFloorTemperature10thC() const;

	int16_t getMinHumidityPerMill() const;
	int16_t getMaxHumidityPerMill() const;

	uint16_t getLoungeHeatingCount() const;
	uint32_t getLoungeHeatingSeconds() const;

	uint16_t getVestibuleHeatingCount() const;
	uint32_t getVestibuleHeatingSeconds() const;

	uint16_t getFanCount() const;
	uint32_t getFanLowSeconds() const;
	uint32_t getFanHighSeconds() const;

private:
	class Implementation;

	Implementation* const implementation;
};
