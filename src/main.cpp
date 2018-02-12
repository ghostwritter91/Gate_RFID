#include <SPI.h>
#include <string.h>
#include <MFRC522.h>
#include "ESP8266WiFi.h"
#include <WiFiClient.h>
#include <DNSServer.h>
#include "ESP8266WebServer.h"
#include "my_mfrc522.h"
#include "my_config.h"
#include "my_eeprom.h"
#include "my_ap.h"
#include "main.h"

static String sReadValue = "";
static String message = "OK";

MFRC522 new_mfrc522(SS_PIN, RST_PIN);

/**
  * Initialize.
  */
void setup()
{
	/*Relay pin initialization*/
	pinMode(RELAY_PIN, OUTPUT);

#if (USE_DEBUG_PRINT == 1)	
	Serial.begin(115200); // Initialize serial communications with the PC
	while (!Serial)
		; // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
#endif
	EEPROM_Init();
	RFID_Init(new_mfrc522);
	/*Full Rx Gain 48 dB*/
	new_mfrc522.PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_max);
#if (USE_LED_INDICATION == 1)
	pinMode(LED_BUILTIN, OUTPUT); 
	digitalWrite(LED_BUILTIN, HIGH);
#endif	

	AP_Init();
}

/**
  * Main loop.
  */
void loop()
{
	/* Handle server mode*/
	if (AP_IsClientConnected())
	{
		AP_HandleClient();
		delay(10);
	}
	/* Handle normal operation*/
	else
	{
		message = ReadTag(&sReadValue);
		if(message == "Done") {
			if(EEPROM_CheckTag(sReadValue)) {
				OpenGate();
			}
		}
		delay(10);
	}
}

void OpenGate(void) {
	digitalWrite(RELAY_PIN, HIGH);
#if (USE_LED_INDICATION == 1)
	digitalWrite(LED_BUILTIN, LOW);
#endif
	delay(GATE_OPEN_TIME_MS);
#if (USE_LED_INDICATION == 1)
	digitalWrite(LED_BUILTIN, HIGH);
#endif
	digitalWrite(RELAY_PIN, LOW);
}