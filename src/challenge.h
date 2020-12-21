#pragma once

#include <stdint.h>

typedef enum {
	EMPTY	= 0x10,
	ADD		= 0x24,
	RESULT	= 0x32,
	DELAY	= 0x43,
	TIMEOUT = 0x51,
	LOG		= 0x65,
} messageType;

typedef struct TLVMessage_t {
	messageType uHeader;
	uint8_t *uMessage;
	uint8_t uDatapointer;
	uint8_t uDatalength;
}TLVMessage_t;

// needs to be implemented for the challenge
void receive_ISR(uint8_t data);

// needs to be implemented for the challenge
void challenge_init();

// needs to be implemented for the challenge
void challenge_run();

