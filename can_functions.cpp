#include "can_functions.h"
#include "globals.h"
#include <string.h>


void logging(uint32_t id, uint8_t dlc, const uint8_t *data) {
  if (isFilteredID(id)) {
    unsigned long t = millis();
    unsigned long hours = t / 3600000;
    unsigned long minutes = (t % 3600000) / 60000;
    unsigned long seconds = (t % 60000) / 1000;
    unsigned long msec = t % 1000;

    // Форматируем время в ЧЧ:ММ:СС.ммм
    if (hours < 10) Serial.print("0");
    Serial.print(hours);
    Serial.print(":");
    if (minutes < 10) Serial.print("0");
    Serial.print(minutes);
    Serial.print(":");
    if (seconds < 10) Serial.print("0");
    Serial.print(seconds);
    Serial.print(".");
    if (msec < 100) {
      if (msec < 10) Serial.print("00");
      else Serial.print("0");
    }
    Serial.print(msec);
    Serial.print(" RECEIVED -> ");

    // Выводим остальной текст
    Serial.print(F("[CAN] ID: 0x"));
    Serial.print(id, HEX);
    Serial.print(F(" DLC: "));
    Serial.print(dlc);
    Serial.print(F(" | DATA: "));
    for (int i = 0; i < dlc; i++) {
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void sendCanMessage(uint32_t id, uint8_t dlc, const uint8_t *data) {
  unsigned long t = millis();
  txMsg.can_id = id;
  txMsg.can_dlc = dlc;
  memcpy(txMsg.data, data, dlc);
  if (mcp2515.sendMessage(&txMsg) == MCP2515::ERROR_OK) {
    unsigned long hours = t / 3600000;
    unsigned long minutes = (t % 3600000) / 60000;
    unsigned long seconds = (t % 60000) / 1000;
    unsigned long msec = t % 1000;

    // Форматируем время в ЧЧ:ММ:СС.ммм
    if (hours < 10) Serial.print("0");
    Serial.print(hours);
    Serial.print(":");
    if (minutes < 10) Serial.print("0");
    Serial.print(minutes);
    Serial.print(":");
    if (seconds < 10) Serial.print("0");
    Serial.print(seconds);
    Serial.print(".");
    if (msec < 100) {
      if (msec < 10) Serial.print("00");
      else Serial.print("0");
    }
    Serial.print(msec);
    Serial.print(" SEND ->.    ");

    // Выводим остальной текст
    Serial.print(F("[CAN] ID: 0x"));
    Serial.print(id, HEX);
    Serial.print(F(" DLC: "));
    Serial.print(dlc);
    Serial.print(F(" | DATA: "));
    for (int i = 0; i < dlc; i++) {
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println(F("Ошибка отправки"));
  }
}


void messageInit() {
  const uint8_t initData[3] = { 0x08, 0xC0, 0xB9 };
  sendCanMessage(0x4D9, 3, initData);
  responseAwait = true;
  lastSendMessage = millis();
}

void messageInitFIS() {
  const uint8_t initData[3] = { 0x08, 0xD0, 0xB9 };
  sendCanMessage(0x4D9, 3, initData);
  responseAwait = false;
  lastSendMessage = millis();
}

void sysTimings() {
  const uint8_t timingData[6] = { 0xA0, 0x04, 0x82, 0x84, 0x46, 0xC5 };
  sendCanMessage(0x6B9, 6, timingData);
  responseAwait = true;
  lastSendMessage = millis();
}

void sysTimingsFIS() {
  const uint8_t timingData[6] = { 0xA1, 0x04, 0x82, 0x84, 0x46, 0xC5 };
  sendCanMessage(0x6B9, 6, timingData);
  responseAwait = false;
  lastSendMessage = millis();
}

void aprMessage(uint8_t messageCount) {
  uint8_t aprData[1] = { 0xB0 + messageCount };
  sendCanMessage(0x6B9, 1, aprData);
  lastSendMessage = millis();
}

void endMessage() {
  const uint8_t data[1] = { 0xA8 };
  sendCanMessage(0x6B9, 1, data);
  lastSendMessage = millis();
}

void dataFrame() {
  switch (progState) {
    case Chanel_Create: {
      const uint8_t dData[8] = { 0x20, 0x02, 0x70, 0x39, 0x10, 0x53, 0x50, 0x4F };
      const uint8_t dData2[3] = { 0x11, 0x52, 0x54 };
      sendCanMessage(0x6B9, 8, dData);
      sendCanMessage(0x6B9, 3, dData2);
      responseAwait = true;
      lastSendMessage = millis();
      break;
    }
    case Chanel_Activate: {
      const uint8_t cData[6] = { 0x10, 0x15, 0x39, 0x20, 0x01, 0x01 };
      sendCanMessage(0x6B9, 6, cData);
      responseAwait = true;
      lastSendMessage = millis();
      break;
    }
    case ID_Prior: {
      const uint8_t idData[3] = { 0x10, 0x0C, ddpChannel };
      sendCanMessage(0x6B9, 3, idData);
      responseAwait = true;
      lastSendMessage = millis();
      break;
    }
    case DDP_Delete: {
      const uint8_t idData[3] = { 0x10, 0x00, 0x39 };
      sendCanMessage(0x6B9, 3, idData);
      responseAwait = true;
      lastSendMessage = millis();
      break;
    }
    case Data_Send: {
      if (msgState == Data_Info) {
        Serial.println(ddpChannel,HEX);
        const uint8_t sendData1[8] = { 0x20, 0x09, ddpChannel, 0x57, 0x0E, 0x36, 0x00, 0x00 };
        const uint8_t sendData2[8] = { 0x21, 0x04, 0x00, 0x41, 0x49, 0x52, 0x20, 0x3A };
        const uint8_t sendData3[8] = { 0x22, 0x20, intakeAir[0], intakeAir[1], intakeAir[2], 0x57, 0x0E, 0x36};
        const uint8_t sendData4[8] = { 0x03, 0x00, 0x00, 0x10, 0x00, 0x4F, 0x49, 0x4C};
        sendCanMessage(0x6B9, 8, sendData1);
        sendCanMessage(0x6B9, 8, sendData2);
        sendCanMessage(0x6B9, 8, sendData3);
        sendCanMessage(0x6B9, 8, sendData4);
        responseAwait = true;
        lastSendMessage = millis();
      }
      if (msgState == Data_Info2) {
        const uint8_t send2Data1[8] = { 0x24, 0x20, 0x3A, 0x20, oilTemp[0], oilTemp[1], oilTemp[2], 0x57};
        const uint8_t send2Data2[8] = { 0x25, 0x0F, 0x36, 0x00, 0x00, 0x1A, 0x00, 0x42};
        const uint8_t send2Data3[8] = { 0x26, 0x4F, 0x4F, 0x53, 0x54, 0x3A, 0x20, boost[0]};
        const uint8_t send2Data4[8] = { 0x07, boost[1], boost[2], 0x57, 0x0F, 0x36, 0x00, 0x00};
        sendCanMessage(0x6B9, 8, send2Data1);
        sendCanMessage(0x6B9, 8, send2Data2);
        sendCanMessage(0x6B9, 8, send2Data3);
        sendCanMessage(0x6B9, 8, send2Data4);
        responseAwait = true;
        lastSendMessage = millis();
      }
      if (msgState == Data_Info3) {
        const uint8_t send3Data1[8] = { 0x28, 0x24, 0x00, 0x54, 0x45, 0x4D, 0x50, 0x20};
        const uint8_t send3Data2[7] = { 0x19, 0x3A, 0x20, colTemp[0], colTemp[1], colTemp[2], 0x08};
        sendCanMessage(0x6B9, 8, send3Data1);
        sendCanMessage(0x6B9, 7, send3Data2);
        responseAwait = true;
        lastSendMessage = millis();
      }
      break;
    }
    default:
      break;
  }
}



// Функция проверки, соответствует ли ID фильтру
bool isFilteredID(unsigned int id) {
  for (int i = 0; i < FILTERED_COUNT; i++) {
    if (id == pgm_read_word(&FILTERED_IDS[i])) return true;
  }
  return false;
}

void processHex(uint8_t hexValue, uint8_t* outputArray, uint8_t &outputLength) {
  int decimalValue = hexValue;
  float modifiedValue = ((decimalValue / 2.0) + 50) / 10.0;
  char buffer[5];
  dtostrf(modifiedValue, 0, 1, buffer);
  outputLength = 0;
  for (int i = 0; buffer[i] != '\0'; i++) {
    if (buffer[i] != ' ')
      outputArray[outputLength++] = (uint8_t)buffer[i];
  }
}

void processSpeed(uint8_t hexUW, uint8_t hexOW) {
  // Извлекаем байты UW и OW из сообщения
  uint8_t UW = hexUW;
  uint8_t OW = hexOW;

  // Формируем 16-битное число из байтов (OW << 8) | UW
  uint16_t value = ((uint16_t)OW << 8) | UW;

  // Вычисляем скорость по заданной формуле
  uint16_t speed = (value - 1) / 190;

  // Форматируем скорость в строку из 3-х символов с выравниванием по правому краю
  // Если значение занимает менее 3-х символов, слева будут пробелы (0x20)
  char speedStr[4]; // 3 символа + завершающий нуль
  snprintf(speedStr, sizeof(speedStr), "%3u", speed);

  // Записываем полученные символы в массив speedHex
  speedHEX[0] = speedStr[0];
  speedHEX[1] = speedStr[1];
  speedHEX[2] = speedStr[2];
}

void processAir(String input){
  input.trim();  // Убираем лишние пробелы

  // Пример входной строки:
  // "Air: 0x20, 0x20, 0x20; Oil: 0x20, 0x20, 0x20; Boost: 0x20, 0x20, 0x20; Temp: 0x20, 0x20, 0x20;"
  int start = 0;

  // Проходим по всей строке, разделяя секции по символу ';'
  while (start < input.length()) {
    int semicolonIndex = input.indexOf(';', start);
    if (semicolonIndex == -1) break; // если ';' не найден, выходим из цикла
    String section = input.substring(start, semicolonIndex);
    section.trim();

    if (section.length() > 0) {
      // Находим разделитель ':' между меткой датчика и данными
      int colonIndex = section.indexOf(':');
      if (colonIndex != -1) {
        String label = section.substring(0, colonIndex);
        String valuesStr = section.substring(colonIndex + 1);
        label.trim();
        valuesStr.trim();

        // Разбиваем valuesStr на три токена, разделённые запятыми
        int firstComma = valuesStr.indexOf(',');
        int secondComma = valuesStr.indexOf(',', firstComma + 1);
        if (firstComma != -1 && secondComma != -1) {
          String token1 = valuesStr.substring(0, firstComma);
          String token2 = valuesStr.substring(firstComma + 1, secondComma);
          String token3 = valuesStr.substring(secondComma + 1);
          token1.trim();
          token2.trim();
          token3.trim();

          // Удаляем префикс "0x", если он присутствует
          if (token1.startsWith("0x") || token1.startsWith("0X"))
            token1 = token1.substring(2);
          if (token2.startsWith("0x") || token2.startsWith("0X"))
            token2 = token2.substring(2);
          if (token3.startsWith("0x") || token3.startsWith("0X"))
            token3 = token3.substring(2);

          // Преобразуем токены из HEX-строки в числа
          byte value1 = (byte) strtol(token1.c_str(), NULL, 16);
          byte value2 = (byte) strtol(token2.c_str(), NULL, 16);
          byte value3 = (byte) strtol(token3.c_str(), NULL, 16);

          // В зависимости от метки записываем значения в соответствующий глобальный массив
          if (label.equalsIgnoreCase("Air")) {
            intakeAir[0] = value1;
            intakeAir[1] = value2;
            intakeAir[2] = value3;
          }
          else if (label.equalsIgnoreCase("Oil")) {
            oilTemp[0] = value1;
            oilTemp[1] = value2;
            oilTemp[2] = value3;
          }
          else if (label.equalsIgnoreCase("Boost")) {
            boost[0] = value1;
            boost[1] = value2;
            boost[2] = value3;
          }
          else if (label.equalsIgnoreCase("Temp")) {
            colTemp[0] = value1;
            colTemp[1] = value2;
            colTemp[2] = value3;
          }
          else {
            Serial.print("Неизвестная метка: ");
            Serial.println(label);
          }
        }
        else {
          Serial.println("Ошибка разбора: недостаточно данных в секции.");
        }
      }
      else {
        Serial.println("Ошибка разбора: отсутствует ':' в секции.");
      }
    }

    start = semicolonIndex + 1;  // переходим к следующей секции
  }
}

void mfl(uint8_t mflValue) {
  if(mflValue == 0x0){
      const uint8_t idData[2] = { 0x0, 0x00};
      sendCanMessage(0x5c3, 2, idData);// Defaul, none button pressed
  }
  if(mflValue == 0x6){
      const uint8_t idData[2] = { 0x0, 0x06};
      sendCanMessage(0x5c3, 2, idData);// Defaul, none button pressed
  }
  if(mflValue == 0x7){
      const uint8_t idData[2] = { 0x0, 0x07};
      sendCanMessage(0x5c3, 2, idData);// Defaul, none button pressed
  }
  if(mflValue == 0x22){
      const uint8_t idData[2] = { 0x0, 0x02};
      sendCanMessage(0x5c3, 2, idData);// Defaul, none button pressed
  }
  if(mflValue == 0x23){
      const uint8_t idData[2] = { 0x0, 0x03};
      sendCanMessage(0x5c3, 2, idData);// Defaul, none button pressed
  }
  if(mflValue == 0x2B){
      Serial.println(F("Перезагрузка"));
      ProgramState progState = DDP_Delete;
      MessageState msgState = M_Init;
      HeartBeatState hbState = h_Registry;
      wdt_enable(WDTO_15MS);   // срабатывание примерно через 15 мс
      while (1) { }
  }
}

void heartBeat() {
  if (hbState == h_Operate) {
    const uint8_t heartBeatData[6] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
    sendCanMessage(0x439, 6, heartBeatData);
    Serial.println(F("Heartbeat отправлен"));
    hbregistered = true;
  } else if (hbState == h_Registry) {
    const uint8_t registerData[6] = { 0x19, 0x02, 0x00, 0x01, 0x00, 0x00 };
    delay(10);
    sendCanMessage(0x439, 6, registerData);
    Serial.println(F("Регистрация нового участника CAN отправлена"));
    if (hbregistered == false) {
      hbregistered = false;
    }
  } else if (hbState == h_Standby) {
    const uint8_t standByData[6] = { 0x00, 0x11, 0x00, 0x00, 0x00, 0x00 };
    sendCanMessage(0x439, 6, standByData);
    Serial.println(F("Can в режиме StandBy"));
    hbregistered = true;
  }
}

void checkHBState(uint32_t id, uint8_t dlc, const uint8_t *data) {
  if ((data[0] == 0x19 || (data[0] == 0x16 && data[1] == 0x02)) && ignState == i_Operate ) {
    lastHeartBeat = millis();
    heartbeatAwait = true;
    hbState = h_Operate;
  }
  else if (data[0] == 0x00) {
    lastHeartBeat = millis();
    hbState = h_Registry;
    heartBeat();
    heartbeatAwait = false;
  }
  else if (data[0] == 0x19 && ignState == i_Off) {
    lastHeartBeat = millis();
    heartbeatAwait = true;
    hbState = h_Standby;
  }
}

void checkIgnitionState (uint32_t id, uint8_t dlc, const uint8_t *data) {

  if (data[0] == 0x07 || data[0] == 0x87) {
    ignState = i_Operate;
  }
  else if (data[0] == 0x01) {
    ignState = i_Off;
    hbState = h_Standby;
    progState = DDP_Delete;
    msgState = M_Init;
  }
  else {
    return;
  }
}
