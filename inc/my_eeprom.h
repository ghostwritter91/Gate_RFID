#ifndef _MY_EEPROM_H_
#define _MY_EEPROM_H_

#include <EEPROM.h>

#define MEM_SIZE 512
#define MEM_BLOCK 16
#define BLOCKs_NUM 31
#define FIRST_BLOCK_ADDR 0
/*This is the last one address available*/
#define ACTUAL_NUMBER_OF_TAGS_IN_MEM_ADDR 511

void EEPROM_Init(void);
void EEPROM_Write(String str);
String EEPROM_DumpAllTags(void);    

#endif