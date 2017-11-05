#include <EEPROM.h>
#include "Arduino.h"
#include "my_eeprom.h"
#include "my_config.h"

#define CONVERT_BYTE_TO_ASCII(num) (num + 0x30)

static byte EEPROM_ReadActualTagBlockNumber(void);
static bool EEPROM_CompareBuffers(byte buff1[], byte buff2[]);
static byte EEPROM_ReadActualNumberOfTags(void);    
static void EEPROM_ReadTagBytesFromBlock(byte buff[], byte block_num);

void EEPROM_Init(void) {
    EEPROM.begin(MEM_SIZE);
}

void EEPROM_Write(String str) {
    byte buffer[MEM_BLOCK_SIZE];
    uint8_t writing_byte_num = 0;
    byte block_num = EEPROM_ReadActualTagBlockNumber();
    byte tags_num = EEPROM_ReadActualNumberOfTags();
    if(MAX_NUM_TAGS > tags_num) {
        tags_num++;
    }
    if(BLOCKs_NUM <= block_num) {
        block_num = FIRST_BLOCK_ADDR;
    }
    /* Save actual block number in memory */
    EEPROM.write(LAST_ADDED_TAG_BLOCK_ADDR_IN_MEM, (block_num + 1));
    /* Save actual number of tags in memory */
    EEPROM.write(ACTUAL_NUMBER_OF_TAGS_ADDR_IN_MEM, tags_num);
    uint16 tag_addr = block_num * MEM_BLOCK_SIZE;
    memset(buffer, '\0', MEM_BLOCK_SIZE);
    str.getBytes(buffer, str.length() + 1, 0);
    for(uint16 i = tag_addr; i < (tag_addr + MEM_BLOCK_SIZE); ++i) {
        EEPROM.write(i, buffer[writing_byte_num]);
        writing_byte_num++;
    }
    EEPROM.commit();
}

String EEPROM_DumpAllTags(void) {
    String str = "";
    uint16_t num_of_bytes_to_dump = EEPROM_ReadActualNumberOfTags() * MEM_BLOCK_SIZE;
    /* Dump everything whitout last block which includes current block and numbers of tags*/
    for(uint16_t i = 0; i < num_of_bytes_to_dump; ++i) {
        str += String( (char)EEPROM.read(i) );
        /* Put EOL sign on every end of block */
        if( (0 == (i % (MEM_BLOCK_SIZE - 1))) && (i != 0) ) {
            str += "\n";
        }
    }
    str += "Actual block: ";
    str += String( (char)  CONVERT_BYTE_TO_ASCII( EEPROM_ReadActualTagBlockNumber() ) );
    str += "\n";
    str += "Actual number of tags: ";
    str += String( (char)  CONVERT_BYTE_TO_ASCII( EEPROM_ReadActualNumberOfTags() ) );
    str += "\n";
    return str;
}

bool EEPROM_CheckTag(String str) {
    byte in_buffer[MEM_BLOCK_SIZE];
    byte mem_buffer[MEM_BLOCK_SIZE];
    byte actual_number_of_tags = EEPROM_ReadActualNumberOfTags();
    memset(in_buffer, '\0', MEM_BLOCK_SIZE);
    str.getBytes(in_buffer, str.length() + 1, 0);
    for(uint8_t i = 0; i < actual_number_of_tags; ++i) {
        EEPROM_ReadTagBytesFromBlock(mem_buffer, i);
        if(EEPROM_CompareBuffers(in_buffer, mem_buffer)) {
            return true;
        }
    }
    return false;
}

void EEPROM_ClearAll(void) {
    /* Reset values about number of tags in memory*/
    EEPROM.write(LAST_ADDED_TAG_BLOCK_ADDR_IN_MEM, 0);
    EEPROM.write(ACTUAL_NUMBER_OF_TAGS_ADDR_IN_MEM, 0);
    /* Clear all but not block number and actual tag number */
    for(uint16_t i = 0; i < MEM_SIZE - MEM_BLOCK_SIZE; ++i) {
        EEPROM.write(i, '\0');
    }
    EEPROM.commit();
}

static bool EEPROM_CompareBuffers(byte buff1[], byte buff2[]) {
    uint8_t cnt = 0;
    for(uint8_t i = 0; i < MEM_BLOCK_SIZE; ++i) {
        if(buff1[i] == buff2[i]) {
            cnt++;
        }
    }
    if(cnt == MEM_BLOCK_SIZE) {
        return true;
    }
    else {
        return false;
    }
}

static byte EEPROM_ReadActualTagBlockNumber(void) {
    return EEPROM.read(LAST_ADDED_TAG_BLOCK_ADDR_IN_MEM);
}

static byte EEPROM_ReadActualNumberOfTags(void) {
    return EEPROM.read(ACTUAL_NUMBER_OF_TAGS_ADDR_IN_MEM);
}

static void EEPROM_ReadTagBytesFromBlock(byte buff[], byte block_num) {
    uint16_t addr = block_num * MEM_BLOCK_SIZE;
    for(uint16_t i = addr, j = 0; i < (addr + MEM_BLOCK_SIZE); ++i, ++j) {
        buff[j] = EEPROM.read(i);
    }
}