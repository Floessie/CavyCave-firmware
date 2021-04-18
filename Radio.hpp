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
class Stats;

class Radio final
{
public:
	struct Configuration {
		uint8_t channel;
		uint8_t address[5];
	};

	Radio(Controller& _controller, Stats& _stats);
	~Radio();

	void begin();

	bool isReady() const;

	const Configuration& getConfiguration() const;
	void setConfiguration(const Configuration& value);

	bool run();

	void dump() const;

private:
	class Implementation;

	Implementation* const implementation;
};

