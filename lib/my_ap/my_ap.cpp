#include "ESP8266WiFi.h"
#include <WiFiClient.h>
#include <DNSServer.h>
#include <Ticker.h>
#include "ESP8266WebServer.h"
#include "my_eeprom.h"
#include "my_config.h"
#include "my_mfrc522.h"
#include "my_ap.h"

/* Ticker instance for wifi timeout*/
static Ticker wifi_ticker;
static volatile bool wifi_flag_onoff = false;

/* Set these to your desired credentials. */
static const char ssid[] = AP_SSID;
static const char password[] = AP_PASS;

/* Server page variables */
static const String RADIO_CHECK_NAME = "CHOICE";
static const String RADIO_WRITE = "WRITE";
static const String RADIO_READ = "READ";
static const String RADIO_ADD_TAG = "ADD_TAG";
static const String RADIO_CHANGE_TIME = "CHANGE_TIME";
static const String RADIO_GET_TIME = "GET_TIME";
static const String RADIO_CLEAR_ALL = "CLEAR_ALL";
static const String RADIO_DUMP_ALL = "DUMP_ALL";
static const String RADIO_IS_IN_TABLE = "IS_IN_TABLE";
static const String RADIO_OPEN_DOOR = "OPEN_DOOR";

static const String WRITE_TXT = "write_txt";
static const String ADD_TXT = "add_txt";
static const String OPEN_TIME_S = "3";
static const String IS_IN_TABLE_TXT = "is_in_table_txt";

static String sReadTagValue = "";
static String sTimeOpenS = "";
static String message = "OK";

static const byte DNS_PORT = 53;
static const String SITE_ADDR = AP_SITE_ADDR;

static bool IsClientConnected = false;

static DNSServer dnsServer;
static ESP8266WebServer server(80);
static WiFiEventHandler APStationConnectedHandler, APStationDisconnectedHandler;

static void AP_Init();
static void AP_TimeOut();
static void handleSubmit();
static void handleRoot();
static void handleNotFound();
static String getPage();
static void AP_DisableWiFi();
static void AP_EnableWiFi();

void AP_HandleOnOffRequest()
{
	static bool onoff = false;

	if (onoff == false && wifi_flag_onoff == true)
	{
		onoff = true;
		AP_EnableWiFi();
		wifi_ticker.once(AP_ENABLE_TIMEOUT, AP_TimeOut);
	}
	else if (onoff == true && wifi_flag_onoff == false)
	{
		onoff = false;
		AP_DisableWiFi();
	}
}

void AP_On()
{
	wifi_flag_onoff = true;
}

void AP_Off()
{
	wifi_flag_onoff = false;
}

void AP_HandleClient()
{
#if (USE_DNS_SERVER == 1)
	dnsServer.processNextRequest();
#endif
	server.handleClient();
}

bool AP_IsClientConnected()
{
	return IsClientConnected;
}

static void AP_TimeOut()
{
	if (!AP_IsClientConnected())
	{
		wifi_flag_onoff = false;
	}
	else
	{
		wifi_ticker.once(AP_ENABLE_TIMEOUT, AP_TimeOut);
	}
}

static void AP_Init()
{
	WiFi.mode(WIFI_AP);
	WiFi.setOutputPower(5);
	WiFi.softAPConfig(IPAddress(DEVICE_IP_1, DEVICE_IP_2, DEVICE_IP_3, DEVICE_IP_4), IPAddress(GATE_IP_1, GATE_IP_2, GATE_IP_3, GATE_IP_4), IPAddress(MASK_IP_1, MASK_IP_2, MASK_IP_3, MASK_IP_4));
	WiFi.softAP(ssid, password);

	APStationConnectedHandler = WiFi.onSoftAPModeStationConnected([](const WiFiEventSoftAPModeStationConnected &evt) {
		IsClientConnected = true;
	});

	APStationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected([](const WiFiEventSoftAPModeStationDisconnected &evt) {
		IsClientConnected = false;
	});

#if (USE_DNS_SERVER == 1)
	// modify TTL associated  with the domain name (in seconds)
	// default is 60 seconds
	dnsServer.setTTL(60);
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

static void AP_DisableWiFi()
{
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1);
	WiFi.disconnect(true);
	delay(1);
}

static void AP_EnableWiFi()
{
	WiFi.forceSleepWake();
	AP_Init();
}

static void handleSubmit()
{
	String RadioValue, TxtValue, OpenTime;
	uint8_t u8OpenTime = 0;
	RadioValue = server.arg(RADIO_CHECK_NAME);
	message = "OK";
	if (RADIO_WRITE == RadioValue)
	{
		TxtValue = server.arg(WRITE_TXT);
		for (byte i = 0; ((i < ATTEMPS_RW_NUMBER_WEBSITE) && (message != "Done")); i++)
		{
			message = WriteTag(TxtValue);
			delay(10);
		}
	}
	else if (RADIO_READ == RadioValue)
	{
		for (byte i = 0; ((i < ATTEMPS_RW_NUMBER_WEBSITE) && (message != "Done")); i++)
		{
			message = ReadTag(&sReadTagValue);
			delay(10);
		}
	}
	else if (RADIO_ADD_TAG == RadioValue)
	{
		TxtValue = server.arg(ADD_TXT);
		EEPROM_Write(TxtValue);
	}
	else if (RADIO_CHANGE_TIME == RadioValue)
	{
		OpenTime = server.arg(OPEN_TIME_S);
		u8OpenTime = (uint8_t)OpenTime.toInt();
		EEPROM_SaveOpenTime(u8OpenTime);
		SetOpenTime(u8OpenTime);
	}
	else if (RADIO_GET_TIME == RadioValue)
	{
		sTimeOpenS = String(GetOpenTime());
	}
	else if (RADIO_CLEAR_ALL == RadioValue)
	{
		EEPROM_ClearAll();
	}
	else if (RADIO_DUMP_ALL == RadioValue)
	{
		message = EEPROM_DumpAllTags();
	}
	else if (RADIO_IS_IN_TABLE == RadioValue)
	{
		TxtValue = server.arg(IS_IN_TABLE_TXT);
		if (EEPROM_CheckTag(TxtValue))
		{
			message = "Tag in table";
		}
		else
		{
			message = "Unknow tag";
		}
	}
	else if (RADIO_OPEN_DOOR == RadioValue)
	{
		OpenGate();
	}
	else
	{
		sReadTagValue = "No Tags";
	}
}

static void handleRoot()
{
	if (server.hasArg(RADIO_CHECK_NAME))
	{
		handleSubmit();
	}
	server.send(200, "text/html", getPage());
}

static void handleNotFound()
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

static String getPage()
{
	String page = "<html lang=en-EN>";
	page += "<style> label {white-space: pre-wrap; } </style>";
	page += "<form action='/' method='POST'>";
	/*Read*/
	page += "READ TAG: <input type='text' name='read_txt' size='60' value='" + sReadTagValue + "'" + "readonly>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_READ + "'>" + "<br><br>";
	/*Write*/
	page += "WRITE TAG (max 16 chars): <input type='text' name='" + WRITE_TXT + "'>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_WRITE + "'>" + "<br><br>";
	/*Add tag*/
	page += "ADD TAG (max 16 chars): <input type='text' name='" + ADD_TXT + "'>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_ADD_TAG + "'>" + "<br><br>";
	/*Change open time*/
	page += "CHANGE OPEN TIME (1-255) [s]: <input type='number' min='1' max='255' step='1' name='" + OPEN_TIME_S + "'>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_CHANGE_TIME + "'>" + "<br><br>";
	/*Get open time*/
	page += "GET OPEN TIME [s]: <input type='text' name='read_txt' size='15' value='" + sTimeOpenS + "'" + "readonly>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_GET_TIME + "'>" + "<br><br>";
	/*Clear all tags*/
	page += "<label>CLEAR ALL</label>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_CLEAR_ALL + "'>" + "<br><br>";
	/*Dump all*/
	page += "<label>DUMP ALL</label>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_DUMP_ALL + "'>" + "<br><br>";
	/*Is tag in table*/
	page += "IS TAG IN TABLE (max 16 chars): <input type='text' name='" + IS_IN_TABLE_TXT + "'>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_IS_IN_TABLE + "'>" + "<br><br>";
	/*Open door*/
	page += "<label>OPEN DOOR</label>";
	page += "<input type='radio' name='" + RADIO_CHECK_NAME + "'" + "value='" + RADIO_OPEN_DOOR + "'>" + "<br><br>";
	/*Submit button*/
	page += "<input type='submit' value='SUBMIT'></form><br>";
	page += "<label>" + message + "</label>";
	page += "</body></html>";
	return page;
}
