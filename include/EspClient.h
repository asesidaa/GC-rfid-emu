#ifndef ESP_READER_EMU_INCLUDE_ESPCLIENT_H
#define ESP_READER_EMU_INCLUDE_ESPCLIENT_H

#include "JVSIO.h"
#include "driver/uart.h"

class EspDataClient final : public JVSIO::DataClient {
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

class EspSenseClient final : public JVSIO::SenseClient {
	void begin() override {
		pinMode(1, OUTPUT);
		digitalWrite(1, LOW);
	}
	
	void set(bool ready) override {
		if (ready) {
			digitalWrite(1, LOW);
		}
		else {
			digitalWrite(1, HIGH);
		}
	}
};


#endif //ESP_READER_EMU_INCLUDE_ESPCLIENT_H
