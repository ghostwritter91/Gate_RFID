#ifndef _MY_CONFIG_H_
#define _MY_CONFIG_H_

#define USE_DEBUG_PRINT 0
#define USE_DNS_SERVER 0

#if (USE_DEBUG_PRINT == 1)
    #define PRINT(txt) Serial.println(txt)
#else 
    #define PRINT(txt)
#endif

#endif