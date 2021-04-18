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

#include <EEPROM.h>

#include "Controller.hpp"

namespace
{

	Fan fan;
	Heating heating;
	Led led;
	Sensors sensors;

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

}

class Controller::Implementation final
{
public:
	Implementation() :
		configuration{
			95,
			110,
			100,
			200,
			750,
			700,
			0,
			0,
			5,
			140,
			180,
			Configuration::AutoMode::INDEPENDENT
		},
		state{
			false,
			0,
			0,
			false,
			0,
			Mode::AUTO,
			Fan::Speed::OFF,
			false,
			false,
			Led::Color::RED
		},
		next_update_timestamp(0),
		fan_timer(FanTimer::OFF),
		fan_timer_timestamp(0),
		fan_speedup_timestamp(0)
	{
	}

	void begin()
	{
		fan.begin();
		heating.begin();
		led.begin();
		sensors.begin();

		loadConfiguration();
		fan.setLowSpeed(configuration.fan_speed_low);
		fan.setHighSpeed(configuration.fan_speed_high);
	}

	const Configuration& getConfiguration() const
	{
		return configuration;
	}

	void setConfiguration(const Configuration& value)
	{
		configuration = value;
		fan.setLowSpeed(value.fan_speed_low);
		fan.setHighSpeed(value.fan_speed_high);
		saveConfiguration();
	}

	void run()
	{
		if (timeAfter(millis(), next_update_timestamp)) {
			next_update_timestamp = millis() + 2500UL;

			state.room_values_valid = sensors.areRoomValuesValid();
			state.temperature_10th_c = sensors.getTemperature10thC();
			state.humidity_per_mill = sensors.getHumidityPerMill();

			state.floor_value_valid = sensors.isFloorValueValid();
			state.floor_temperature_10th_c = sensors.getFloorTemperature10thC();

			if (state.mode == Mode::AUTO) {
				switch (configuration.auto_mode) {
					case Configuration::AutoMode::INDEPENDENT: {
						if (state.room_values_valid) {
							if (!state.heating_lounge && state.temperature_10th_c <= configuration.min_room_temperature_10th_c) {
								state.heating_lounge = true;
							}
							else if (state.heating_lounge && state.temperature_10th_c >= configuration.max_room_temperature_10th_c) {
								state.heating_lounge = false;
							}
						}

						if (state.floor_value_valid) {
							if (!state.heating_vestibule && state.floor_temperature_10th_c <= configuration.min_floor_temperature_10th_c) {
								state.heating_vestibule = true;
							}
							else if (state.heating_vestibule && state.floor_temperature_10th_c >= configuration.max_floor_temperature_10th_c) {
								state.heating_vestibule = false;
							}
						}
						break;
					}

					case Configuration::AutoMode::LINKED: {
						if (state.room_values_valid && state.floor_value_valid) {
							if (
								!state.heating_lounge
								&& (
									state.temperature_10th_c <= configuration.min_room_temperature_10th_c
									|| state.floor_temperature_10th_c <= configuration.min_floor_temperature_10th_c
								)
								&& state.floor_temperature_10th_c < configuration.max_floor_temperature_10th_c
							) {
								state.heating_lounge = true;
								state.heating_vestibule = true;
							}
							else if (state.heating_lounge && state.floor_temperature_10th_c >= configuration.max_floor_temperature_10th_c) {
								state.heating_lounge = false;
								state.heating_vestibule = false;
							}
						}
						break;
					}
				}

				if (fan_timer != FanTimer::PAUSE && state.room_values_valid) {
					if (state.fan_speed == Fan::Speed::OFF && state.humidity_per_mill >= configuration.max_humidity_per_mill) {
						state.fan_speed = Fan::Speed::LOW;
						fan_speedup_timestamp = millis() + configuration.fan_speedup_delay_minutes * 60UL * 1000UL;
					}
					else if (state.fan_speed != Fan::Speed::OFF && state.humidity_per_mill <= configuration.min_humidity_per_mill) {
						state.fan_speed = Fan::Speed::OFF;
						fan_timer = FanTimer::OFF;
					}
				}

				if (state.room_values_valid && state.floor_value_valid) {
					if (state.heating_lounge || state.heating_vestibule) {
						state.led_color = Led::Color::YELLOW;
					} else {
						state.led_color = Led::Color::GREEN;
					}
				} else {
					state.led_color = Led::Color::RED;
				}
			}
		}

		if (state.mode == Mode::AUTO) {
			if (state.fan_speed == Fan::Speed::LOW && timeAfter(millis(), fan_speedup_timestamp)) {
				state.fan_speed = Fan::Speed::HIGH;
			}

			if (
				configuration.fan_max_run_minutes
				&& state.fan_speed != Fan::Speed::OFF
				&& fan_timer == FanTimer::OFF
			) {
				fan_timer = FanTimer::ON;
				fan_timer_timestamp = millis() + configuration.fan_max_run_minutes * 60UL * 1000UL;
			}

			if (fan_timer != FanTimer::OFF && timeAfter(millis(), fan_timer_timestamp)) {
				if (fan_timer == FanTimer::ON) {
					state.fan_speed = Fan::Speed::OFF;
					fan_timer = FanTimer::PAUSE;
					fan_timer_timestamp = millis() + configuration.fan_pause_minutes * 60UL * 1000UL;
				} else {
					fan_timer = FanTimer::OFF;
				}
			}
		}

		fan.setSpeed(state.fan_speed);

		heating.setLounge(state.heating_lounge);
		heating.setVestibule(state.heating_vestibule);

		led.setColor(state.led_color);
	}

	void dump() const
	{
		Serial.println(F("Sensors:"));
		if (state.room_values_valid) {
			Serial.print(F("  Temperature: "));
			printTemperature(state.temperature_10th_c);

			Serial.print(F("  Humidity: "));
			printHumidity(state.humidity_per_mill);
		} else {
			Serial.println(F("  Error reading room values."));
		}

		if (state.floor_value_valid) {
			Serial.print(F("  Floor temperature: "));
			printTemperature(state.floor_temperature_10th_c);
		} else {
			Serial.println(F("  Error reading floor value."));
		}

		Serial.println(F("State:"));

		Serial.print(F("  Operational mode: "));
		switch (state.mode) {
			case Mode::AUTO: {
				Serial.println(F("AUTO"));
				break;
			}

			case Mode::MANUAL: {
				Serial.println(F("MANUAL"));
				break;
			}
		}

		Serial.print(F("  Fan speed: "));
		switch (state.fan_speed) {
			case Fan::Speed::OFF: {
				Serial.println(F("OFF"));
				break;
			}

			case Fan::Speed::LOW: {
				Serial.println(F("LOW"));
				break;
			}

			case Fan::Speed::HIGH: {
				Serial.println(F("HIGH"));
				break;
			}
		}

		Serial.print(F("  Lounge heating: "));
		switch (state.heating_lounge) {
			case false: {
				Serial.println(F("OFF"));
				break;
			}

			case true: {
				Serial.println(F("ON"));
				break;
			}
		}

		Serial.print(F("  Vestibule heating: "));
		switch (state.heating_vestibule) {
			case false: {
				Serial.println(F("OFF"));
				break;
			}

			case true: {
				Serial.println(F("ON"));
				break;
			}
		}

		Serial.print(F("  LED color: "));
		switch (state.led_color) {
			case Led::Color::GREEN: {
				Serial.println(F("GREEN"));
				break;
			}

			case Led::Color::YELLOW: {
				Serial.println(F("YELLOW"));
				break;
			}

			case Led::Color::RED: {
				Serial.println(F("RED"));
				break;
			}
		}

		Serial.println(F("Configuration:"));

		Serial.print(F("  Minimum room temperature: "));
		printTemperature(configuration.min_room_temperature_10th_c);
		Serial.print(F("  Maximum room temperature: "));
		printTemperature(configuration.max_room_temperature_10th_c);

		Serial.print(F("  Minimum floor temperature: "));
		printTemperature(configuration.min_floor_temperature_10th_c);
		Serial.print(F("  Maximum floor temperature: "));
		printTemperature(configuration.max_floor_temperature_10th_c);

		Serial.print(F("  Maximum humidity: "));
		printHumidity(configuration.max_humidity_per_mill);
		Serial.print(F("  Minimum humidity: "));
		printHumidity(configuration.min_humidity_per_mill);

		Serial.print(F("  Fan maximum run minutes: "));
		if (configuration.fan_max_run_minutes) {
			Serial.println(static_cast<unsigned int>(configuration.fan_max_run_minutes));
		} else {
			Serial.println(F("unlimited"));
		}
		Serial.print(F("  Fan pause minutes: "));
		Serial.println(static_cast<unsigned int>(configuration.fan_pause_minutes));
		Serial.print(F("  Fan speedup delay minutes: "));
		Serial.println(static_cast<unsigned int>(configuration.fan_speedup_delay_minutes));
		Serial.print(F("  Fan speed LOW value: "));
		Serial.println(static_cast<unsigned int>(configuration.fan_speed_low));
		Serial.print(F("  Fan speed HIGH value: "));
		Serial.println(static_cast<unsigned int>(configuration.fan_speed_high));

		Serial.print(F("  Auto mode: "));
		switch (configuration.auto_mode) {
			case Configuration::AutoMode::INDEPENDENT: {
				Serial.println(F("INDEPENDENT"));
				break;
			}

			case Configuration::AutoMode::LINKED: {
				Serial.println(F("LINKED"));
				break;
			}
		}
	}

	const State& getState() const
	{
		return state;
	}

	Mode getMode() const
	{
		return state.mode;
	}

	void setAutoMode()
	{
		state.mode = Mode::AUTO;

		state.fan_speed = Fan::Speed::OFF;
		fan_timer = FanTimer::OFF;
	}

	void setFanSpeed(Fan::Speed value)
	{
		state.mode = Mode::MANUAL;

		state.fan_speed = value;
		fan_timer = FanTimer::OFF;
	}

	void setHeatingLounge(bool value)
	{
		state.mode = Mode::MANUAL;

		state.heating_lounge = value;
	}

	void setHeatingVestibule(bool value)
	{
		state.mode = Mode::MANUAL;

		state.heating_vestibule = value;
	}

	void setLedColor(Led::Color value)
	{
		state.mode = Mode::MANUAL;

		state.led_color = value;
	}

private:
	enum class FanTimer {
		OFF,
		ON,
		PAUSE
	};

	void loadConfiguration()
	{
		if (EEPROM.read(0) != 0xFF) {
			EEPROM.get(1, configuration);
			if (configuration.auto_mode != Configuration::AutoMode::INDEPENDENT && configuration.auto_mode != Configuration::AutoMode::LINKED) {
				configuration.auto_mode = Configuration::AutoMode::INDEPENDENT;
			}
		}
	}

	void saveConfiguration()
	{
		EEPROM.write(0, 0);
		EEPROM.put(1, configuration);
	}

	Configuration configuration;
	State state;

	uint32_t next_update_timestamp;

	FanTimer fan_timer;
	uint32_t fan_timer_timestamp;
	uint32_t fan_speedup_timestamp;
};

Controller::Controller() :
	implementation(new Implementation)
{
}

Controller::~Controller()
{
	delete implementation;
}

void Controller::begin()
{
	implementation->begin();
}

const Controller::Configuration& Controller::getConfiguration() const
{
	return implementation->getConfiguration();
}

void Controller::setConfiguration(const Configuration& value)
{
	implementation->setConfiguration(value);
}

void Controller::run()
{
	implementation->run();
}

void Controller::dump() const
{
	implementation->dump();
}

const Controller::State& Controller::getState() const
{
	return implementation->getState();
}

Controller::Mode Controller::getMode() const
{
	return implementation->getMode();
}

void Controller::setAutoMode()
{
	implementation->setAutoMode();
}

void Controller::setFanSpeed(Fan::Speed value)
{
	implementation->setFanSpeed(value);
}

void Controller::setHeatingLounge(bool value)
{
	implementation->setHeatingLounge(value);
}

void Controller::setHeatingVestibule(bool value)
{
	implementation->setHeatingVestibule(value);
}

void Controller::setLedColor(Led::Color value)
{
	implementation->setLedColor(value);
}
