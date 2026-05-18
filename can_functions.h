#ifndef CAN_FUNCTIONS_H
#define CAN_FUNCTIONS_H

#include <Arduino.h>
#include <stdint.h>
struct can_frame;

// Set to 0 if Nano/Uno still does not fit. Protocol state logs stay enabled.
#ifndef RAW_CAN_LOG
#define RAW_CAN_LOG 1
#endif

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

// Compact debug / protocol helpers
void printHex2(uint8_t v);
void dbgState();
void dbgTimeout(uint8_t code);
void dbgDdp(uint8_t code, const struct can_frame &frame);
void dbgErr(uint8_t code, const struct can_frame &frame);
bool isTachoTpRequest(const struct can_frame &frame);
bool isTachoTimingRequest(const struct can_frame &frame);
bool isDdpStatusOrError(const struct can_frame &frame);
void startFisSessionFromTacho();

// Прототип функции heartBeat()
void heartBeat();
void checkHBState(uint32_t id, uint8_t dlc, const uint8_t *data);
void checkIgnitionState(uint32_t id, uint8_t dlc, const uint8_t *data);


#endif
