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

#include "Debug.hpp"

#include "Controller.hpp"
#include "Radio.hpp"
#include "Stats.hpp"

namespace
{

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

	void handle(const String& command, Controller& controller, Radio& radio, Stats& stats)
	{
		bool handled = false;

		if (!command.length()) {
			controller.dump();
			stats.dump();
			radio.dump();
			handled = true;
		}
		else if (command == F("auto")) {
			controller.setAutoMode();
			Serial.println(F("Mode set to AUTO"));
			handled = true;
		}
		else if (command == F("reset")) {
			stats.reset();
			Serial.println(F("Statistics reset"));
			handled = true;
		}

		unsigned int equal_pos = 0;
		for (; equal_pos < command.length() && command[equal_pos] != '='; ++equal_pos);

		if (equal_pos != command.length()) {
			const String& cmd = command.substring(0, equal_pos);
			const String& val = command.substring(equal_pos + 1);

			if (cmd == F("fan")) {
				if (val == F("off")) {
					controller.setFanSpeed(Fan::Speed::OFF);
					Serial.println(F("Fan OFF"));
					handled = true;
				}
				else if (val == F("low")) {
					controller.setFanSpeed(Fan::Speed::LOW);
					Serial.println(F("Fan LOW"));
					handled = true;
				}
				else if (val == F("high")) {
					controller.setFanSpeed(Fan::Speed::HIGH);
					Serial.println(F("Fan HIGH"));
					handled = true;
				}
			}
			else if (cmd == F("lounge")) {
				if (val == F("on")) {
					controller.setHeatingLounge(true);
					Serial.println(F("Lounge heating ON"));
					handled = true;
				}
				else if (val == F("off")) {
					controller.setHeatingLounge(false);
					Serial.println(F("Lounge heating OFF"));
					handled = true;
				}
			}
			else if (cmd == F("vestibule")) {
				if (val == F("on")) {
					controller.setHeatingVestibule(true);
					Serial.println(F("Vestibule heating ON"));
					handled = true;
				}
				else if (val == F("off")) {
					controller.setHeatingVestibule(false);
					Serial.println(F("Vestibule heating OFF"));
					handled = true;
				}
			}
			else if (cmd == F("led")) {
				if (val == F("green")) {
					controller.setLedColor(Led::Color::GREEN);
					Serial.println(F("LED color GREEN"));
					handled = true;
				}
				else if (val == F("yellow")) {
					controller.setLedColor(Led::Color::YELLOW);
					Serial.println(F("LED color YELLOW"));
					handled = true;
				}
				else if (val == F("red")) {
					controller.setLedColor(Led::Color::RED);
					Serial.println(F("LED color RED"));
					handled = true;
				}
			}
			else if (cmd == F("min_room_temp")) {
				const int16_t v = val.toFloat() * 10;
				Controller::Configuration configuration = controller.getConfiguration();
				configuration.min_room_temperature_10th_c = v;
				controller.setConfiguration(configuration);
				Serial.print(F("Minimum room temperature set to "));
				printTemperature(v);
				handled = true;
			}
			else if (cmd == F("max_room_temp")) {
				const int16_t v = val.toFloat() * 10;
				Controller::Configuration configuration = controller.getConfiguration();
				configuration.max_room_temperature_10th_c = v;
				controller.setConfiguration(configuration);
				Serial.print(F("Maximum room temperature set to "));
				printTemperature(v);
				handled = true;
			}
			else if (cmd == F("min_floor_temp")) {
				const int16_t v = val.toFloat() * 10;
				Controller::Configuration configuration = controller.getConfiguration();
				configuration.min_floor_temperature_10th_c = v;
				controller.setConfiguration(configuration);
				Serial.print(F("Minimum floor temperature set to "));
				Serial.print(v / 10);
				Serial.print(F("."));
				Serial.print(abs(v) % 10);
				Serial.println(F("°C"));
				handled = true;
			}
			else if (cmd == F("max_floor_temp")) {
				const int16_t v = val.toFloat() * 10;
				Controller::Configuration configuration = controller.getConfiguration();
				configuration.max_floor_temperature_10th_c = v;
				controller.setConfiguration(configuration);
				Serial.print(F("Maximum floor temperature set to "));
				printTemperature(v);
				handled = true;
			}
			else if (cmd == F("max_humidity")) {
				const int16_t v = val.toFloat() * 10;
				Controller::Configuration configuration = controller.getConfiguration();
				configuration.max_humidity_per_mill = v;
				controller.setConfiguration(configuration);
				Serial.print(F("Maximum humidity set to "));
				printHumidity(v);
				handled = true;
			}
			else if (cmd == F("min_humidity")) {
				const int16_t v = val.toFloat() * 10;
				Controller::Configuration configuration = controller.getConfiguration();
				configuration.min_humidity_per_mill = v;
				controller.setConfiguration(configuration);
				Serial.print(F("Minimum humidity set to "));
				printHumidity(v);
				handled = true;
			}
			else if (cmd == F("fan_speedup_delay_minutes")) {
				const uint8_t v = val.toInt();
				Controller::Configuration configuration = controller.getConfiguration();
				configuration.fan_speedup_delay_minutes = v;
				controller.setConfiguration(configuration);
				Serial.print(F("Fan speedup delay minutes set to "));
				Serial.println(static_cast<unsigned int>(v));
				handled = true;
			}
			else if (cmd == F("fan_speed_low")) {
				const uint8_t v = val.toInt();
				Controller::Configuration configuration = controller.getConfiguration();
				configuration.fan_speed_low = v;
				controller.setConfiguration(configuration);
				Serial.print(F("Fan speed LOW value set to "));
				Serial.println(static_cast<unsigned int>(v));
				handled = true;
			}
			else if (cmd == F("fan_speed_high")) {
				const uint8_t v = val.toInt();
				Controller::Configuration configuration = controller.getConfiguration();
				configuration.fan_speed_high = v;
				controller.setConfiguration(configuration);
				Serial.print(F("Fan speed HIGH value set to "));
				Serial.println(static_cast<unsigned int>(v));
				handled = true;
			}
			else if (cmd == F("auto_mode")) {
				Controller::Configuration configuration = controller.getConfiguration();
				if (val == F("independent")) {
					configuration.auto_mode = Controller::Configuration::AutoMode::INDEPENDENT;
					Serial.println(F("Auto mode set to INDEPENDENT"));
					handled = true;
				}
				if (val == F("linked")) {
					configuration.auto_mode = Controller::Configuration::AutoMode::LINKED;
					Serial.println(F("Auto mode set to LINKED"));
					handled = true;
				}
				if (handled) {
					controller.setConfiguration(configuration);
				}
			}
			else if (cmd == F("channel")) {
				const uint8_t v = val.toInt();
				Radio::Configuration configuration = radio.getConfiguration();
				configuration.channel = v;
				radio.setConfiguration(configuration);
				Serial.print(F("Channel set to "));
				Serial.println(v);
				Serial.println(F("Restart required"));
				handled = true;
			}
			else if (cmd == F("address") && val.length() == 10) {
				const auto hex_to_int =
					[](char in, bool& valid) -> uint8_t
					{
						if (in >= '0' && in <= '9') {
							return in - '0';
						}
						if (in >= 'A' && in <= 'F') {
							return in - 'A' + 10;
						}
						if (in >= 'a' && in <= 'f') {
							return in - 'a' + 10;
						}
						valid = false;
						return 0;
					};

				bool valid = true;
				uint8_t v[5];
				for (uint8_t i = 0; i < 5 && valid; ++i) {
					v[i] = hex_to_int(val[i * 2], valid) << 4 | hex_to_int(val[i * 2 + 1], valid);
				}

				if (valid) {
					Radio::Configuration configuration = radio.getConfiguration();
					for (uint8_t i = 0; i < 5; ++i) {
						configuration.address[i] = v[i];
					}
					radio.setConfiguration(configuration);
					Serial.print(F("Address set to "));
					for (uint8_t i = 0; i < 5; ++i) {
						if (i) {
							Serial.print(' ');
						}
						if (v[i] < 16) {
							Serial.print('0');
						}
						Serial.print(v[i], HEX);
					}
					Serial.println();
					Serial.println(F("Restart required"));
				}

				handled = true;
			}
		}

		if (!handled) {
			Serial.println(F("Unknown command"));
		}
	}

}

Debug::Debug(Controller& _controller, Radio& _radio, Stats& _stats) :
	controller(_controller),
	radio(_radio),
	stats(_stats)
{
}

void Debug::begin(uint32_t baudrate)
{
	Serial.begin(baudrate);
}

void Debug::onSerialEvent()
{
	constexpr unsigned int command_buffer_size = 32;

	static String command_buffer;
	static bool command_buffer_initialized = false;

	if (!command_buffer_initialized) {
		command_buffer.reserve(command_buffer_size);
		command_buffer_initialized = true;
	}

	while (Serial.available()) {
		const char input = Serial.read();
		const bool is_eol = input == '\n';

		if (!is_eol) {
			command_buffer += input;
		}

		if (is_eol || command_buffer.length() == command_buffer_size) {
			handle(command_buffer, controller, radio, stats);
			command_buffer = "";
		}
	}
}
