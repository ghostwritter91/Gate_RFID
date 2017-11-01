#include <SPI.h>
#include <MFRC522.h>
#include "my_config.h"

MFRC522::MIFARE_Key key;

MFRC522 mfrc522;

void RFID_Init(MFRC522 new_mfrc522) {
    SPI.begin();                // Init SPI bus
    mfrc522 = new_mfrc522;
    mfrc522.PCD_Init(); // Init MFRC522 card
    // Prepare the key (used both as key A and as key B)
	// using FFFFFFFFFFFFh which is the default at chip delivery from the factory
	 for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
}

/* sector to read, trailerBlock is used to authentication (often is last in reading sector)*/
bool RFID_ReadTag(byte sector, byte trailerBlock, byte blockAddr, byte *buffer, byte *bufferSize) {
     // Look for new cards
	 if ( ! mfrc522.PICC_IsNewCardPresent()) {
        return false;
     }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
        return false;
    }

    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    PRINT(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
	if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        PRINT(F("This sample only works with MIFARE Classic cards."));
        return false;
    }

    // Authenticate using key A
     PRINT(F("Authenticating using key A..."));
     MFRC522::StatusCode status;
	 status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
	 if (status != MFRC522::STATUS_OK) {
		 Serial.print(F("PCD_Authenticate() failed: "));
		 PRINT(mfrc522.GetStatusCodeName(status));
		 return false;
     }

    // Show the whole sector as it currently is
	 PRINT(F("Current data in sector:"));
	 mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
	 PRINT();
 
	 // Read data from the block
	 Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
	 PRINT(F(" ..."));
	 status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, bufferSize);
	 if (status != MFRC522::STATUS_OK) {
		 Serial.print(F("MIFARE_Read() failed: "));
         PRINT(mfrc522.GetStatusCodeName(status));
         return false;
     }
    return true;
    PRINT();
}


/* sector to read, trailerBlock is used to authentication (often is last in reading sector)*/
bool RFID_WriteTag(byte sector, byte trailerBlock, byte blockAddr, byte *buffer, byte bufferSize) {
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
        return false;
    }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
        return false;
    }

    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    PRINT(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        PRINT(F("This sample only works with MIFARE Classic cards."));
        return false;
    }

	 // Authenticate using key B
     PRINT(F("Authenticating again using key B..."));
     MFRC522::StatusCode status;     
	 status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
	 if (status != MFRC522::STATUS_OK) {
		 Serial.print(F("PCD_Authenticate() failed: "));
		 PRINT(mfrc522.GetStatusCodeName(status));
		 return false;
	 }

    // Show the whole sector as it currently is
    PRINT(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    PRINT();
 
	// Write data to the block
	Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
	PRINT(F(" ..."));
	status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, buffer, bufferSize);
	if (status != MFRC522::STATUS_OK) {
		 Serial.print(F("MIFARE_Write() failed: "));
         PRINT(mfrc522.GetStatusCodeName(status));
         return false;
    }
    return true;
	PRINT();
}

void RFID_Stop() {
    // Halt PICC
	 mfrc522.PICC_HaltA();
	 // Stop encryption on PCD
	 mfrc522.PCD_StopCrypto1();
}