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

#include "Stats.hpp"

#include "Controller.hpp"

namespace
{

	constexpr uint32_t period_ms = 5000;

	constexpr bool timeAfter(uint32_t a, uint32_t b)
	{
		return static_cast<int32_t>(b - a) < 0;
	}

	void printTemperature(int16_t temperature_10th_c)
	{
		Serial.print(temperature_10th_c / 10);
		Serial.print(F("."));
		Serial.print(abs(temperature_10th_c) % 10);
		Serial.println(F("°C"));
	}

	void printHumidity(int16_t humidity_per_mill)
	{
		Serial.print(humidity_per_mill / 10);
		Serial.print(F("."));
		Serial.print(abs(humidity_per_mill) % 10);
		Serial.println(F("%"));
	}

	void print02(uint32_t value)
	{
		if (value < 10) {
			Serial.print(F("0"));
		}
		Serial.print(value);
	}

	void printDuration(uint32_t seconds)
	{
		const uint32_t hours = seconds / 3600UL;
		seconds -= hours * 3600UL;
		const uint32_t minutes = seconds / 60UL;
		seconds -= minutes * 60UL;

		print02(hours);
		Serial.print(F(":"));
		print02(minutes);
		Serial.print(F(":"));
		print02(seconds);
		Serial.println();
	}

}

class Stats::Implementation final
{
public:
	Implementation(const Controller& _controller) :
		controller(_controller)
	{
	}

	void begin()
	{
		reset();
	}

	void run()
	{
		const uint32_t now = millis();

		if (timeAfter(now, next_update_timestamp)) {
			constexpr uint32_t seconds = period_ms / 1000UL;

			next_update_timestamp += period_ms;

			seconds_since_reset += seconds;

			const Controller::State state = controller.getState();

			if (state.room_values_valid) {
				min_room_temperature_10th_c = min(min_room_temperature_10th_c, state.temperature_10th_c);
				max_room_temperature_10th_c = max(max_room_temperature_10th_c, state.temperature_10th_c);

				min_humidity_per_mill = min(min_humidity_per_mill, state.humidity_per_mill);
				max_humidity_per_mill = max(max_humidity_per_mill, state.humidity_per_mill);
			}

			if (state.floor_value_valid) {
				min_floor_temperature_10th_c = min(min_floor_temperature_10th_c, state.floor_temperature_10th_c);
				max_floor_temperature_10th_c = max(max_floor_temperature_10th_c, state.floor_temperature_10th_c);
			}

			if (!prev_lounge_heating && state.heating_lounge) {
				++lounge_heating_count;
			}
			prev_lounge_heating = state.heating_lounge;
			if (state.heating_lounge) {
				lounge_heating_seconds += seconds;
			}

			if (!prev_vestibule_heating && state.heating_vestibule) {
				++vestibule_heating_count;
			}
			prev_vestibule_heating = state.heating_vestibule;
			if (state.heating_vestibule) {
				vestibule_heating_seconds += seconds;
			}

			if (!prev_fan && state.fan_speed != Fan::Speed::OFF) {
				++fan_count;
			}
			prev_fan = state.fan_speed != Fan::Speed::OFF;
			if (state.fan_speed == Fan::Speed::LOW) {
				fan_low_seconds += seconds;
			}
			else if (state.fan_speed == Fan::Speed::HIGH) {
				fan_high_seconds += seconds;
			}
		}
	}

	void dump() const
	{
		Serial.println(F("Statistics:"));

		Serial.print(F("  Counting for: "));
		printDuration(seconds_since_reset);

		Serial.print(F("  Minimum temperature: "));
		printTemperature(min_room_temperature_10th_c);
		Serial.print(F("  Maximum temperature: "));
		printTemperature(max_room_temperature_10th_c);

		Serial.print(F("  Minimum humidity: "));
		printHumidity(min_humidity_per_mill);
		Serial.print(F("  Maximum humidity: "));
		printHumidity(max_humidity_per_mill);

		Serial.print(F("  Minimum floor temperature: "));
		printTemperature(min_floor_temperature_10th_c);
		Serial.print(F("  Maximum floor temperature: "));
		printTemperature(max_floor_temperature_10th_c);

		Serial.print(F("  Lounge heating count: "));
		Serial.println(lounge_heating_count);
		Serial.print(F("  Lounge heating duration: "));
		printDuration(lounge_heating_seconds);

		Serial.print(F("  Vestibule heating count: "));
		Serial.println(vestibule_heating_count);
		Serial.print(F("  Vestibule heating duration: "));
		printDuration(vestibule_heating_seconds);

		Serial.print(F("  Fan run count: "));
		Serial.println(fan_count);
		Serial.print(F("  Fan LOW duration: "));
		printDuration(fan_low_seconds);
		Serial.print(F("  Fan HIGH duration: "));
		printDuration(fan_high_seconds);
	}

	void reset()
	{
		next_update_timestamp = millis() + period_ms;

		seconds_since_reset = 0;

		min_room_temperature_10th_c = INT16_MAX;
		max_room_temperature_10th_c = -INT16_MAX;

		min_floor_temperature_10th_c = INT16_MAX;
		max_floor_temperature_10th_c = -INT16_MAX;

		min_humidity_per_mill = INT16_MAX;
		max_humidity_per_mill = -INT16_MAX;

		prev_lounge_heating = false;
		lounge_heating_count = 0;
		lounge_heating_seconds = 0;

		prev_vestibule_heating = false;
		vestibule_heating_count = 0;
		vestibule_heating_seconds = 0;

		prev_fan = false;
		fan_count = 0;
		fan_low_seconds = 0;
		fan_high_seconds = 0;
	}

	uint32_t getSecondsSinceReset() const
	{
		return seconds_since_reset;
	}

	int16_t getMinRoomTemperature10thC() const
	{
		return min_room_temperature_10th_c;
	}

	int16_t getMaxRoomTemperature10thC() const
	{
		return max_room_temperature_10th_c;
	}

	int16_t getMinFloorTemperature10thC() const
	{
		return min_floor_temperature_10th_c;
	}

	int16_t getMaxFloorTemperature10thC() const
	{
		return max_floor_temperature_10th_c;
	}

	int16_t getMinHumidityPerMill() const
	{
		return min_humidity_per_mill;
	}

	int16_t getMaxHumidityPerMill() const
	{
		return max_humidity_per_mill;
	}

	uint16_t getLoungeHeatingCount() const
	{
		return lounge_heating_count;
	}

	uint32_t getLoungeHeatingSeconds() const
	{
		return lounge_heating_seconds;
	}

	uint16_t getVestibuleHeatingCount() const
	{
		return vestibule_heating_count;
	}

	uint32_t getVestibuleHeatingSeconds() const
	{
		return vestibule_heating_seconds;
	}

	uint16_t getFanCount() const
	{
		return fan_count;
	}

	uint32_t getFanLowSeconds() const
	{
		return fan_low_seconds;
	}

	uint32_t getFanHighSeconds() const
	{
		return fan_high_seconds;
	}

private:
	const Controller& controller;

	uint32_t next_update_timestamp;

	uint32_t seconds_since_reset;

	int16_t min_room_temperature_10th_c;
	int16_t max_room_temperature_10th_c;

	int16_t min_floor_temperature_10th_c;
	int16_t max_floor_temperature_10th_c;

	int16_t min_humidity_per_mill;
	int16_t max_humidity_per_mill;

	bool prev_lounge_heating;
	uint16_t lounge_heating_count;
	uint32_t lounge_heating_seconds;

	bool prev_vestibule_heating;
	uint16_t vestibule_heating_count;
	uint32_t vestibule_heating_seconds;

	bool prev_fan;
	uint16_t fan_count;
	uint32_t fan_low_seconds;
	uint32_t fan_high_seconds;
};

Stats::Stats(const Controller& _controller) :
	implementation(new Implementation(_controller))
{
}

void Stats::reset()
{
	implementation->reset();
}

void Stats::begin()
{
	implementation->begin();
}

void Stats::run()
{
	implementation->run();
}

void Stats::dump() const
{
	implementation->dump();
}

uint32_t Stats::getSecondsSinceReset() const
{
	return implementation->getSecondsSinceReset();
}

int16_t Stats::getMinRoomTemperature10thC() const
{
	return implementation->getMinRoomTemperature10thC();
}

int16_t Stats::getMaxRoomTemperature10thC() const
{
	return implementation->getMaxRoomTemperature10thC();
}

int16_t Stats::getMinFloorTemperature10thC() const
{
	return implementation->getMinFloorTemperature10thC();
}

int16_t Stats::getMaxFloorTemperature10thC() const
{
	return implementation->getMaxFloorTemperature10thC();
}

int16_t Stats::getMinHumidityPerMill() const
{
	return implementation->getMinHumidityPerMill();
}

int16_t Stats::getMaxHumidityPerMill() const
{
	return implementation->getMaxHumidityPerMill();
}

uint16_t Stats::getLoungeHeatingCount() const
{
	return implementation->getLoungeHeatingCount();
}

uint32_t Stats::getLoungeHeatingSeconds() const
{
	return implementation->getLoungeHeatingSeconds();
}

uint16_t Stats::getVestibuleHeatingCount() const
{
	return implementation->getVestibuleHeatingCount();
}

uint32_t Stats::getVestibuleHeatingSeconds() const
{
	return implementation->getVestibuleHeatingSeconds();
}

uint16_t Stats::getFanCount() const
{
	return implementation->getFanCount();
}

uint32_t Stats::getFanLowSeconds() const
{
	return implementation->getFanLowSeconds();
}

uint32_t Stats::getFanHighSeconds() const
{
	return implementation->getFanHighSeconds();
}
