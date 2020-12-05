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

#include <Arduino.h>

#include "Fan.hpp"
#include "Heating.hpp"
#include "Led.hpp"
#include "Sensors.hpp"

class Controller final
{
public:
	enum class Mode : uint8_t {
		AUTO,
		MANUAL
	};

	struct Configuration {
		enum class AutoMode : uint8_t {
			INDEPENDENT,
			LINKED
		};

		int16_t min_room_temperature_10th_c;
		int16_t max_room_temperature_10th_c;

		int16_t min_floor_temperature_10th_c;
		int16_t max_floor_temperature_10th_c;

		int16_t max_humidity_per_mill;
		int16_t min_humidity_per_mill;

		uint8_t fan_speedup_delay_minutes;
		uint8_t fan_speed_low;
		uint8_t fan_speed_high;

		AutoMode auto_mode;
	};

	struct State {
		bool room_values_valid;
		int16_t temperature_10th_c;
		int16_t humidity_per_mill;

		bool floor_value_valid;
		int16_t floor_temperature_10th_c;

		Mode mode;

		Fan::Speed fan_speed;

		bool heating_lounge;
		bool heating_vestibule;

		Led::Color led_color;
	};

	Controller();
	~Controller();

	void begin();

	const Configuration& getConfiguration() const;
	void setConfiguration(const Configuration& value);

	void run();

	void dump() const;

	const State& getState() const;

	Mode getMode() const;
	void setAutoMode();

	void setFanSpeed(Fan::Speed value);

	void setHeatingLounge(bool value);
	void setHeatingVestibule(bool value);

	void setLedColor(Led::Color value);

private:
	class Implementation;

	Implementation* const implementation;
};
