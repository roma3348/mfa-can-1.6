#include "globals.h"
#include "can_functions.h"
#include "state_functions.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // Ожидание подключения Serial

  mcp2515.reset();
  mcp2515.setBitrate(CAN_100KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  Serial.println(F("Инициализация завершена"));
}

void loop() {
  if (Serial.available() > 0) {
    String receivedData = Serial.readStringUntil('\n');
    receivedData.trim();
    processAir(receivedData);
  }
  if (mcp2515.readMessage(&rxMsg) == MCP2515::ERROR_OK) {
    logging(rxMsg.can_id, rxMsg.can_dlc, rxMsg.data);
    // Check changes of HeartBeat state
    if (rxMsg.can_id == 0x436) {
      checkHBState(rxMsg.can_id, rxMsg.can_dlc, rxMsg.data);
    }
    // Check if we need to send HeartBeat
    if (heartbeatAwait && millis() - lastHeartBeat >= delayHeartBeat) {
      heartBeat();
      heartbeatAwait = false;  // Сбрасываем флаг
    }
    // Check car state
    if (rxMsg.can_id == 0x271) {
      checkIgnitionState(rxMsg.can_id, rxMsg.can_dlc, rxMsg.data);
    }

    if (ignState == i_Operate && hbState == h_Operate && hbregistered == true) {
      //Receive battery charge information
      if (rxMsg.can_id == 0x571){
        processHex(rxMsg.data[0], outputArray, outputLength);
        }
      if (rxMsg.can_id == 0x5C1){
        mfl(rxMsg.data[0]);
        }
      if (rxMsg.can_id == 0x351){
        if (rxMsg.data[0] != 0x01){
          processSpeed(rxMsg.data[1],rxMsg.data[2]);
        }
        else{
          speedHEX[0] = 0x20;
          speedHEX[1] = 0x20;
          speedHEX[2] = 0x20;

        }
      }
      if (rxMsg.can_id == 0x2E8 && rxMsg.data[0] == 0x39 && rxMsg.data[1] == 0xC0 && rxMsg.data[2] == 0x99) {
        progState = FisToCan;
        msgState = M_Init;
        responseAwait = true;
        lastReceivedMessaage = millis();
      }
      if (rxMsg.can_id == 0x699 && rxMsg.data[0] == 0xA0) {
        progState = FisToCan;
        msgState = Sys_Info;
        responseAwait = true;
        lastReceivedMessaage = millis();
      }
      if (rxMsg.can_id == 0x699 && rxMsg.data[0] == 0x10 && rxMsg.data[1] == 0x23 && rxMsg.data[3] == 0x01) {
        progState = FisToCan;
        msgState = Status_Req;
        ddpChannel = rxMsg.data[2];
        responseAwait = true;
        lastReceivedMessaage = millis();
      }
      if (rxMsg.can_id == 0x699 && rxMsg.data[0] == 0x10 && rxMsg.data[1] == 0x27 && rxMsg.data[3] == 0x01) {
        progState = FisToCan;
        msgState = Status_Req;
        ddpChannel = rxMsg.data[2];
        responseAwait = true;
        lastReceivedMessaage = millis();
      }
      if (rxMsg.can_id == 0x699 && rxMsg.data[0] == 0x10 && rxMsg.data[1] == 0x2B ) {
        progState = FisToCan;
        msgState = Status_Req;
        responseAwait = true;
        lastReceivedMessaage = millis();
      }
      

      // Выбор дальнейшей обработки в зависимости от основного состояния
      switch (progState) {
          case DDP_Delete:      processDDPDelete();      break;
          case FisToCan:        processDataFromFis();    break;
          case Chanel_Create:   processChanelCreate();   break;
          case Chanel_Activate: processChanelActivate(); break;
          case ID_Prior:        processIDPrior();        break;
          case Data_Send:       processDataSend();       break;
          case Request_Await:                            break;
          default: break;
      }
    }
    if (ignState == i_Off) {
      Serial.println(F("standby"));
      hbState = h_Standby;
    }
  }
}
