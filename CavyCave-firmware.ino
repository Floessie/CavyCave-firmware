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

#include "Controller.hpp"
#include "Debug.hpp"
#include "Radio.hpp"
#include "Stats.hpp"

namespace
{

	Controller controller;
	Stats stats(controller);
	Radio radio(controller, stats);
	Debug debug(controller, radio, stats);

}

void setup()
{
	controller.begin();
	stats.begin();
	debug.begin(57600);
	radio.begin();
}

void loop()
{
	controller.run();
	stats.run();
	while (radio.run());
}

void serialEvent()
{
	debug.onSerialEvent();
}
