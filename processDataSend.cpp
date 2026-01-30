#include "globals.h"
#include "state_functions.h"
#include "can_functions.h"
#include <Arduino.h>

void processDataSend() {
  if (msgState == M_Init) 
  {
    if (!responseAwait && millis() - lastSendMessage >= delayMessages) 
      {
        messageInit();
      } 
    else if (!responseAwait && millis() - lastSendMessage < delayMessages) 
      {
        return;
      } 
    else if (responseAwait && rxMsg.can_id == 0x2E8 && rxMsg.data[0] == 0x39 && rxMsg.data[1] == 0xD0) 
      {
        lastReceivedMessaage = millis();
        msgState = Sys_Info;
        responseAwait = false;
      } 
    else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) 
      {
        msgState = M_Init;
        responseAwait = false;
      } 
    else if (rxMsg.can_id == 0x699 && rxMsg.data[0] == 0x10) 
      {
        if (rxMsg.data[1] == 0x27) {
          lastReceivedMessaage = millis();
          aprMessage(1);
          responseAwait = true;
          if (rxMsg.data[3] == 0x01) {
            chanelStatus = true;
          } else if (rxMsg.data[3] == 0x00) {
            chanelStatus = false;
          }
          msgState = End_Wait;
        } else if (rxMsg.data[1] == 0x25) {
          lastReceivedMessaage = millis();
          msgState = M_Init;
          responseAwait = false;
          Serial.println(F("Канал не приоритизирован----- ОШИБКА"));
        } else {
          Serial.println(F("Канал не приоритизирован ----- Другая ошибка"));
        }
      }
  }
  else if (msgState == Sys_Info) {
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
    } 
    else if (rxMsg.can_id == 0x699 && rxMsg.data[0] == 0x10) 
      {
        if (rxMsg.data[1] == 0x27) {
          lastReceivedMessaage = millis();
          aprMessage(1);
          responseAwait = true;
          if (rxMsg.data[3] == 0x01) {
            chanelStatus = true;
          } else if (rxMsg.data[3] == 0x00) {
            chanelStatus = false;
          }
          msgState = End_Wait;
        } else if (rxMsg.data[1] == 0x25) {
          lastReceivedMessaage = millis();
          msgState = M_Init;
          responseAwait = false;
          Serial.println(F("Канал не приоритизирован----- ОШИБКА"));
        } else {
          Serial.println(F("Канал не приоритизирован ----- Другая ошибка"));
        }
      }
  } else if (msgState == Data_Info) {
    if (!responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      dataFrame();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } else if (responseAwait && rxMsg.can_id == 0x699 &&
               rxMsg.data[0] == 0xB4) {
      lastReceivedMessaage = millis();
      msgState = Data_Info2;
      responseAwait = false;
    } else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = M_Init;
      responseAwait = false;;
    }
  } else if (msgState == Data_Info2) {
    if (!responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      dataFrame();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    } 
    else if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.data[0] == 0xB8)
    {
      lastReceivedMessaage = millis();
      msgState = Data_Info3;
      responseAwait = false;
    }
    else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = M_Init;
      responseAwait = false;;
    }
  }
    else if (msgState == Data_Info3) {
    if (!responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
      dataFrame();
    } else if (!responseAwait && millis() - lastReceivedMessaage < delayMessages) {
      return;
    }
    else if (responseAwait && rxMsg.can_id == 0x699 && rxMsg.data[0] == 0xBA)
    {
      lastReceivedMessaage = millis();
      msgState = Status_Req;
      responseAwait = true;
    }
    else if (rxMsg.can_id == 0x699 && rxMsg.data[0] == 0x10) 
      {
        if (rxMsg.data[1] == 0x27) {
          lastReceivedMessaage = millis();
          aprMessage(1);
          responseAwait = true;
          if (rxMsg.data[3] == 0x01) {
            chanelStatus = true;
          } else if (rxMsg.data[3] == 0x00) {
            chanelStatus = false;
          }
          msgState = End_Wait;
        } else if (rxMsg.data[1] == 0x25) {
          lastReceivedMessaage = millis();
          msgState = M_Init;
          responseAwait = false;
          Serial.println(F("Канал не приоритизирован----- ОШИБКА"));
        }
        else if (rxMsg.data[1] == 0x2B) {
          lastReceivedMessaage = millis();
          msgState = M_Init;
          responseAwait = false;
          Serial.println(F("Канал не приоритизирован----- ОШИБКА"));
        }
         else {
          Serial.println(F("Канал не приоритизирован ----- Другая ошибка"));
        }
      }
    else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) 
    {
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
        } else if (rxMsg.data[3] == 0x00) {
          chanelStatus = false;
        }
        msgState = End_Wait;
      } else if (rxMsg.data[1] == 0x2B) {
        lastReceivedMessaage = millis();
        aprMessage(1);
        msgState = M_Init;
        responseAwait = false;
        Serial.println(F("Ошибка канала"));
      } else {
        Serial.println(F("Канал не приоритизирован ----- Другая ошибка"));
      }
    }
    else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = End_Wait;
      responseAwait = true;
    }
  } else if (msgState == End_Wait) {
    if (responseAwait && millis() - lastSendMessage >= delayMessages) {
      if (chanelStatus == true) {
        progState = Data_Send;
        msgState = M_Init;
      } else if (chanelStatus == false) {
        progState = Request_Await;
        msgState = M_Init;
      }
      responseAwait = false;
      endMessage();
    } else if (responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else {
      Serial.println(F("Ошибка отправки A8"));
    }
  }
}       

