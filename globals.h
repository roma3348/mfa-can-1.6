#ifndef GLOBALS_H
#define GLOBALS_H

#include <SPI.h>
#include <mcp2515.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

// Предполагается, что структура can_frame уже определена в mcp2515.h
struct can_frame;

// Глобальные объекты (extern)
extern struct can_frame rxMsg, txMsg;
extern MCP2515 mcp2515;

// Перечисления состояний
enum ProgramState {
  DDP_Delete,
  Segment_Request,
  Chanel_Create,
  Chanel_Create_Upper,
  ID_Prior_Upper,
  Data_Send,
  Data_Send_Upper,
  FisToCan,
  Request_Await
};
enum MessageState { M_Init, Sys_Info, Data_Info, Data_Info2, Data_Info3, Status_Req, End_Wait };
enum HeartBeatState { h_Registry, h_Operate, h_Standby, h_Error };
enum IgnitionState { i_Off, i_Operate, i_Start };

// Глобальные переменные
extern bool responseAwait;
extern bool messageAwait;
extern bool heartbeatAwait;
extern bool hbregistered;
extern uint8_t ddpChannel;
extern uint8_t ddpChannelUpper;
extern uint8_t ignitionChannel;
extern bool chanelStatus;
extern bool mainChannelReady;
extern bool upperChannelReady;
extern uint8_t activeDdpChannel;
extern ProgramState lastLoggedProgState;
extern MessageState lastLoggedMsgState;

// Константы в PROGMEM
extern const unsigned int FILTERED_IDS[] PROGMEM;
extern const int FILTERED_COUNT;

// Тайминги
extern unsigned long lastHeartBeat;
extern unsigned long delayHeartBeat;
extern unsigned long lastSendMessage;
extern unsigned long lastReceivedMessaage;
extern unsigned long responseMessagesMax;
extern unsigned long delayMessages;
extern unsigned long lastMainUpdate;
extern unsigned long lastUpperUpdate;
extern unsigned long mainMinInterval;
extern unsigned long upperMinInterval;
extern unsigned long displayCooldownUntil;
extern unsigned long errorCooldownInterval;

// Текущие состояния
extern ProgramState progState;
extern MessageState msgState;
extern HeartBeatState hbState;
extern IgnitionState ignState;

// Прочие переменные
extern uint8_t inputHex;
extern uint8_t outputArray[4];
extern uint8_t outputLength;

extern uint8_t speedHEX[3];
extern uint8_t intakeAir[3];
extern uint8_t oilTemp[3];
extern uint8_t boost[3];
extern uint8_t colTemp[3];


#endif
