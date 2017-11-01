#include <EEPROM.h>
#include "Arduino.h"
#include "my_eeprom.h"
#include "my_config.h"

static byte EEPROM_ReadActualNumberOfTags(void);

void EEPROM_Init(void) {
    EEPROM.begin(MEM_SIZE);
}

void EEPROM_Write(String str) {
    byte buffer[MEM_BLOCK];
    uint8_t writing_byte_num = 0;
    byte tags_num = EEPROM_ReadActualNumberOfTags() + 1;
    if(BLOCKs_NUM == tags_num) {
        tags_num = 0;
    }
    uint16 tag_addr = tags_num * MEM_BLOCK;
    EEPROM.write(ACTUAL_NUMBER_OF_TAGS_IN_MEM_ADDR, tags_num);
    memset(buffer, '\0', 0);
    str.getBytes(buffer, str.length() + 1, 0);
    for(uint16 i = tag_addr; i < (tag_addr + MEM_BLOCK); ++i) {
        EEPROM.write(i, buffer[writing_byte_num]);
        writing_byte_num++;
    }
    EEPROM.commit();
}

String EEPROM_DumpAllTags(void) {
    String str = "";
    for(uint16_t i = 0; i < MEM_SIZE; ++i) {
        str += String( (char)EEPROM.read(i) );
    }
    return str;
}

static byte EEPROM_ReadActualNumberOfTags(void) {
    return EEPROM.read(ACTUAL_NUMBER_OF_TAGS_IN_MEM_ADDR);
}