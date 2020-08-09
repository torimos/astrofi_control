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
    position[0] = position[1] = 0;
    last_pos_time[0] = last_pos_time[1] = 0;
    
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
    msg->header.preamble = MSG_PREAMBLE;
    msg->header.length = size + 3;
    msg->header.source = DEV_HC;
    msg->header.dest = dest;
    msg->header.id = id;
    memcpy(msg->payload, data, size);
    msg->crc = calcCRC(msg);
    return 0;
}

// Send a message and receive its response
int NexStarAux::sendCommand(uint8_t dest, uint8_t id, uint8_t size, char* data,
            NexStarMessage *resp)
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
    send_mode = true;
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
    send_mode = false;
    return 0;
}

int NexStarAux::sendCommand(uint8_t dest, uint8_t id, uint8_t size, char* data)
{
    NexStarMessage resp;
    return sendCommand(dest, id, size, data, &resp);
}

int NexStarAux::sendCommand(uint8_t dest, uint8_t id)
{
    return sendCommand(dest, id, 0, NULL);
}

int NexStarAux::setPosition(uint8_t dest, uint32_t pos)
{
    NexStarMessage resp;
    position[dest == DEV_AZ ? 0 : 1] = pos;
    char payload[3];
    uint32To24bits(pos, payload);
    return sendCommand(dest, MC_SET_POSITION, 3, payload, &resp);
}

int NexStarAux::requestPosition(uint8_t dest)
{
    if ((millis() - last_pos_time[dest == DEV_AZ ? 0 : 1]) >= 500)
        return sendCommand(dest, MC_GET_POSITION);
    else
        return -1000;
}

uint32_t NexStarAux::getPosition(uint8_t dest)
{
    return position[dest == DEV_AZ ? 0 : 1];
}

int NexStarAux::gotoPosition(uint8_t dest, bool slow, uint32_t pos)
{
    NexStarMessage resp;
    char payload[3];
    uint32To24bits(pos, payload);

    char cmdId = slow ? MC_GOTO_SLOW : MC_GOTO_FAST;
    return sendCommand(dest, cmdId, 3, payload, &resp);
}

int NexStarAux::move(uint8_t dest, bool dir, uint8_t rate)
{
    NexStarMessage resp;
    uint8_t payload[1] = { rate };

    char cmdId = dir ? MC_MOVE_POS : MC_MOVE_NEG;
    return sendCommand(dest, cmdId, 1, (char *)payload, &resp);
}

int NexStarAux::slewDone(uint8_t dest, bool *done)
{
    NexStarMessage resp;
    int ret = sendCommand(dest, MC_SLEW_DONE, 0, NULL, &resp);
    *done = (bool)resp.payload[0];
    return ret;
}

int NexStarAux::setGuiderate(uint8_t dest, bool dir, bool custom_rate, uint32_t rate)
{
    NexStarMessage resp;

    char payload[3];
    uint32To24bits(rate << 16, payload);

    char cmdId = dir ? MC_SET_POS_GUIDERATE : MC_SET_NEG_GUIDERATE;
    char msgSize = custom_rate ? 3 : 2;
    return sendCommand(dest, cmdId, msgSize, payload, &resp);
}

int NexStarAux::setApproach(uint8_t dest, bool dir)
{
    NexStarMessage resp;
    char payload[1] = { dir };
    return sendCommand(dest, MC_SET_APPROACH, 1, payload, &resp);
}

int NexStarAux::getApproach(uint8_t dest, bool *dir)
{
    NexStarMessage resp;
    int ret = sendCommand(dest, MC_GET_APPROACH, 0, NULL, &resp);
    *dir = (bool)resp.payload[0];
    return ret;
}

int NexStarAux::getVersion(uint8_t dest, char *major, char *minor)
{
    NexStarMessage resp;
    int ret = sendCommand(dest, MC_GET_VER, 0, NULL, &resp);
    *major = resp.payload[0];
    *minor = resp.payload[1];
    return ret;
}

char nbuffer[128];
const char* cmdName(int cmd)
{
    const char *name = NULL;
    switch (cmd)
    {
        case MC_GET_POSITION: name = "GET_POSITION"; break;
        case MC_GOTO_FAST: name = "GOTO_FAST"; break;
        case MC_SET_POSITION: name = "SET_POSITION"; break;
        case MC_SET_POS_GUIDERATE: name = "SET_POS_GUIDERATE"; break;
        case MC_SET_NEG_GUIDERATE: name = "SET_NEG_GUIDERATE"; break;
        case MC_LEVEL_START: name = "LEVEL_START"; break;
        case MC_PEC_RECORD_START: name = "PEC_RECORD_START"; break;
        case MC_PEC_PLAYBACK: name = "PEC_PLAYBACK"; break;
        case MC_SET_POS_BACKLASH: name = "SET_POS_BACKLASH"; break;
        case MC_SET_NEG_BACKLASH: name = "SET_NEG_BACKLASH"; break;
        case MC_LEVEL_DONE: name = "LEVEL_DONE"; break;
        case MC_SLEW_DONE: name = "SLEW_DONE"; break;
        case MC_PEC_RECORD_DONE: name = "PEC_RECORD_DONE"; break;
        case MC_PEC_RECORD_STOP: name = "PEC_RECORD_STOP"; break;
        case MC_GOTO_SLOW: name = "GOTO_SLOW"; break;
        case MC_AT_INDEX: name = "AT_INDEX"; break;
        case MC_SEEK_INDEX: name = "SEEK_INDEX"; break;
        case MC_MOVE_POS: name = "MOVE_POS"; break;
        case MC_MOVE_NEG: name = "MOVE_NEG"; break;
        case MC_MOVE_PULSE: name = "MOVE_PULSE"; break;
        case MC_GET_PULSE_STATUS: name = "GET_PULSE_STATUS"; break;
        case MC_ENABLE_CORDWRAP: name = "ENABLE_CORDWRAP"; break;
        case MC_DISABLE_CORDWRAP: name = "DISABLE_CORDWRAP"; break;
        case MC_SET_CORDWRAP_POS: name = "SET_CORDWRAP_POS"; break;
        case MC_POLL_CORDWRAP: name = "POLL_CORDWRAP"; break;
        case MC_GET_CORDWRAP_POS: name = "GET_CORDWRAP_POS"; break;
        case MC_GET_POS_BACKLASH: name = "GET_POS_BACKLASH"; break;
        case MC_GET_NEG_BACKLASH: name = "GET_NEG_BACKLASH"; break;
        case MC_SET_AUTOGUIDE_RATE: name = "SET_AUTOGUIDE_RATE"; break;
        case MC_GET_AUTOGUIDE_RATE: name = "GET_AUTOGUIDE_RATE"; break;
        case MC_PROGRAM_ENTER: name = "PROGRAM_ENTER"; break;
        case MC_PROGRAM_INIT: name = "PROGRAM_INIT"; break;
        case MC_PROGRAM_DATA: name = "PROGRAM_DATA"; break;
        case MC_PROGRAM_END: name = "PROGRAM_END"; break;
        case MC_GET_APPROACH: name = "GET_APPROACH"; break;
        case MC_SET_APPROACH: name = "SET_APPROACH"; break;
        case MC_GET_VER: name = "GET_VER"; break;
    }
    if (name != NULL)
    {
        sprintf(nbuffer, "%s(0x%X)", name, cmd);
    }
    else
    {
        sprintf(nbuffer, "??(0x%X)", cmd);
    }
    
    return nbuffer;
}

void NexStarAux::run(){
    if (!send_mode)
    {
        if (serialAvailable()) {
            unsigned char cc = serialRead();
            if (msg_receiver.process(cc))
            {
                NexStarMessage* msg = msg_receiver.getMessage();
                switch (msg->header.id)
                {
                case MC_GET_POSITION:
                    if (msg->header.source == DEV_AZ || msg->header.source == DEV_ALT)
                    {
                        position[msg->header.source == DEV_AZ ? 0 : 1] = msg->payload[0]<<16 | msg->payload[1]<<8 | msg->payload[2];
                        last_pos_time[msg->header.source == DEV_AZ ? 0 : 1] = millis();
                    }
                    break;
                }

                Serial.printf("[%d] CMD: %s [%X >> %X] sz=%d: ", millis(), cmdName(msg->header.id), msg->header.source, msg->header.dest, msg->header.length);
                for (int ii = 0; ii < msg->header.length; ii++) {
                    unsigned char cc =msg->payload[ii];
                    if (cc < 0x10) Serial.print('0');
                    Serial.print(cc, 16);
                    Serial.print(' ');
                }
                if (msg->header.length>0) {
                    Serial.println();
                }
            }
        }
    }
}
