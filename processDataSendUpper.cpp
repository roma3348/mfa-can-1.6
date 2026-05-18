#include "globals.h"
#include "state_functions.h"
#include "can_functions.h"
#include <Arduino.h>

void processDataSendUpper() {
  if (msgState == M_Init) {
    if (!responseAwait) {
      unsigned long now = millis();
      if (now < displayCooldownUntil) return;
      if (lastUpperUpdate != 0 && now - lastUpperUpdate < upperMinInterval) {
        if (ddpChannel != 0xFF && (lastMainUpdate == 0 || now - lastMainUpdate >= mainMinInterval)) {
          activeDdpChannel = 0;
          progState = Data_Send;
          msgState = M_Init;
          Serial.println(F("[TH]U>M"));
        }
        return;
      }
    }
    if (!responseAwait && millis() - lastSendMessage >= delayMessages) {
      messageInit();
    } else if (!responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x2E8 && rxMsg.data[0] == 0x39 && rxMsg.data[1] == 0xD0) {
      lastReceivedMessaage = millis();
      msgState = Sys_Info;
      responseAwait = false;
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = M_Init;
      responseAwait = false;
    }
  } else if (msgState == Sys_Info) {
    if (!responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      sysTimings();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.data[0] == 0xA1) {
      lastReceivedMessaage = millis();
      msgState = Data_Info;
      responseAwait = false;
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = M_Init;
      responseAwait = false;
    }
  } else if (msgState == Data_Info) {
    if (!responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      dataFrame();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.data[0] == 0xB5) {
      lastReceivedMessaage = millis();
      msgState = Status_Req;
      responseAwait = true;
    } else if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.data[0] == 0x10) {
      lastReceivedMessaage = millis();
      msgState = Status_Req;
      responseAwait = true;
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = M_Init;
      responseAwait = false;
    }
  } else if (msgState == Status_Req) {
    if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.data[0] == 0x10) {
      if (rxMsg.data[1] == 0x27) {
        lastReceivedMessaage = millis();
        aprMessage(1);
        responseAwait = true;
        if (rxMsg.data[3] == 0x01) {
          chanelStatus = true;
          upperChannelReady = true;
        } else if (rxMsg.data[3] == 0x00) {
          chanelStatus = false;
          upperChannelReady = false;
        }
        msgState = End_Wait;
      } else if (rxMsg.data[1] == 0x2B) {
        lastReceivedMessaage = millis();
        aprMessage(1);
        msgState = M_Init;
        responseAwait = false;
        upperChannelReady = false;
        Serial.println(F("[U]err"));
      }
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = End_Wait;
      responseAwait = true;
    }
  } else if (msgState == End_Wait) {
    if (responseAwait && millis() - lastSendMessage >= delayMessages) {
      lastUpperUpdate = millis();
      endMessage();
      responseAwait = false;
      activeDdpChannel = 0;
      progState = Data_Send;
      msgState = M_Init;
    } else if (responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else {
      Serial.println(F("[U]A8"));
    }
  }
}
