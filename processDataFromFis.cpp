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
            Serial.println(F("Ошибка инициализации сообщения FIS -> CAN"));
        }
    } else if (msgState == Sys_Info) {
        if (responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
            sysTimingsFIS();
        } else if (responseAwait && millis() - lastReceivedMessaage < delayMessages) {
            return;
        } else {
            Serial.println(F("Ошибка системных сообщений FIS -> CAN"));
        }
    } else if (msgState == Status_Req) {
        if (responseAwait && millis() - lastReceivedMessaage >= delayMessages) {
            aprMessage(1);
            responseAwait = true;
            Serial.print(F("Канал успешно назначен"));
            Serial.print(F(" ddpChannel: "));
            Serial.println(ddpChannel, HEX);
            chanelStatus = true;
            msgState = End_Wait;
        } else if (responseAwait && millis() - lastReceivedMessaage < delayMessages) {
            return;
        } else {
            Serial.println(F("Ошибка подтверждения связи FIS -> CAN"));
        }
    } else if (msgState == End_Wait) {
        if (responseAwait && millis() - lastSendMessage >= delayMessages) {
            endMessage();
            responseAwait = false;
            progState = Data_Send;
            msgState = M_Init;
        } else if (responseAwait && millis() - lastReceivedMessaage < delayMessages) {
            return;
        } else {
            Serial.println(F("Неопознанная ошибка FIS to CAN"));
        }
    }
}
