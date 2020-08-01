/******************************************************************
    Author:     Juan Menendez Blanco    <juanmb@gmail.com>

    This code is part of the NexStarAdapter project:
        https://github.com/juanmb/NexStarAdapter

    This code is based on Andre Paquette's documentation about
    the NexStar AUX protocol:
    http://www.paquettefamily.ca/nexstar/NexStar_AUX_Commands_10.pdf

*******************************************************************/

#include <Arduino.h>
#include "nexstar_aux.h"

#define AUX_BAUDRATE 19200
#define RESP_TIMEOUT 800   // timeout in milliseconds

// In Arduino DUE, we use Serial1
#define serialBegin(x) (Serial2.begin(x))
#define serialWrite(x) (Serial2.write(x))
#define serialFlush(x) (Serial2.flush())
#define serialAvailable() (Serial2.available())
#define serialRead() (Serial2.read())

uint8_t calcCRC(NexStarMessage *msg)
{
    int result = 0;
    char *data = (char*)msg;

    for (int i = 1; i < msg->header.length + 2; i++) {
        result += data[i];
    }
    return -result & 0xff;
}

NexStarAux::NexStarAux(int select_in, int select_out)
{
    select_in_pin = select_in;
    select_out_pin = select_out;
}

// Initialize pins and setup serial port
int NexStarAux::init()
{
    pinMode(select_out_pin, OUTPUT);
    digitalWrite(select_out_pin, HIGH);
    pinMode(select_in_pin, INPUT);
    serialBegin(AUX_BAUDRATE);

    return 0;
}

// Fill NexStarMessage struct
// data: payload data
// size: size of payload data
int NexStarAux::newMessage(NexStarMessage *msg, uint8_t dest, uint8_t id, uint8_t size, char* data)
{
    if (size > MAX_PAYLOAD_SIZE) {
        return ERR_INVALID;
    }
    msg->header.preamble = 0x3b;
    msg->header.length = size + 3;
    msg->header.source = DEV_HC;
    msg->header.dest = dest;
    msg->header.id = id;
    memcpy(msg->payload, data, size);
    msg->crc = calcCRC(msg);
    return 0;
}

// Send a message and receive its response
int NexStarAux::sendCommand(uint8_t dest, uint8_t id, uint8_t size, char* data)
{
    NexStarMessage msg;
    char *bytes = (char*)(&msg);

    int ret = newMessage(&msg, dest, id, size, data);
    if (ret != 0) {
        return ret;
    }
#if DEBUG
    Serial.printf("[%d] sendCommand[dst=%x,id=%x,sz=%d](start)", millis(), dest, id, size);
    Serial.println();
    Serial.print(">> ");
#endif

    digitalWrite(select_out_pin, LOW); // send_mode
    delayMicroseconds(50);
    for (int i = 0; i < size + 5; i++) {
        serialWrite(bytes[i]);
#if DEBUG
        if (bytes[i] < 10) Serial.print('0');
        Serial.print(bytes[i], 16);  Serial.print(' ');
#endif
    }
    serialWrite(msg.crc);
    serialFlush();
#if DEBUG
    if (msg.crc < 10) Serial.print('0');
    Serial.print(msg.crc, 16);  Serial.print(' ');
#endif
    digitalWrite(select_out_pin, HIGH); // receive_mode
    return 0;
}


int NexStarAux::waitForResponse(NexStarMessage *resp)
{
    long int t0 = millis();
    while (digitalRead(select_in_pin) == LOW){
        delayMicroseconds(5);
        if (millis() - t0 > RESP_TIMEOUT) {
            Serial.println("ERR_TIMEOUT in last command");
            return ERR_TIMEOUT;
        }
    }
    t0 = millis();
    while (true) {
        if (digitalRead(select_in_pin) == LOW)
            break;
        delayMicroseconds(50);
        if (millis() - t0 > RESP_TIMEOUT) {
            Serial.println("ERR_TIMEOUT in last command");
            return ERR_TIMEOUT;
        }
    }

    unsigned int pos = 0;
    char *bytes = (char*)(resp);
    t0 = millis();
    while (true) {
        delayMicroseconds(50);
        if (serialAvailable()) {
            unsigned char cc = serialRead();
            bytes[pos++] = cc;
        }
        if (pos>4) {
            int sz = bytes[1];
            if (sz == (pos-3)) break;
        }
        if (millis() - t0 > RESP_TIMEOUT) {
            Serial.println("ERR_TIMEOUT in last command");
            return ERR_TIMEOUT;
        }
    }

#if DEBUG
    Serial.println();
    Serial.print("<< ");
    for (int ii = 0; ii < pos; ii++) {
        unsigned char cc =bytes[ii];
        if (cc < 0x10)
            Serial.print('0');
        Serial.print(cc, 16);
        Serial.print(' ');
    }
    if (pos>0)
      Serial.println();
    Serial.printf("[%d] sendCommand(end)", millis());
    Serial.println();
#endif
    if (pos <= sizeof(NexStarHeader)) {
        Serial.println("ERR_BAD_SIZE in last command");
        return ERR_BAD_SIZE;
    }
    resp->crc = bytes[resp->header.length + 2];
    if (calcCRC(resp) != resp->crc) {
        Serial.println("ERR_CRC in last command");
        return ERR_CRC;
    }
    return 0;
}

int NexStarAux::setPosition(uint8_t dest, uint32_t pos)
{
    char payload[3];
    uint32To24bits(pos, payload);
    return sendCommand(dest, MC_SET_POSITION, 3, payload);
}

int NexStarAux::getPosition(uint8_t dest, uint32_t *pos)
{
    NexStarMessage resp;
    int ret = sendCommand(dest, MC_GET_POSITION, 0, NULL);
    ret = waitForResponse(&resp);
    if (!ret) {
        *pos = uint32From24bits(resp.payload);
    }
    return ret;
}

int NexStarAux::gotoPosition(uint8_t dest, bool slow, uint32_t pos)
{
    char payload[3];
    uint32To24bits(pos, payload);
    char cmdId = slow ? MC_GOTO_SLOW : MC_GOTO_FAST;
    return sendCommand(dest, cmdId, 3, payload);
}

int NexStarAux::move(uint8_t dest, bool dir, uint8_t rate)
{
    uint8_t payload[1] = { rate };
    char cmdId = dir ? MC_MOVE_POS : MC_MOVE_NEG;
    return sendCommand(dest, cmdId, 1, (char *)payload);
}

int NexStarAux::slewDone(uint8_t dest, bool *done)
{
    NexStarMessage resp;
    int ret = sendCommand(dest, MC_SLEW_DONE, 0, NULL);
    ret = waitForResponse(&resp);
    if (!ret) {
        *done = (bool)resp.payload[0];
    }
    return ret;
}

int NexStarAux::setGuiderate(uint8_t dest, bool dir, bool custom_rate, uint32_t rate)
{
    char payload[3];
    uint32To24bits(rate << 16, payload);
    char cmdId = dir ? MC_SET_POS_GUIDERATE : MC_SET_NEG_GUIDERATE;
    char msgSize = custom_rate ? 3 : 2;
    return sendCommand(dest, cmdId, msgSize, payload);
}

int NexStarAux::setApproach(uint8_t dest, bool dir)
{
    char payload[1] = { dir };
    return sendCommand(dest, MC_SET_APPROACH, 1, payload);
}

int NexStarAux::getApproach(uint8_t dest, bool *dir)
{
    NexStarMessage resp;
    int ret = sendCommand(dest, MC_GET_APPROACH, 0, NULL);
    ret = waitForResponse(&resp);
    if (!ret) {
        *dir = (bool)resp.payload[0];
    }
    return ret;
}

int NexStarAux::getVersion(uint8_t dest, char *major, char *minor)
{
    NexStarMessage resp;
    int ret = sendCommand(dest, MC_GET_VER, 0, NULL);
    ret = waitForResponse(&resp);
    if (!ret) {
        *major = resp.payload[0];
        *minor = resp.payload[1];
    }
    return ret;
}
