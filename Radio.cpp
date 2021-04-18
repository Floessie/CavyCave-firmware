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
#include <EEPROM.h>

#include <nRF24L01.h>
#include <RF24.h>

#include "Radio.hpp"

#include "Controller.hpp"
#include "Pins.hpp"
#include "Stats.hpp"

class Radio::Implementation final
{
public:
	Implementation(Controller& _controller, Stats& _stats) :
		rf24(Pin::CE, Pin::CSN),
		controller(_controller),
		stats(_stats),
		configuration{
			110,
			{'C', 'C', 'a', 'v', 'e'}
		}
	{
	}

	void begin()
	{
		loadConfiguration();

		rf24.begin();
		rf24.enableDynamicPayloads();
		rf24.setAutoAck(true);
		rf24.enableAckPayload();
		rf24.setRetries(15, 15);
		rf24.setChannel(configuration.channel);
		rf24.setPALevel(RF24_PA_MAX);
		rf24.setDataRate(RF24_250KBPS);
		rf24.setAddressWidth(5);
		rf24.setCRCLength(RF24_CRC_16);

		rf24.openReadingPipe(1, configuration.address);

		rf24.startListening();
	}

	bool isReady()
	{
		return rf24.isChipConnected();
	}

	const Configuration& getConfiguration() const
	{
		return configuration;
	}

	void setConfiguration(const Configuration& value)
	{
		configuration = value;
		saveConfiguration();
	}

	bool run()
	{
		enum class Command : uint8_t {
			POLL,
			GET_STATE,
			GET_CONFIG,
			SET_CONFIG,
			SET_AUTO,
			SET_FAN,
			SET_LOUNGE,
			SET_VESTIBULE,
			SET_LED,
			GET_STATS_MIN_MAX,
			GET_STATS_DURATIONS,
			RESET_STATS
		};

		bool again = false;

		if (rf24.available() && rf24.getDynamicPayloadSize()) {
			char buffer[32];
			rf24.read(buffer, min(32, rf24.getDynamicPayloadSize()));

			switch (Command(buffer[0])) {
				case Command::POLL: {
					break;
				}

				case Command::GET_STATE: {
					struct Reply {
						Command command;
						Controller::State state;
					};

					const Reply reply = {
						Command::GET_STATE,
						controller.getState()
					};

					delay(5);

					rf24.flush_tx();
					rf24.writeAckPayload(1, &reply, sizeof(reply));

					again = true;
					break;
				}

				case Command::GET_CONFIG: {
					struct Reply {
						Command command;
						Controller::Configuration configuration;
					};

					const Reply reply = {
						Command::GET_CONFIG,
						controller.getConfiguration()
					};

					delay(5);

					rf24.flush_tx();
					rf24.writeAckPayload(1, &reply, sizeof(reply));

					again = true;
					break;
				}

				case Command::SET_CONFIG: {
					if (rf24.getDynamicPayloadSize() - 1 == sizeof(Controller::Configuration)) {
						controller.setConfiguration(*reinterpret_cast<Controller::Configuration*>(buffer + 1));
					}
					break;
				}

				case Command::SET_AUTO: {
					controller.setAutoMode();
					break;
				}

				case Command::SET_FAN: {
					if (rf24.getDynamicPayloadSize() > 1) {
						controller.setFanSpeed(Fan::Speed(buffer[1]));
					}
					break;
				}

				case Command::SET_LOUNGE: {
					if (rf24.getDynamicPayloadSize() > 1) {
						controller.setHeatingLounge(buffer[1]);
					}
					break;
				}

				case Command::SET_VESTIBULE: {
					if (rf24.getDynamicPayloadSize() > 1) {
						controller.setHeatingVestibule(buffer[1]);
					}
					break;
				}

				case Command::SET_LED: {
					if (rf24.getDynamicPayloadSize() > 1) {
						controller.setLedColor(Led::Color(buffer[1]));
					}
					break;
				}

				case Command::GET_STATS_MIN_MAX: {
					struct Reply {
						Command command;

						int16_t min_room_temperature_10th_c;
						int16_t max_room_temperature_10th_c;

						int16_t min_floor_temperature_10th_c;
						int16_t max_floor_temperature_10th_c;

						int16_t min_humidity_per_mill;
						int16_t max_humidity_per_mill;
					};

					const Reply reply = {
						Command::GET_STATS_MIN_MAX,
						stats.getMinRoomTemperature10thC(),
						stats.getMaxRoomTemperature10thC(),
						stats.getMinFloorTemperature10thC(),
						stats.getMaxFloorTemperature10thC(),
						stats.getMinHumidityPerMill(),
						stats.getMaxHumidityPerMill()
					};

					delay(5);

					rf24.flush_tx();
					rf24.writeAckPayload(1, &reply, sizeof(reply));

					again = true;
					break;
				}

				case Command::GET_STATS_DURATIONS: {
					struct Reply {
						Command command;

						uint32_t seconds_since_reset;

						uint16_t lounge_heating_count;
						uint32_t lounge_heating_seconds;

						uint16_t vestibule_heating_count;
						uint32_t vestibule_heating_seconds;

						uint16_t fan_count;
						uint32_t fan_low_seconds;
						uint32_t fan_high_seconds;
					};

					const Reply reply = {
						Command::GET_STATS_DURATIONS,
						stats.getSecondsSinceReset(),
						stats.getLoungeHeatingCount(),
						stats.getLoungeHeatingSeconds(),
						stats.getVestibuleHeatingCount(),
						stats.getVestibuleHeatingSeconds(),
						stats.getFanCount(),
						stats.getFanLowSeconds(),
						stats.getFanHighSeconds()
					};

					delay(5);

					rf24.flush_tx();
					rf24.writeAckPayload(1, &reply, sizeof(reply));

					again = true;
					break;
				}

				case Command::RESET_STATS: {
					stats.reset();
					break;
				}
			}
		}

		return again;
	}

	void dump()
	{
		Serial.println(F("Radio:"));

		Serial.print(F("  Ready: "));
		if (isReady()) {
			Serial.println(F("YES"));
		} else {
			Serial.println(F("NO"));
		}

		Serial.print(F("  Carrier: "));
		if (rf24.testCarrier()) {
			Serial.println(F("YES"));
		} else {
			Serial.println(F("NO"));
		}

		Serial.print(F("  RPD: "));
		if (rf24.testRPD()) {
			Serial.println(F("YES"));
		} else {
			Serial.println(F("NO"));
		}

		Serial.print(F("  Channel: "));
		Serial.println(configuration.channel);

		Serial.print(F("  Address: "));
		for (uint8_t i = 0; i < 5; ++i) {
			if (i) {
				Serial.print(' ');
			}
			if (configuration.address[i] < 16) {
				Serial.print('0');
			}
			Serial.print(configuration.address[i], HEX);
		}
		Serial.println();
	}

private:
	void loadConfiguration()
	{
		if (EEPROM.read(256) != 0xFF) {
			EEPROM.get(257, configuration);
		}
	}

	void saveConfiguration()
	{
		EEPROM.write(256, 0);
		EEPROM.put(257, configuration);
	}

	RF24 rf24;
	Controller& controller;
	Stats& stats;

	Configuration configuration;
};

Radio::Radio(Controller& _controller, Stats& _stats) :
	implementation(new Implementation(_controller, _stats))
{
}

Radio::~Radio()
{
	delete implementation;
}

void Radio::begin()
{
	implementation->begin();
}

bool Radio::isReady() const
{
	return implementation->isReady();
}

const Radio::Configuration& Radio::getConfiguration() const
{
	return implementation->getConfiguration();
}

void Radio::setConfiguration(const Configuration& value)
{
	implementation->setConfiguration(value);
}

bool Radio::run()
{
	return implementation->run();
}

void Radio::dump() const
{
	implementation->dump();
}
