#include "nexstar_msg_reciever.h"

NexstarMessageReceiver::NexstarMessageReceiver() :
		preamble_received(false), 
        length_received(false), 
        finished(false), 
        valid(false), 
        index(0), 
        last_index(0), 
        last_receive_timestamp(-1) {
}

NexStarMessage* NexstarMessageReceiver::getMessage() {
	return &message;
}

bool NexstarMessageReceiver::isValid() {
	return valid;
}

bool NexstarMessageReceiver::validate_checksum() {
	if (!finished || message.crc == 0) {
		return false;
	}
	int result = 0;
    uint8_t *data = (uint8_t*)&message;
    for (int i = 1; i < message.header.length + 2; i++) {
        result += data[i];
    }
    result = -result;
	return (message.crc == (result & 0xff));
}

void NexstarMessageReceiver::reset() {
	index = 0;
	preamble_received = false;
	length_received = false;
	finished = false;
	valid = false;
	last_index = 0;
	last_receive_timestamp = -1;
}

bool NexstarMessageReceiver::process(int data) {
	// Too long delay between uint8_ts?
	unsigned long current_millis=millis();
    uint8_t* bytes = (uint8_t*)(&message);
	if (last_receive_timestamp != -1
			&& current_millis - last_receive_timestamp > 250) {
		reset();
		return false;
	}
	last_receive_timestamp = current_millis;

	if ((!preamble_received) && (data == MSG_PREAMBLE)) {
		index = 0;
		preamble_received = true;
#if DEBUG
        Serial.println();
        Serial.printf("[%d]:%x ",index, data);
#endif
		bytes[index++] = data;
		return false;
	}

	if (preamble_received && (index == 1)) {
		length_received = true;
#if DEBUG
        Serial.printf("[%d]:%x ",index, data);
#endif
		bytes[index++] = data;

		// Overflow check
		if (data > 15) {
			reset();
			return false;
		}
        
		last_index = data + 3;
		return false;
	}

	if (length_received && (index > 1)) {
#if DEBUG
        Serial.printf("[%d]:%x ",index, data);
#endif
		bytes[index++] = data;
	}

	// Is it the end?
	if (index == last_index) {
        message.crc = bytes[last_index-1];
        bytes[last_index-1] = 0;
		reset();
		finished = true;

		valid = validate_checksum();

		return valid;
	}

	return false;
}

