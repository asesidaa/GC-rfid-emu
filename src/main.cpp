#include <Arduino.h>
#include "EspClient.h"
#include "PN532.h"
#include "Wire.h"
#include "PN532_SPI.h"

TaskHandle_t CardTask;
[[noreturn]]
void CardTaskCode([[maybe_unused]]void* pvParameters);

SPIClass spi{};

PN532_SPI pn532Spi{spi, 7};
PN532 nfc{pn532Spi};
EspDataClient dataClient;
EspSenseClient sense;	
JVSIO::LedClient led;
QueueHandle_t queue;

static const uint8_t START_PAGE = 3;
static const uint8_t END_PAGE = 9;
static const char id[] = "TAITO CORP.;RFID CTRL P.C.B.;Ver1.00;";
uint8_t cardData[] = { 0x04, 0xC2, 0x3D, 0xDA, 0x6F, 0x52, 0x80, 0x00, 0x37, 0x30, 0x32, 0x30, 0x33, 0x39, 
							   0x32, 0x30, 0x31, 0x30, 0x32, 0x38, 0x31, 0x35, 0x30, 0x32 };
bool cardIn = false;
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
	queue = xQueueCreate(1, sizeof(cardData));
	if (queue == nullptr) {
		Serial.println("Create queue failed");
		while (true) {
			delay(1000);
		} // halt
	}
	
	spi.begin(16, 17, 15, SS);
	nfc.begin();
	spi.setFrequency(2000000);

	uint32_t versionData = nfc.getFirmwareVersion();
	if (! versionData) {
		Serial.print("Didn't find PN53x board");
		while (true) {
			delay(1000);
		} // halt
	}
	// Got ok data, print it out!
	Serial.print("Found chip PN5"); Serial.println((versionData >> 24) & 0xFF, HEX);
	Serial.print("Firmware ver. "); Serial.print((versionData >> 16) & 0xFF, DEC);
	Serial.print('.'); Serial.println((versionData >> 8) & 0xFF, DEC);

	// configure board to read RFID tags
	nfc.SAMConfig();
	// nfc.setPassiveActivationRetries(0);

	xTaskCreatePinnedToCore(
		CardTaskCode, /* Function to implement the task */
		"Task1", /* Name of the task */
		10000,  /* Stack size in words */
		nullptr,  /* Task input parameter */
		0,  /* Priority of the task */
		&CardTask,  /* Task handle. */
		0); /* Core where the task should run */
	Serial.println("Card reader ready ...");
	
	io.begin();
	Serial.println("IO ready ...");
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
void loop() {
	uint8_t len;
	uint8_t* data = io.getNextCommand(&len);
	if (!data){
		return;
	}
		
	uint8_t buffer[24];
	
	auto cmd = static_cast<JVSIO::Cmd>(*data);
	if (cmd == JVSIO::Cmd::kCmdDriverInput) {
		cardIn = xQueueReceive(queue, buffer, 0) == pdPASS;
		memcpy(cardData, buffer, 24);
	}
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
		Serial.println("kCmdDriverInput");
		
		io.pushReport(JVSIO::kReportOk);
		for (auto i = 0; i < data[1]; ++i)
		{
			io.pushReport(cardIn ? 0x19 : 0);
		}
		break;
	case JVSIO::Cmd::kCmdDriverOutput:
		Serial.println("kCmdDriverOutput");
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
#pragma clang diagnostic pop



void CardTaskCode([[maybe_unused]]void* pvParameters) {
	uint8_t page[32];
	uint8_t success;
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
	uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
	uint8_t buffer[24] {};
	for (;;) {
		// Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
// 'uid' will be populated with the UID, and uidLength will indicate
// if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
		success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
		if (success && uidLength == 7) {
			int8_t offset = 0;
			for (int8_t i = START_PAGE; i < END_PAGE; ++i)
			{
				success = nfc.mifareultralight_ReadPage (i, page);
				if (success) {
					memcpy(buffer + offset, page, 4);
					offset += 4;
				}
				else {
					success = false;
					break;
				}
			}
			if (success) {
				dump("Data", buffer, 24);
				xQueueOverwrite(queue, buffer);
			}
		}
	}
}