#include <FreeRTOS.h>
#include <task.h>
#include <console.h>
#include <stdlib.h>
#include "challenge.h"

#define MAX_BUFFER_SIZE 16

TLVMessage_t TLVReceive;
TLVMessage_t TLVSend;
uint8_t receiveGuard = 0;

/**
 * Call this to "send" data over the (simulated) serial interface.
 * @param message message buffer
 * @param length length of the message
 */
void send(uint8_t* message, uint32_t length);

/**
 * This will get called for each byte of data received.
 * @param data received byte
 */
void receive_ISR(uint8_t data) {
	receiveGuard = 1;
	if (TLVReceive.uDatalength == 0) {
		TLVReceive.uHeader = data;
		TLVReceive.uDatalength++;
	}
	else {
		TLVReceive.uMessage[TLVReceive.uDatapointer] = data;
		TLVReceive.uDatapointer++;
		TLVReceive.uDatalength++;
	}
	receiveGuard = 0;
}

void TLVMessage_init(TLVMessage_t* TLVMessage) {
	TLVMessage->uDatalength = 0;
	TLVMessage->uDatapointer = 0;
	TLVMessage->uMessage = malloc(16);
	TLVMessage->uHeader = 0;
}

/**
 * Initialize challenge. This is called once on startup, before any interrupts can occur.
 */
void challenge_init() {
	TLVMessage_init(&TLVReceive);
	TLVMessage_init(&TLVSend);
}

void challenge_add() {
	TLVSend.uHeader = RESULT;
	TLVSend.uMessage[0] = TLVReceive.uMessage[0] + TLVReceive.uMessage[2];
	TLVSend.uMessage[1] = TLVReceive.uMessage[1] + TLVReceive.uMessage[3];
	TLVSend.uDatalength = 3;
}

void challenge_delay() {
	TickType_t ms_begin = xTaskGetTickCount();
	uint16_t time_elapsed = 0;
	uint16_t delay = TLVReceive.uMessage[0] << 8 + TLVReceive.uMessage[1];
	console_print("Delay = %u ms\n", delay);
	TLVSend.uHeader = TIMEOUT;
	TLVSend.uMessage[0] = TLVReceive.uMessage[2];
	TLVSend.uDatalength = 2;
	while (time_elapsed <= delay) {
		time_elapsed = ms_begin - xTaskGetTickCount();
	}
}

void challenge_log() {
	console_print("%s\n", TLVReceive.uMessage);
}

/**
 * Main function for the coding challenge. Must not return.
 *
 * Please use the following API functions if necessary:
 *
 * print string to stdout
 * console_print("format %d", 123);
 *
 * millisecond delay
 * vTaskDelay(123);
 *
 * get elapsed milliseconds
 * TickType_t ms = xTaskGetTickCount();
 */
void challenge_run() {
    // TODO: insert awesome stuff here
	for (;;) {
		if (TLVReceive.uDatalength != 0 && receiveGuard == 0) {
			vTaskDelay(10);
			if (TLVReceive.uHeader == EMPTY) {
				TLVSend = TLVReceive;
				TLVSend.uDatalength = 1;
				console_print("This is empty callback function\n");
			}
			if (TLVReceive.uHeader == ADD && TLVReceive.uDatalength >= 4) {
				challenge_add();
				console_print("%u + %u = %u\n", TLVReceive.uMessage[0] << 8 + TLVReceive.uMessage[1], TLVReceive.uMessage[2] << 8 + TLVReceive.uMessage[3],
					TLVSend.uMessage[0] << 8 + TLVSend.uMessage[1]);
				console_print("This is add callback function\n");
			}
			if (TLVReceive.uHeader == DELAY) {
				challenge_delay();
			}
			if (TLVReceive.uHeader == LOG) {
				challenge_log;
			}
			uint8_t tx[] = { TLVSend.uHeader, TLVSend.uMessage[0], TLVSend.uMessage[1] };
			send(tx, TLVSend.uDatalength);
			TLVReceive.uDatalength = 0;
			TLVReceive.uDatapointer = 0;
			console_print("This is challenge callback function\n");
		}
	}
}