#ifndef _MY_AP_H_
#define _MY_AP_H_

#define DEVICE_IP_1 192
#define DEVICE_IP_2 168
#define DEVICE_IP_3 0
#define DEVICE_IP_4 1

#define GATE_IP_1 192
#define GATE_IP_2 168
#define GATE_IP_3 0
#define GATE_IP_4 1

#define MASK_IP_1 255
#define MASK_IP_2 255
#define MASK_IP_3 255
#define MASK_IP_4 0

#define AP_SSID "Furtka"
#define AP_PASS "unitrazodiak"
#define AP_SITE_ADDR "www.furtka.io"

void AP_HandleOnOffRequest();
bool AP_IsClientConnected();
void AP_HandleClient();
void AP_Off();
void AP_On();

#endif