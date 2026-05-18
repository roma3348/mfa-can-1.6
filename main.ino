#include "globals.h"
#include "can_functions.h"
#include "state_functions.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // Ожидание подключения Serial

  mcp2515.reset();
  mcp2515.setBitrate(CAN_100KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  Serial.println(F("INIT_DONE"));
  dbgState();
}

void loop() {
  if (Serial.available() > 0) {
    String receivedData = Serial.readStringUntil('\n');
    receivedData.trim();
    if (receivedData.length() > 0) {
      processAir(receivedData);
    }
  }

  if (mcp2515.readMessage(&rxMsg) == MCP2515::ERROR_OK) {
    logging(rxMsg.can_id, rxMsg.can_dlc, rxMsg.data);

    // Keep legacy Heartbeat trigger to avoid changing the working ring behavior.
    if (rxMsg.can_id == 0x40C && rxMsg.can_dlc >= 1) {
      checkHBState(rxMsg.can_id, rxMsg.can_dlc, rxMsg.data);
    }

    // Check if we need to send HeartBeat
    if (heartbeatAwait && millis() - lastHeartBeat >= delayHeartBeat) {
      heartBeat();
      heartbeatAwait = false;
    }

    // Check car state
    if (rxMsg.can_id == 0x271 && rxMsg.can_dlc >= 1) {
      checkIgnitionState(rxMsg.can_id, rxMsg.can_dlc, rxMsg.data);
    }

    if (ignState == i_Operate && hbState == h_Operate && hbregistered == true) {
      // Receive battery charge information
      if (rxMsg.can_id == 0x571 && rxMsg.can_dlc >= 1) {
        processHex(rxMsg.data[0], outputArray, outputLength);
      }
      if (rxMsg.can_id == 0x5C1 && rxMsg.can_dlc >= 1) {
        mfl(rxMsg.data[0]);
      }
      if (rxMsg.can_id == 0x351 && rxMsg.can_dlc >= 3) {
        if (rxMsg.data[0] != 0x01) {
          processSpeed(rxMsg.data[1], rxMsg.data[2]);
        } else {
          speedHEX[0] = 0x20;
          speedHEX[1] = 0x20;
          speedHEX[2] = 0x20;
        }
      }

      // The cluster may initiate TP1.6 not only from Request_Await. In the last log it did this during ID_Prior_Upper.
      bool canHandleFisRequest =
        (progState == Request_Await ||
         progState == FisToCan ||
         progState == ID_Prior_Upper ||
         progState == Data_Send ||
         progState == Data_Send_Upper);

      if (canHandleFisRequest && isTachoTpRequest(rxMsg)) {
        startFisSessionFromTacho();
      }
      else if (canHandleFisRequest && isTachoTimingRequest(rxMsg)) {
        Serial.println(F("[FIS]A0"));
        progState = FisToCan;
        msgState = Sys_Info;
        responseAwait = true;
        lastReceivedMessaage = millis();
        dbgState();
      }
      else if (canHandleFisRequest && isDdpStatusOrError(rxMsg)) {
        if (rxMsg.data[1] == 0x2B) {
          dbgErr(1, rxMsg);
          displayCooldownUntil = millis() + errorCooldownInterval;
        } else {
          dbgDdp(1, rxMsg);
        }

        if (rxMsg.data[1] == 0x23 || rxMsg.data[1] == 0x27) {
          uint8_t requestedChannel = (rxMsg.can_dlc >= 3) ? rxMsg.data[2] : 0xFF;
          uint8_t requestedStatus = (rxMsg.can_dlc >= 4) ? rxMsg.data[3] : 0xFF;

          bool upperCtx = (progState == Chanel_Create_Upper ||
                           progState == ID_Prior_Upper ||
                           progState == Data_Send_Upper ||
                           activeDdpChannel == 1);

          if (requestedStatus == 0x01 && upperCtx &&
              requestedChannel != 0xFF && requestedChannel != ddpChannel) {
            ddpChannelUpper = requestedChannel;
            upperChannelReady = true;
            activeDdpChannel = 1;
            Serial.print(F("[U]id="));
            printHex2(ddpChannelUpper);
            Serial.println();
          } else if (requestedChannel == ddpChannelUpper && requestedChannel != 0xFF &&
                     ddpChannelUpper != ddpChannel) {
            activeDdpChannel = 1;
          } else if (requestedChannel == ddpChannel && requestedChannel != 0xFF) {
            activeDdpChannel = 0;
          } else {
            Serial.print(F("[W]ch="));
            if (requestedChannel < 0x10) Serial.print('0');
            Serial.println(requestedChannel, HEX);
            activeDdpChannel = 0;
          }

          if (requestedStatus == 0x01) {
            progState = FisToCan;
            msgState = Status_Req;
            responseAwait = true;
            lastReceivedMessaage = millis();
            dbgState();
          } else {
            Serial.println(F("[D]lock"));
            displayCooldownUntil = millis() + errorCooldownInterval;
            if (requestedChannel == ddpChannelUpper && ddpChannelUpper != 0xFF) {
              upperChannelReady = false;
              activeDdpChannel = 1;
            } else if (requestedChannel == ddpChannel && ddpChannel != 0xFF) {
              activeDdpChannel = 0;
            }
            progState = FisToCan;
            msgState = Status_Req;
            responseAwait = true;
            lastReceivedMessaage = millis();
            dbgState();
          }
        }
        else if (rxMsg.data[1] == 0x2B) {
          progState = FisToCan;
          msgState = Status_Req;
          responseAwait = true;
          lastReceivedMessaage = millis();
          dbgState();
        }
      }

      dbgState();

      // Выбор дальнейшей обработки в зависимости от основного состояния
      switch (progState) {
        case DDP_Delete:             processDDPDelete();           break;
        case FisToCan:               processDataFromFis();         break;
        case Chanel_Create:          processChanelCreate();        break;
        case Chanel_Create_Upper:    processChanelCreateUpper();   break;
        case ID_Prior_Upper:         processIDPriorUpper();        break;
        case Data_Send:              processDataSend();            break;
        case Data_Send_Upper:        processDataSendUpper();       break;
        case Request_Await:                                       break;
        default: break;
      }

      dbgState();
    }

    if (ignState == i_Off) {
      
      hbState = h_Standby;
    }
  }
}
