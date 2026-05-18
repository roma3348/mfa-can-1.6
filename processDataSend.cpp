#include "globals.h"
#include "state_functions.h"
#include "can_functions.h"
#include <Arduino.h>

void processDataSend() {
  if (msgState == M_Init) 
  {
    if (!responseAwait) {
      unsigned long now = millis();
      if (now < displayCooldownUntil) return;
      if (lastMainUpdate != 0 && now - lastMainUpdate < mainMinInterval) {
        if (ddpChannelUpper != 0xFF && (lastUpperUpdate == 0 || now - lastUpperUpdate >= upperMinInterval)) {
          activeDdpChannel = 1;
          progState = Data_Send_Upper;
          msgState = M_Init;
          Serial.println(F("[TH]M>U"));
        }
        return;
      }
    }
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
          Serial.println(F("[CH]p"));
        } else {
          Serial.println(F("[CH]e"));
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
          Serial.println(F("[CH]p"));
        } else {
          Serial.println(F("[CH]e"));
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
          Serial.println(F("[CH]p"));
        }
        else if (rxMsg.data[1] == 0x2B) {
          lastReceivedMessaage = millis();
          msgState = M_Init;
          responseAwait = false;
          Serial.println(F("[CH]p"));
        }
         else {
          Serial.println(F("[CH]e"));
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
        Serial.println(F("[CH]e"));
      } else {
        Serial.println(F("[CH]e"));
      }
    }
    else if (responseAwait && millis() - lastSendMessage >= responseMessagesMax) {
      msgState = End_Wait;
      responseAwait = true;
    }
  } else if (msgState == End_Wait) {
    if (responseAwait && millis() - lastSendMessage >= delayMessages) {
      lastMainUpdate = millis();
      if (ddpChannelUpper != 0xFF) {
        activeDdpChannel = 1;
        progState = Data_Send_Upper;
        Serial.println(chanelStatus ? F("[M]>U") : F("[M]L>U"));
      } else if (chanelStatus == true) {
        activeDdpChannel = 0;
        progState = Data_Send;
      } else {
        progState = Request_Await;
      }
      msgState = M_Init;
      responseAwait = false;
      endMessage();
    } else if (responseAwait && millis() - lastSendMessage < delayMessages) {
      return;
    } else {
      Serial.println(F("[A8]err"));
    }
  }
}       

