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

class Sensors final
{
public:
	Sensors();
	~Sensors();

	void begin();

	bool areRoomValuesValid() const;
	int16_t getTemperature10thC() const;
	int16_t getHumidityPerMill() const;

	bool isFloorValueValid() const;
	int16_t getFloorTemperature10thC() const;

private:
	class Implementation;

	Implementation* const implementation;
};
