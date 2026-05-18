#include "globals.h"
#include "state_functions.h"
#include "can_functions.h"
#include <Arduino.h>

void processChanelCreateUpper() {
  if (msgState == M_Init) {
    if (!responseAwait && millis() - lastSendMessage >= delayMessages) {
      
      messageInit();
    } else if (!responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x2E8 && rxMsg.can_dlc >= 3 &&
               rxMsg.data[0] == 0x39 && rxMsg.data[1] == 0xD0 && rxMsg.data[2] == 0x99) {
      lastReceivedMessaage = millis();
      msgState = Sys_Info;
      responseAwait = false;
      
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      dbgTimeout(1);
      msgState = M_Init;
      responseAwait = false;
    }
  } else if (msgState == Sys_Info) {
    if (!responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      
      sysTimings();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.can_dlc >= 1 && rxMsg.data[0] == 0xA1) {
      lastReceivedMessaage = millis();
      msgState = Data_Info;
      responseAwait = false;
      
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      dbgTimeout(1);
      msgState = M_Init;
      responseAwait = false;
    }
  } else if (msgState == Data_Info) {
    if (!responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      
      dataFrame();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.can_dlc >= 1 && rxMsg.data[0] == 0xB1) {
      lastReceivedMessaage = millis();
      msgState = Status_Req;
      responseAwait = true;
      
    } else if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.can_dlc >= 2 && rxMsg.data[0] == 0x10) {
      lastReceivedMessaage = millis();
      msgState = Status_Req;
      responseAwait = true;
      dbgDdp(1, rxMsg);
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      dbgTimeout(1);
      msgState = M_Init;
      responseAwait = false;
    }
  } else if (msgState == Status_Req) {
    if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.can_dlc >= 2 && rxMsg.data[0] == 0x10) {
      if (rxMsg.data[1] == 0x23 && rxMsg.can_dlc >= 4) {
        lastReceivedMessaage = millis();
        uint8_t assigned = rxMsg.data[2];
        if (assigned == ddpChannel && ddpChannel != 0xFF) {
          Serial.println(F("[UC]sameID"));
        }
        ddpChannelUpper = assigned;
        aprMessage(1);
        responseAwait = true;
        chanelStatus = (rxMsg.data[3] == 0x01);
        upperChannelReady = true;
        dbgDdp(1, rxMsg);
        msgState = End_Wait;
      } else if (rxMsg.data[1] == 0x2B) {
        lastReceivedMessaage = millis();
        aprMessage(1);
        dbgErr(1, rxMsg);
        msgState = M_Init;
        responseAwait = false;
      } else if (rxMsg.data[1] == 0x25) {
        lastReceivedMessaage = millis();
        dbgDdp(1, rxMsg);
        msgState = M_Init;
        responseAwait = false;
      } else {
        dbgDdp(1, rxMsg);
      }
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      dbgTimeout(1);
      msgState = M_Init;
      responseAwait = false;
    }
  } else if (msgState == End_Wait) {
    if (responseAwait && millis() - lastSendMessage >= delayMessages) {
      if (!chanelStatus) {
        Serial.println(F("[UC]lock"));
      }
      progState = ID_Prior_Upper;
      msgState = M_Init;
      responseAwait = false;
      endMessage();
    } else if (responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else {
      Serial.println(F("[UC]A8"));
    }
  }
}
