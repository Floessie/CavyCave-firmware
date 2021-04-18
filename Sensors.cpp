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

#include <TroykaDHT.h>

#include <OneWire.h>
#define REQUIRESALARMS false
#include <DallasTemperature.h>

#include "Sensors.hpp"

#include "Pins.hpp"

namespace
{

	constexpr bool timeAfter(uint32_t a, uint32_t b)
	{
		return static_cast<int32_t>(b - a) < 0;
	}

}

class Sensors::Implementation final
{
public:
	Implementation() :
		dht22(Pin::DHT_A, DHT22),
		dht22_next_update_timestamp(0),
		dht22_consecutive_errors(0),
		one_wire(Pin::DS_A),
		temperature_sensors(&one_wire),
		ds18b20_last_value(20),
		ds18b20_consecutive_errors(0)
	{
		pinMode(Pin::DHT_PWR, OUTPUT);
		digitalWrite(Pin::DHT_PWR, true);
		pinMode(Pin::DS_PWR, OUTPUT);
		digitalWrite(Pin::DS_PWR, true);
	}

	void begin()
	{
		dht22.begin();
	}

	bool areRoomValuesValid() const
	{
		return dht22_consecutive_errors < 5;
	}

	int16_t getTemperature10thC()
	{
		updateDht22();
		return dht22.getTemperatureC() * 10;
	}

	int16_t getHumidityPerMill()
	{
		updateDht22();
		return dht22.getHumidity() * 10;
	}

	bool isFloorValueValid() const
	{
		return ds18b20_consecutive_errors < 5;
	}

	int16_t getFloorTemperature10thC()
	{
		if (temperature_sensors.requestTemperaturesByIndex(0)) {
			ds18b20_last_value = temperature_sensors.getTempCByIndex(0) * 10;
			ds18b20_consecutive_errors = 0;
		} else {
			++ds18b20_consecutive_errors;

			if (!ds18b20_consecutive_errors) {
				++ds18b20_consecutive_errors;
			}

			digitalWrite(Pin::DS_PWR, static_cast<bool>(ds18b20_consecutive_errors % 5));
		}

		return ds18b20_last_value;
	}

private:
	void updateDht22()
	{
		if (timeAfter(millis(), dht22_next_update_timestamp)) {
			if (dht22.read() == DHT_OK) {
				dht22_consecutive_errors = 0;
			} else {
				++dht22_consecutive_errors;

				if (!dht22_consecutive_errors) {
					++dht22_consecutive_errors;
				}

				digitalWrite(Pin::DHT_PWR, static_cast<bool>(dht22_consecutive_errors % 5));
			}

			dht22_next_update_timestamp = millis() + 2000U;
		}
	}

	DHT dht22;
	uint32_t dht22_next_update_timestamp;
	uint16_t dht22_consecutive_errors;

	OneWire one_wire;
	DallasTemperature temperature_sensors;
	int16_t ds18b20_last_value;
	uint16_t ds18b20_consecutive_errors;
};

Sensors::Sensors() :
	implementation(new Implementation)
{
};

Sensors::~Sensors()
{
	delete implementation;
}

void Sensors::begin()
{
	implementation->begin();
}

bool Sensors::areRoomValuesValid() const
{
	return implementation->areRoomValuesValid();
}

int16_t Sensors::getTemperature10thC() const
{
	return implementation->getTemperature10thC();
}

int16_t Sensors::getHumidityPerMill() const
{
	return implementation->getHumidityPerMill();
}

bool Sensors::isFloorValueValid() const
{
	return implementation->isFloorValueValid();
}

int16_t Sensors::getFloorTemperature10thC() const
{
	return implementation->getFloorTemperature10thC();
}
