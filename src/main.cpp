#include <SPI.h>
#include <string.h>
#include <MFRC522.h>
#include <Ticker.h>
#include "my_mfrc522.h"
#include "my_config.h"
#include "my_eeprom.h"
#include "my_ap.h"

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
	new_mfrc522.PCD_Init();
	/*Full Rx Gain 48 dB*/
	new_mfrc522.PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_max);
	new_mfrc522.PCD_AntennaOn();

#if (USE_LED_INDICATION == 1)
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
#endif

	SetOpenTime(EEPROM_GetOpenTime());

	if (GetOpenTime() == (uint8_t)EMPTY_MEMORY)
	{
		SetOpenTime(DEFAULT_GATE_OPEN_TIME_S);
	}

	AP_On();
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
		if (message == "Done")
		{
			if (sReadValue == CONFIG_TAG_NAME)
			{
				AP_On();
			}
			else if (EEPROM_CheckTag(sReadValue))
			{
				OpenGate();
			}
		}
		delay(10);
	}
	/* This function gets wifi on requests and manages wifi off time*/
	AP_HandleOnOffRequest();
}
