
#ifndef _nexstar_mr_h_
#define _nexstar_mr_h_

#include "nexstar_base.h"
#include <Arduino.h>

class NexstarMessageReceiver {
public:
	NexstarMessageReceiver();
	bool process(int data);
	bool isValid();
	NexStarMessage* getMessage();
	void reset();

protected:
	bool validate_checksum();

	bool preamble_received;
	bool length_received;
	bool finished;
	bool valid;
	uint8_t index;
	uint8_t last_index;
	long last_receive_timestamp;
	NexStarMessage message;
};

#endif
