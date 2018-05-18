#include <SPI.h>
#include <MFRC522.h>
#include "my_config.h"

#define BYTES_TO_READ 18
#define BYTES_TO_WRITE 16

/* Open gate time*/
static uint8_t OpenTime_S = DEFAULT_GATE_OPEN_TIME_S;

/* Reading & Writing Tags */
static const byte SECTOR = 1;
static const byte BLOCK_ADDR = 4;
static const byte TRAILER_BLOCK = 7;

static MFRC522::MIFARE_Key key;
static MFRC522 mfrc522;

static bool RFID_ReadTag(byte sector, byte trailerBlock, byte blockAddr, byte *buffer, byte *bufferSize);
static bool RFID_WriteTag(byte sector, byte trailerBlock, byte blockAddr, byte *buffer, byte bufferSize);
static void RFID_Stop();
static String dump_byte_array_to_string(byte *buffer, byte bufferSize);

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

void OpenGate(void)
{
	digitalWrite(RELAY_PIN, HIGH);
#if (USE_LED_INDICATION == 1)
	digitalWrite(LED_BUILTIN, LOW);
#endif
	delay(OpenTime_S * 1000);
#if (USE_LED_INDICATION == 1)
	digitalWrite(LED_BUILTIN, HIGH);
#endif
	digitalWrite(RELAY_PIN, LOW);
}

void SetOpenTime(uint8_t time)
{
	OpenTime_S = time;
}

uint8_t GetOpenTime()
{
	return OpenTime_S;
}

void RFID_Init(MFRC522 new_mfrc522)
{
	SPI.begin(); // Init SPI bus
	mfrc522 = new_mfrc522;
	mfrc522.PCD_Init(); // Init MFRC522 card
						// Prepare the key (used both as key A and as key B)
						// using FFFFFFFFFFFFh which is the default at chip delivery from the factory
	for (byte i = 0; i < 6; i++)
	{
		key.keyByte[i] = 0xFF;
	}
}

/* sector to read, trailerBlock is used to authentication (often is last in reading sector)*/
static bool RFID_ReadTag(byte sector, byte trailerBlock, byte blockAddr, byte *buffer, byte *bufferSize)
{
	// Look for new cards
	if (!mfrc522.PICC_IsNewCardPresent())
	{
		return false;
	}

	// Select one of the cards
	if (!mfrc522.PICC_ReadCardSerial())
	{
		return false;
	}

	MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
	PRINT(mfrc522.PICC_GetTypeName(piccType));

	// Check for compatibility
	if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
	{
		PRINT(F("This sample only works with MIFARE Classic cards."));
		return false;
	}

	// Authenticate using key A
	PRINT(F("Authenticating using key A..."));
	MFRC522::StatusCode status;
	status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
	if (status != MFRC522::STATUS_OK)
	{
		PRINT(F("PCD_Authenticate() failed: "));
		PRINT(mfrc522.GetStatusCodeName(status));
		return false;
	}

	// Show the whole sector as it currently is
	PRINT(F("Current data in sector:"));
	mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
	PRINT();

	// Read data from the block
	PRINT(F("Reading data from block "));
	PRINT(blockAddr);
	PRINT(F(" ..."));
	status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, bufferSize);
	if (status != MFRC522::STATUS_OK)
	{
		PRINT(F("MIFARE_Read() failed: "));
		PRINT(mfrc522.GetStatusCodeName(status));
		return false;
	}
	return true;
}

/* sector to read, trailerBlock is used to authentication (often is last in reading sector)*/
static bool RFID_WriteTag(byte sector, byte trailerBlock, byte blockAddr, byte *buffer, byte bufferSize)
{
	// Look for new cards
	if (!mfrc522.PICC_IsNewCardPresent())
	{
		return false;
	}

	// Select one of the cards
	if (!mfrc522.PICC_ReadCardSerial())
	{
		return false;
	}

	MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
	PRINT(mfrc522.PICC_GetTypeName(piccType));

	// Check for compatibility
	if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
	{
		PRINT(F("This sample only works with MIFARE Classic cards."));
		return false;
	}

	// Authenticate using key B
	PRINT(F("Authenticating again using key B..."));
	MFRC522::StatusCode status;
	status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
	if (status != MFRC522::STATUS_OK)
	{
		PRINT(F("PCD_Authenticate() failed: "));
		PRINT(mfrc522.GetStatusCodeName(status));
		return false;
	}

	// Show the whole sector as it currently is
	PRINT(F("Current data in sector:"));
	mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);

	// Write data to the block
	PRINT(F("Writing data into block "));
	PRINT(blockAddr);
	PRINT(F(" ..."));
	status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(blockAddr, buffer, bufferSize);
	if (status != MFRC522::STATUS_OK)
	{
		PRINT(F("MIFARE_Write() failed: "));
		PRINT(mfrc522.GetStatusCodeName(status));
		return false;
	}
	return true;
}

static void RFID_Stop()
{
	// Halt PICC
	mfrc522.PICC_HaltA();
	// Stop encryption on PCD
	mfrc522.PCD_StopCrypto1();
}

static String dump_byte_array_to_string(byte *buffer, byte bufferSize)
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