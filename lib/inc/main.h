#ifndef _MAIN_H_
#define _MAIN_H_

#define GATE_OPEN_TIME_MS 3000

constexpr uint8_t RST_PIN = D3; 
constexpr uint8_t SS_PIN = D8;
constexpr uint8_t RELAY_PIN = D1;

void OpenGate(void);
void SetOpenTime(uint8_t time);

#endif