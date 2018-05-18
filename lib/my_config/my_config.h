#ifndef _MY_CONFIG_H_
#define _MY_CONFIG_H_

#include <pins_arduino.h>

#define USE_DEBUG_PRINT 0
#define USE_DNS_SERVER 1
#define USE_LED_INDICATION 1

#define ATTEMPS_RW_NUMBER_WEBSITE 100

#if (USE_DEBUG_PRINT == 1)
    #define PRINT(txt) Serial.println(txt)
#else 
    #define PRINT(txt)
#endif

#define DEFAULT_GATE_OPEN_TIME_S 3
#define EMPTY_MEMORY 0xFFFFFFFF
#define AP_ENABLE_TIMEOUT 180
/* After read of this tag WiFi is enable during AP_ENABLE_TIMEOUT time*/
#define CONFIG_TAG_NAME "CONFIG"

#define RST_PIN D3 
#define SS_PIN D8
#define RELAY_PIN D1

#endif