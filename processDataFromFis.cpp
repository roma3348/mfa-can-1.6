#include "globals.h"
#include "state_functions.h"
#include "can_functions.h"
#include <Arduino.h>

void processDataFromFis() {
  if (msgState == M_Init) {
    if (responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      
      messageInitFIS();
    } else if (responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else {
      Serial.println(F("[F]i"));
    }
  } else if (msgState == Sys_Info) {
    if (responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      
      sysTimingsFIS();
    } else if (responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else {
      Serial.println(F("[F]s"));
    }
  } else if (msgState == Status_Req) {
    if (responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      aprMessage(1);
      responseAwait = true;
      Serial.print(F("[F]ack a="));
      Serial.print(activeDdpChannel);
      Serial.print(F(" m="));
      if (ddpChannel < 0x10) Serial.print('0');
      Serial.print(ddpChannel, HEX);
      Serial.print(F(" u="));
      if (ddpChannelUpper < 0x10) Serial.print('0');
      Serial.println(ddpChannelUpper, HEX);
      chanelStatus = true;
      msgState = End_Wait;
    } else if (responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else {
      Serial.println(F("[F]sr"));
    }
  } else if (msgState == End_Wait) {
    if (responseAwait && millis() - lastSendMessage >= delayMessages) {
      endMessage();
      responseAwait = false;

      if (activeDdpChannel == 0) {
        if (ddpChannelUpper != 0xFF) {
          activeDdpChannel = 1;
          progState = Data_Send_Upper;
          Serial.println(chanelStatus ? F("[N]M>U") : F("[N]ML>U"));
        } else {
          activeDdpChannel = 0;
          progState = Data_Send;
          Serial.println(F("[N]M"));
        }
      } else {
        if (ddpChannel != 0xFF) {
          activeDdpChannel = 0;
          progState = Data_Send;
          Serial.println(chanelStatus ? F("[N]U>M") : F("[N]UL>M"));
        } else {
          activeDdpChannel = 1;
          progState = Data_Send_Upper;
          Serial.println(F("[N]U"));
        }
      }
      msgState = M_Init;
      dbgState();
    } else if (responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else {
      Serial.println(F("[F]end"));
    }
  }
}
