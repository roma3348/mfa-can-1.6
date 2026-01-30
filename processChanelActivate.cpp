#include "globals.h"
#include "state_functions.h"
#include "can_functions.h"
#include <Arduino.h>

void processChanelActivate() {
  if (msgState == M_Init) {
    if (!responseAwait && millis() - lastSendMessage >= delayMessages) {
      messageInit();
    } else if (!responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x2E8 &&
               rxMsg.data[0] == 0x39 && rxMsg.data[1] == 0xD0) {
      lastReceivedMessaage = millis();
      msgState = Sys_Info;
      responseAwait = false;
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = M_Init;
      responseAwait = false;
    } else {
      Serial.println(F("Ошибка инициализации сообщения"));
    }
  } else if (msgState == Sys_Info) {
    if (!responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      sysTimings();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x699 &&
               rxMsg.data[0] == 0xA1) {
      lastReceivedMessaage = millis();
      msgState = Data_Info;
      responseAwait = false;
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = M_Init;
      responseAwait = false;
    } else {
      Serial.println(F("Ошибка таймингов в Chanel_Create"));
    }
  } else if (msgState == Data_Info) {
    if (!responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      dataFrame();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x699 &&
               rxMsg.data[0] == 0xB1) {
      lastReceivedMessaage = millis();
      msgState = Status_Req;
      responseAwait = true;
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = M_Init;
      responseAwait = false;
    }
  } else if (msgState == Status_Req) {
    if (responseAwait && rxMsg.can_id == 0x699 &&
        rxMsg.data[0] == 0x10 && rxMsg.data[1] == 0x35) {
      lastReceivedMessaage = millis();
      ddpChannel = rxMsg.data[2];
      aprMessage(1);
      responseAwait = true;
      chanelStatus = true;
      msgState = End_Wait;
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = M_Init;
      Serial.println(F("Ошибка активации канала"));
    }
  } else if (msgState == End_Wait) {
    if (responseAwait && millis() - lastSendMessage >= delayMessages) {
      progState = Request_Await;
      msgState = M_Init;
      responseAwait = false;
      endMessage();
    } else if (responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else {
      Serial.println(F("Ошибка отправки A8"));
    }
  }
}
