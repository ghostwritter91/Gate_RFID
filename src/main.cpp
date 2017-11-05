#include <SPI.h>
#include <string.h>
#include <MFRC522.h>
#include "my_mfrc522.h"
#include "my_config.h"
#include "ESP8266WiFi.h"
#include <WiFiClient.h>
#include <DNSServer.h>
#include "ESP8266WebServer.h"
#include "my_eeprom.h"

#define DEVICE_IP_1 192
#define DEVICE_IP_2 168
#define DEVICE_IP_3 0
#define DEVICE_IP_4 1

#define GATE_IP_1 192
#define GATE_IP_2 168
#define GATE_IP_3 0
#define GATE_IP_4 1

#define MASK_IP_1 255
#define MASK_IP_2 255
#define MASK_IP_3 255
#define MASK_IP_4 0

#define BYTES_TO_READ 18
#define BYTES_TO_WRITE 16

#define ATTEMPS_NUMBER 20

String getPage();
String ReadTag(String *str);
String WriteTag(String str);
String dump_byte_array_to_string(byte *buffer, byte bufferSize);

constexpr uint8_t RST_PIN = 0; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 15; // Configurable, see typical pin layout above
/* Reading & Writing Tags */
const byte SECTOR = 1;
const byte BLOCK_ADDR = 4;
const byte TRAILER_BLOCK = 7;

/* Set these to your desired credentials. */
const char *ssid = "Furtka";
const char *password = "unitrazodiak";

const byte DNS_PORT = 53;
const String SITE_ADDR = "www.furtka.io";
/* Server page variables */
const String RADIO_CHECK_NAME = "CHOICE";
const String RADIO_WRITE = "WRITE";
const String RADIO_READ = "READ";
const String RADIO_ADD_TAG = "ADD_TAG";
const String RADIO_CLEAR_ALL = "CLEAR_ALL";
const String RADIO_DUMP_ALL = "DUMP_ALL";

const String WRITE_TXT = "write_txt";
const String ADD_TXT = "add_txt";

String sReadValue = "";
String message = "OK";

DNSServer dnsServer;
ESP8266WebServer server(80);

void handleSubmit()
{
	String RadioValue, WriteValue, AddValue;
	RadioValue = server.arg(RADIO_CHECK_NAME);
	message = "OK";
	if (RADIO_WRITE == RadioValue)
	{
		WriteValue = server.arg(WRITE_TXT);
		for (byte i = 0; ((i < ATTEMPS_NUMBER) && (message != "Done")); i++)
		{
			message = WriteTag(WriteValue);
		}
	}
	else if (RADIO_READ == RadioValue)
	{
		for (byte i = 0; ((i < ATTEMPS_NUMBER) && (message != "Done")); i++)
		{
			message = ReadTag(&sReadValue);
		}
	}
	else if (RADIO_ADD_TAG == RadioValue) 
	{
		AddValue = server.arg(ADD_TXT);
		EEPROM_Write(AddValue);
	}
	else if (RADIO_CLEAR_ALL == RadioValue) 
	{
		EEPROM_ClearAll();
	}
	else if (RADIO_DUMP_ALL == RadioValue) 
	{
		message = EEPROM_DumpAllTags();
	}
	else
	{
		sReadValue = "No Tags";
	}
}

void handleRoot()
{
	if (server.hasArg(RADIO_CHECK_NAME))
	{
		handleSubmit();
	}
	server.send(200, "text/html", getPage());
}

void handleNotFound()
{
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++)
	{
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
}

MFRC522 new_mfrc522(SS_PIN, RST_PIN);

void dump_byte_array(byte *buffer, byte bufferSize);

/**
  * Initialize.
  */
void setup()
{
#if (USE_DEBUG_PRINT == 1)	
	Serial.begin(115200); // Initialize serial communications with the PC
	while (!Serial)
		; // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
#endif
	EEPROM_Init();
	RFID_Init(new_mfrc522);

	WiFi.mode(WIFI_AP_STA);
	WiFi.softAPConfig(IPAddress(DEVICE_IP_1, DEVICE_IP_2, DEVICE_IP_3, DEVICE_IP_4), IPAddress(GATE_IP_1, GATE_IP_2, GATE_IP_3, GATE_IP_4), IPAddress(MASK_IP_1, MASK_IP_2, MASK_IP_3, MASK_IP_4));
	WiFi.softAP(ssid, password);

#if (USE_DNS_SERVER == 1)
	// modify TTL associated  with the domain name (in seconds)
	// default is 60 seconds
	dnsServer.setTTL(300);
	// set which return code will be used for all other domains (e.g. sending
	// ServerFailure instead of NonExistentDomain will reduce number of queries
	// sent by clients)
	// default is DNSReplyCode::NonExistentDomain
	dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);

	// start DNS server for a specific domain name
	dnsServer.start(DNS_PORT, SITE_ADDR, IPAddress(DEVICE_IP_1, DEVICE_IP_2, DEVICE_IP_3, DEVICE_IP_4));
#endif

	server.on("/", handleRoot);
	server.onNotFound(handleNotFound);
	server.begin();
	PRINT("\n HTTP server started");
}

/**
  * Main loop.
  */
void loop()
{
	/* Handle service mode*/
	if (WiFi.softAPgetStationNum())
	{
#if (USE_DNS_SERVER == 1)
		dnsServer.processNextRequest();
#endif
		server.handleClient();
	}
	/* Handle normal operation*/
	else
	{
	}
}

/**
  * Helper routine to dump a byte array as hex values to Serial.
  */
void dump_byte_array(byte *buffer, byte bufferSize)
{
	for (byte i = 0; i < bufferSize; i++)
	{
		Serial.print(buffer[i] < 0x10 ? " 0" : " ");
		Serial.print(buffer[i], HEX);
	}
}

String getPage()
{
	String page = "<html lang=en-EN>";
	page += "<style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; } label {white-space: pre-wrap; } </style>";
	page += "<form action='/' method='POST'>";
	/*Read*/
	page += "READ TAG: <input type='text' name='read_txt' size='60' value='" + sReadValue + "'" + "readonly>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_READ + "'>" + "<br>";
	/*Write*/
	page += "WRITE TAG (max 16 chars): <input type='text' name='" + WRITE_TXT + "'>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_WRITE + "'>" + "<br>";
	/*Add tag*/
	page += "ADD TAG (max 16 chars): <input type='text' name='" + ADD_TXT + "'>";	
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_ADD_TAG + "'>" + "<br>";
	/*Clear all tags*/
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_CLEAR_ALL + "'>" + "CLEAR ALL" "<br>";
	/*Dump all*/
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_DUMP_ALL + "'>" + "DUMP ALL" "<br>";
	/*Submit button*/
	page += "<input type='submit' value='SUBMIT'></form><br>";
	page += "<label>" + message + "</label>";
	page += "</body></html>";
	return page;
}

String ReadTag(String *str)
{
	byte size = 18;
	byte buffer[BYTES_TO_READ];
	memset(buffer, '\0', size);
	*str = "";

	// Read data from TAG
	if (RFID_ReadTag(SECTOR, TRAILER_BLOCK, BLOCK_ADDR, buffer, &size))
	{
		*str = dump_byte_array_to_string(buffer, size);
		RFID_Stop();
		return "Done";
	}
	else
	{
		RFID_Stop();
		return "No Tags";
	}
}

String WriteTag(String str)
{
	byte size = 16;
	byte buffer[BYTES_TO_WRITE];
	memset(buffer, '\0', size);

	if (str.length() > size)
	{
		return "Too long";
	}
	str.getBytes(buffer, str.length() + 1, 0);

	// Write data to TAG
	if (RFID_WriteTag(SECTOR, TRAILER_BLOCK, BLOCK_ADDR, buffer, size))
	{
		RFID_Stop();
		return "Done";
	}
	else
	{
		RFID_Stop();
		return "No Tags";
	}
}

String dump_byte_array_to_string(byte *buffer, byte bufferSize)
{
	String temp = "";
	for (byte i = 0; i < bufferSize; i++)
	{
		temp += String((char)buffer[i]);
		if ('\0' == (char)buffer[i])
		{
			break;
		}
	}
	return temp;
}