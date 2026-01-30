#include "globals.h"

// CAN‑объекты
struct can_frame rxMsg, txMsg;
MCP2515 mcp2515(10);

// Флаги и состояния
bool responseAwait = false;
bool messageAwait = false;
bool heartbeatAwait = false;
bool hbregistered = false;
uint8_t ddpChannel = 0xFF;
uint8_t ignitionChannel = 0x271;
bool chanelStatus = false;

// Константный массив в PROGMEM
const unsigned int FILTERED_IDS[] PROGMEM = { 0x4D9, 0x2E8, 0x6B9, 0x699, 0x439, 0x436};
const int FILTERED_COUNT = sizeof(FILTERED_IDS) / sizeof(FILTERED_IDS[0]);

// Тайминги
unsigned long lastHeartBeat = 0;
unsigned long delayHeartBeat = 50;
unsigned long lastSendMessage = 0;
unsigned long lastReceivedMessaage = 0;
unsigned long responseMessagesMax = 500;
unsigned long delayMessages = 1;

// Состояния
ProgramState progState = DDP_Delete;
MessageState msgState = M_Init;
HeartBeatState hbState = h_Registry;
IgnitionState ignState = i_Off;

// Прочие переменные
uint8_t inputHex = 0xB6;
uint8_t outputArray[4];
uint8_t outputLength = 0;

uint8_t speedHEX[3] = {0x20, 0x20, 0x20};
uint8_t intakeAir[3] = {0x20, 0x20, 0x20};
uint8_t oilTemp[3] = {0x20, 0x20, 0x20};
uint8_t boost[3] = {0x20, 0x20, 0x20};
uint8_t colTemp[3] = {0x20, 0x20, 0x20};
