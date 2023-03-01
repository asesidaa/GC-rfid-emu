#include <Arduino.h>
#include "EspClient.h"

EspDataClient dataClient;
EspSenseClient sense;
JVSIO::LedClient led;

static const char id[] = "TAITO CORP.;RFID CTRL P.C.B.;Ver1.00;";
static const uint8_t cardData[] = { 0x04, 0xC2, 0x3D, 0xDA, 0x6F, 0x52, 0x80, 0x00, 0x37, 0x30, 0x32, 0x30, 0x33, 0x39, 
							   0x32, 0x30, 0x31, 0x30, 0x32, 0x38, 0x31, 0x35, 0x30, 0x32 };
JVSIO io(&dataClient, &sense, &led);

void dump(const char* str, uint8_t* data, size_t len) {
	// TODO : do Serial.begin();
	// for Arduino series which have native USB CDC (=Serial),
	//   such as Leonardo, ProMicro, etc.
	Serial.print(str);
	Serial.print(": ");
	for (size_t i = 0; i < len; ++i) {
		if (data[i] < 16)
			Serial.print("0");
		Serial.print(data[i], HEX);
		Serial.print(" ");
	}
	Serial.println("");
}

void setup() {
	Serial.begin(115200);
	Serial1.begin(115200);
	Serial1.setPins(2, 42, -1, -1);
	pinMode(41, INPUT);
	
	io.begin();
}
bool cardIn = false;
void loop() {
	auto pressed = (digitalRead(41) == HIGH);
	if (pressed) {
		cardIn = true;
	}
	uint8_t len;
	uint8_t* data = io.getNextCommand(&len);
	if (!data){
		return;
	}
		
	
	auto cmd = static_cast<JVSIO::Cmd>(*data);
	switch (cmd) {
	case JVSIO::Cmd::kCmdIoId:
		Serial.println("GetID");
		io.pushReport(JVSIO::kReportOk);
		for (size_t i = 0; id[i]; ++i)
			io.pushReport(id[i]);
		io.pushReport(0);
		break;
	case JVSIO::Cmd::kCmdFunctionCheck:
		Serial.println("FunctionCheck");
		io.pushReport(JVSIO::kReportOk);

		io.pushReport(0x01);  // sw
		io.pushReport(0x07);  // players
		io.pushReport(0x00);  // buttons
		io.pushReport(0x08);
		io.pushReport(0x00);

		io.pushReport(0x12);  // general purpose driver
		io.pushReport(0x08);  // slots
		io.pushReport(0x00);
		io.pushReport(0x00);

		io.pushReport(0x00);
		break;
	case JVSIO::Cmd::kCmdSwInput:
		Serial.println("kCmdSwInput");
		io.pushReport(JVSIO::kReportOk);
		io.pushReport(0x00);  // TEST, TILT1-3, and undefined x4.
		for (size_t player = 0; player < data[1]; ++player) {
			for (size_t line = 1; line <= data[2]; ++line)
				io.pushReport(0x00);
		}
		break;
	case JVSIO::Cmd::kCmdCoinInput:
		Serial.println("kCmdCoinInput");
		io.pushReport(JVSIO::kReportOk);
		for (size_t slot = 0; slot < data[1]; ++slot) {
			io.pushReport(0x00);
			io.pushReport(0x00);
		}
		break;
	case JVSIO::Cmd::kCmdAnalogInput:
		Serial.println("kCmdAnalogInput");
		io.pushReport(JVSIO::kReportOk);
		for (size_t channel = 0; channel < data[1]; ++channel) {
			io.pushReport(0x80);
			io.pushReport(0x00);
		}
		break;
	case JVSIO::Cmd::kCmdCoinSub:
	case JVSIO::Cmd::kCmdCoinAdd:
		io.pushReport(JVSIO::kReportOk);
		break;
	case JVSIO::Cmd::kCmdDriverInput:
		dump("kCmdDriverInput", data, 10);
		
		io.pushReport(JVSIO::kReportOk);
		for (auto i = 0; i < data[1]; ++i)
		{
			io.pushReport(cardIn ? 0x19 : 0);
		}
		break;
	case JVSIO::Cmd::kCmdDriverOutput:
		dump("kCmdDriverOutput", data, 10);
		io.pushReport(JVSIO::kReportOk);
		for (unsigned char i : cardData)
		{
			io.pushReport(cardIn ? i: 0);
		}
		io.pushReport(JVSIO::kReportOk);
		cardIn = false;
		break;
	case JVSIO::Cmd::kCmdTaito01:
	case JVSIO::Cmd::kCmdTaito03:
		Serial.println("kCmdTaito01/3");
		io.pushReport(JVSIO::kReportOk);
		io.pushReport(1);
		break;
	case JVSIO::Cmd::kCmdTaito04:
	case JVSIO::Cmd::kCmdTaito05:
	case JVSIO::Cmd::kCmdHandlePayout:
		Serial.println("kCmdTaito04/5");
		io.pushReport(JVSIO::kReportOk);
		break;

	}
}