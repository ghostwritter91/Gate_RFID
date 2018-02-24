#ifndef _MY_EEPROM_H_
#define _MY_EEPROM_H_

#include <EEPROM.h>

#define MEM_SIZE 512      /* Memory size in bytes */
#define MEM_BLOCK_SIZE 16 /* Size of block memory reserved for one TAG in bytes*/
#define MAX_NUM_TAGS 31   /* Maximum number of tags */
#define BLOCKs_NUM 31     /* Number of blocks in memory */
#define FIRST_BLOCK_ADDR 0
#define LAST_ADDED_TAG_BLOCK_ADDR_IN_MEM 511  /* This is the last one address available and memory at this address stores block number of last added tag*/
#define ACTUAL_NUMBER_OF_TAGS_ADDR_IN_MEM 510 /* Memory at this address stores actual number of written tags */

#define ACTUAL_OPEN_TIME_ADDR_IN_MEM 509 /* Memory at this address sotres gate open time*/ 

void EEPROM_Init(void);
void EEPROM_Write(String str);
String EEPROM_DumpAllTags(void);    
bool EEPROM_CheckTag(String str);
void EEPROM_ClearAll(void);
void EEPROM_SaveOpenTime(uint8_t time);
uint8_t EEPROM_GetOpenTime(void);

#endif