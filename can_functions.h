#ifndef CAN_FUNCTIONS_H
#define CAN_FUNCTIONS_H

#include <Arduino.h>
#include <stdint.h>

// Прототип функции отправки CAN-сообщения
void sendCanMessage(uint32_t id, uint8_t dlc, const uint8_t *data);

// Прототипы функций для CAN‑операций
void messageInit();
void messageInitFIS();
void sysTimings();
void sysTimingsFIS();
void aprMessage(uint8_t messageCount);
void endMessage();
void dataFrame();
void processHex(uint8_t hexValue, uint8_t* outputArray, uint8_t &outputLength);
void processSpeed(uint8_t hexUW, uint8_t hexOW);
void processAir(String input);
void logging(uint32_t id, uint8_t dlc, const uint8_t *data);
void mfl(uint8_t mflValue);
bool isFilteredID(unsigned int id);

// Прототип функции heartBeat()
void heartBeat();
void checkHBState(uint32_t id, uint8_t dlc, const uint8_t *data);
void checkIgnitionState(uint32_t id, uint8_t dlc, const uint8_t *data);


#endif
