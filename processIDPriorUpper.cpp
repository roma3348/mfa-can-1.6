#include "globals.h"
#include "state_functions.h"
#include "can_functions.h"
#include <Arduino.h>

void processIDPriorUpper() {
  // Safety net: if the cluster starts TP1.6 while we are here, accept it instead of looping on timing errors.
  if (isTachoTpRequest(rxMsg)) {
    startFisSessionFromTacho();
    return;
  }

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
      if (ddpChannelUpper == 0xFF) {
        Serial.println(F("[UP]noid"));
        progState = Request_Await;
        msgState = M_Init;
        responseAwait = false;
        return;
      }
      
      dataFrame();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.can_dlc >= 1 && rxMsg.data[0] == 0xB1) {
      lastReceivedMessaage = millis();
      msgState = Status_Req;
      responseAwait = true;
      
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      dbgTimeout(1);
      msgState = M_Init;
      responseAwait = false;
    }
  } else if (msgState == Status_Req) {
    if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.can_dlc >= 2 && rxMsg.data[0] == 0x10) {
      if ((rxMsg.data[1] == 0x23 || rxMsg.data[1] == 0x27) && rxMsg.can_dlc >= 4) {
        lastReceivedMessaage = millis();
        dbgDdp(1, rxMsg);
        if (rxMsg.data[3] == 0x01 && rxMsg.data[2] != ddpChannel && rxMsg.data[2] != 0xFF) {
          ddpChannelUpper = rxMsg.data[2];
          activeDdpChannel = 1;
          Serial.print(F("[U]id="));
          printHex2(ddpChannelUpper);
          Serial.println();
        }
        if (rxMsg.data[2] == ddpChannelUpper) {
          aprMessage(1);
          responseAwait = true;
          chanelStatus = (rxMsg.data[3] == 0x01);
          upperChannelReady = chanelStatus;
          msgState = End_Wait;
        } else {
          Serial.println(F("[UP]other"));
          aprMessage(1);
          responseAwait = true;
        }
      } else if (rxMsg.data[1] == 0x25) {
        lastReceivedMessaage = millis();
        dbgDdp(1, rxMsg);
        aprMessage(1);
        chanelStatus = true;
        upperChannelReady = true;
        responseAwait = true;
        msgState = End_Wait;
      } else if (rxMsg.data[1] == 0x2B) {
        lastReceivedMessaage = millis();
        dbgErr(1, rxMsg);
        aprMessage(1);
        // Do not get stuck on prioritize error; keep the channel and continue with normal data loop.
        chanelStatus = true;
        upperChannelReady = true;
        responseAwait = true;
        msgState = End_Wait;
      } else {
        dbgDdp(1, rxMsg);
      }
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      dbgTimeout(1);
      // If prioritize does not answer, do not deadlock. Data request from cluster will still be handled globally.
      chanelStatus = true;
      upperChannelReady = true;
      msgState = End_Wait;
      responseAwait = true;
    }
  } else if (msgState == End_Wait) {
    if (responseAwait && millis() - lastSendMessage >= delayMessages) {
      if (chanelStatus == true) {
        progState = Data_Send_Upper;
        msgState = M_Init;
        activeDdpChannel = 1;
        Serial.println(F("[UP]okU"));
      } else {
        progState = Request_Await;
        msgState = M_Init;
        Serial.println(F("[UP]lock"));
      }
      responseAwait = false;
      endMessage();
    } else if (responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else {
      Serial.println(F("[UP]A8"));
    }
  }
}
