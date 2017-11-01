#ifndef _MY_MFRC522_H_
#define _MY_MFRC522_H_

#include <stdbool.h>

void RFID_Init(MFRC522 new_mfrc522);
bool RFID_ReadTag(byte sector, byte trailerBlock, byte blockAddr, byte *buffer, byte *bufferSize);
bool RFID_WriteTag(byte sector, byte trailerBlock, byte blockAddr, byte *buffer, byte bufferSize);
void RFID_Stop();

#endif 