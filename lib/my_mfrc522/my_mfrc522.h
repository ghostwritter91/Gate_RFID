#ifndef _MY_MFRC522_H_
#define _MY_MFRC522_H_

#include <MFRC522.h>

void RFID_Init(MFRC522 new_mfrc522);
String ReadTag(String *str);
String WriteTag(String str);
void OpenGate(void);
void SetOpenTime(uint8_t time);
uint8_t GetOpenTime();

#endif 