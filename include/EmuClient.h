#ifndef ESP_READER_EMU_INCLUDE_EMUCLIENT_H
#define ESP_READER_EMU_INCLUDE_EMUCLIENT_H

#include "JVSIO.h"

class EmuDataClient final : public JVSIO::DataClient {
	int available() override {
		return Serial1.available();
	}
	
	uint8_t read() override {
		return Serial1.read();
	}
	
	void write(uint8_t data) override {
		Serial1.write(data);
	}
};

class EmuSenseClient final : public JVSIO::SenseClient {
#ifdef ARDUINO_ARCH_ESP32
	const uint8_t PIN = 1;
#else
	const uint8_t PIN = 2;
#endif
	void begin() override {
		pinMode(PIN, OUTPUT);
		digitalWrite(PIN, LOW);
	}
	
	void set(bool ready) override {
		if (ready) {
			digitalWrite(PIN, LOW);
		}
		else {
			digitalWrite(PIN, HIGH);
		}
	}
};


#endif //ESP_READER_EMU_INCLUDE_EMUCLIENT_H
