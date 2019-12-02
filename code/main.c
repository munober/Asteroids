/**
 *
 * @author: Alex Hoffman alex.hoffman@tum.de (RCS, TUM)
 * 			Jonathan M��ller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 */
#include "includes.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DISPLAY_SIZE_X  		320
#define DISPLAY_SIZE_Y  		240

#define TEXT_X(TEXT)	 		DISPLAY_SIZE_X / 2 - (gdispGetStringWidth(TEXT, font1) / 2)
#define TEXT_Y(LINE)	 		DISPLAY_SIZE_Y / 2 - (gdispGetFontMetric(font1, fontHeight) * -(LINE + 0.5)) + 65

#define STATE_QUEUE_LENGTH 		 1
#define BUTTON_QUEUE_LENGTH		20
#define STATE_COUNT 3
#define STATE_ONE   1
#define STATE_TWO   2
#define STATE_THREE	3

#define NEXT_TASK   1
#define PREV_TASK   2

#define STACK_SIZE 200

// Start and stop bytes for the UART protocol (READ-ONLY)
static const uint8_t startByte = 0xAA, stopByte = 0x55;

// Load font for ugfx
font_t font1;

// Function prototypes
// General
void frameSwapTask(void * params);
void basicStateMachine(void * params);
void checkButtons(void * params);
void mainMenu(void * params);

// Exercise 3.2.5
void sendPositionUART(void * params);
void receivePositionUART(void * params);


// General
QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;
SemaphoreHandle_t DrawReady; // After swapping buffer calll drawing

// General
QueueHandle_t ButtonQueue;
QueueHandle_t StateQueue;
TaskHandle_t frameSwapHandle;
TaskHandle_t mainMenuHandle;
TaskHandle_t singlePlayerDisplay;
TaskHandle_t stateMachineHandle;

// Exercise 3.2.5
TaskHandle_t sendPositionUARTHandle;
TaskHandle_t receivePositionUARTHandle;

int main(void){

	// Initialize Board functions and graphics
	ESPL_SystemInit();
	font1 = gdispOpenFont("DejaVuSans24*");
	// General
	ButtonQueue = xQueueCreate(BUTTON_QUEUE_LENGTH, sizeof(struct buttons));
	StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));

	ESPL_DisplayReady = xSemaphoreCreateBinary();
	DrawReady = xSemaphoreCreateBinary();

	// Initializes Tasks with their respective priority
	// Core tasks
	xTaskCreate(frameSwapTask, "frameSwapper", 1000, NULL, 5, &frameSwapHandle);
	xTaskCreate(basicStateMachine, "StateMachine", 1000, NULL, 3, &stateMachineHandle);
	xTaskCreate(checkButtons, "checkButtons", 1000, NULL, 4, NULL);

	// Display tasks for the different exercises
	xTaskCreate(mainMenu, "mainMenu", 1000, NULL, 2, &mainMenuHandle);

	// Exercise 3.2.5
	xTaskCreate(sendPositionUART, "sendPositionUART", 1000, NULL, 4, &sendPositionUARTHandle);
	xTaskCreate(receivePositionUART, "receivePositionUART", 1000, NULL, 4, &receivePositionUARTHandle);

	// Initial task suspends
	vTaskSuspend(mainMenuHandle);
    vTaskSuspend(receivePositionUARTHandle);
    vTaskSuspend(sendPositionUARTHandle);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}

void frameSwapTask(void * params) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t frameratePeriod = 20;

	while (1){
		// Draw next frame
		xSemaphoreGive(DrawReady);
		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// Swap buffers
		ESPL_DrawLayer();
		vTaskDelayUntil(&xLastWakeTime, frameratePeriod);
	}
}

void changeState(volatile unsigned char *state, unsigned char forwards) {
	switch (forwards) {
	case 0:
		if (*state == 0)
			*state = STATE_COUNT;
		else
			(*state)--;
		break;
	case 1:
		if (*state == STATE_COUNT)
			*state = 0;
		else
			(*state)++;
		break;
	default:
		break;
	}
}

void basicStateMachine(void * params) {
	unsigned char current_state = 1; // Default state
	unsigned char state_changed = 1; // Only re-evaluate state if it has changed
	unsigned char input = 0;
	while (1) {
		if (state_changed)
			goto initial_state;

		// Handle state machine input
		if (xQueueReceive(StateQueue, &input, portMAX_DELAY) == pdTRUE) {
			if (input == NEXT_TASK) {
				changeState(&current_state, 1);
				state_changed = 1;
			}
			else if (input == PREV_TASK) {
				changeState(&current_state, 0);
				changeState(&current_state, 0);
				state_changed = 1;
			}
		}
		initial_state:
		// Handle current state
		if (state_changed) {
			switch (current_state) {
			case STATE_ONE:
				vTaskSuspend(receivePositionUARTHandle);
				vTaskSuspend(sendPositionUARTHandle);
				vTaskResume(mainMenuHandle);
				state_changed = 0;
				break;
			case STATE_TWO:
				vTaskSuspend(mainMenuHandle);
				vTaskResume(sendPositionUARTHandle);
				vTaskResume(receivePositionUARTHandle);
				state_changed = 0;
				break;
			case STATE_THREE:
				vTaskSuspend(receivePositionUARTHandle);
				vTaskSuspend(sendPositionUARTHandle);
				vTaskSuspend(mainMenuHandle);
				state_changed = 0;
				break;
			default:
				break;
			}
		}
	}
}

void mainMenu(void * params) {
	struct buttons buttonStatus; // joystick queue input buffer
	const unsigned char next_state_signal = NEXT_TASK;

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE);

			// State machine input
			if (buttonCount(BUT_E)){
				xQueueSend(StateQueue, &next_state_signal, 100);
			}

            // Clear background
		    gdispClear(White);

			// Displaying text below figures
			char str[1][70] = {"ASTEROIDS FROM TEO espl"};
			for (unsigned char i = 0; i < 1; i++)
							gdispDrawString(TEXT_X(str[i]) , TEXT_Y(i), str[i],	font1, Black);
		}
	}
}



// Exercise 3.2.5
void sendPositionUART(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t pollingRate = 20;
	struct buttons buttonStatus;
	struct coord prev_joystick, abs_joystick;
	const int joystick_threshold = 20;
	unsigned char send=0;
	while(1){
		xQueueReceive(ButtonQueue, &buttonStatus, 100);
		const uint8_t checksum = buttonStatus.joystick.x ^ buttonStatus.joystick.y
				^ buttonStatus.A ^ buttonStatus.B ^ buttonStatus.C ^ buttonStatus.D
				^ buttonStatus.E ^ buttonStatus.K;
		if(buttonStatus.A)
			send = 1;
		if(buttonStatus.B)
			send = 1;
		if(buttonStatus.C)
			send = 1;
		if(buttonStatus.D)
			send = 1;
		if(buttonStatus.K)
			send = 1;
		abs_joystick.x = abs(buttonStatus.joystick.x - prev_joystick.x);
		abs_joystick.y = abs(buttonStatus.joystick.y - prev_joystick.y);
		if(abs_joystick.x > joystick_threshold)
			send = 1;
		if(abs_joystick.y > joystick_threshold)
			send = 1;
		if(send){
			UART_SendData(startByte);
			UART_SendData(buttonStatus.joystick.x);
			UART_SendData(buttonStatus.joystick.y);
			UART_SendData(buttonStatus.A);
			UART_SendData(buttonStatus.B);
			UART_SendData(buttonStatus.C);
			UART_SendData(buttonStatus.D);
			UART_SendData(buttonStatus.E);
			UART_SendData(buttonStatus.K);
			UART_SendData(checksum);
			UART_SendData(stopByte);
			memcpy(&prev_joystick, &buttonStatus.joystick, sizeof(struct coord));
		}
		send=0;
		vTaskDelayUntil(&xLastTickTime, pollingRate);
	}
}

void receivePositionUART(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t pollingRate = 20;
	char input;
	uint8_t pos = 0;
	char checksum;
	char buffer[11] = { { 0 } };
	while (1) {
		// wait for data in queue
		xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);
		// decode package by buffer position
		switch (pos) {
		// start byte
		case 0:
			if (input != startByte)
				break;
			buffer[pos] = input;
			pos++;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			// read received data in buffer
			buffer[pos] = input;
			pos++;
			break;
		case 10:
			// Check if package is corrupted
			checksum = buffer[1] ^ buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5]
					^ buffer[6] ^ buffer[7] ^ buffer[8];
			if (input == stopByte && checksum == buffer[9]) {
//				buttonStatus_internal.joystick.x = buffer[1];
//				buttonStatus_internal.joystick.y = buffer[2];
//				buttonStatus_internal.A = buffer[3];
//				buttonStatus_internal.B = buffer[4];
//				buttonStatus_internal.C = buffer[5];
//				buttonStatus_internal.D = buffer[6];
//				buttonStatus_internal.E = buffer[7];
//				buttonStatus_internal.K = buffer[8];
			}
			pos = 0;
			break;
		}
		vTaskDelayUntil(&xLastTickTime, pollingRate);
	}
}


void checkButtons(void * params) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	struct buttons buttonStatus = { { 0 } };
	const TickType_t PollingRate = 20;

	while (TRUE) {
		// Remember last joystick values
		buttonStatus.joystick.x = (uint8_t)(
				ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		buttonStatus.joystick.y = (uint8_t) 255
				- (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);
		buttonStatus.joystick_direct.x = (uint8_t)(
				ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		buttonStatus.joystick_direct.y = (uint8_t) 255
				- (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);
		buttonStatus.A = buttonCount(BUT_A);
		buttonStatus.B = buttonCount(BUT_B);
		buttonStatus.C = buttonCount(BUT_C);
		buttonStatus.D = buttonCount(BUT_D);
		buttonStatus.E = 0;
		buttonStatus.K = buttonCount(BUT_K);
		xQueueSend(ButtonQueue, &buttonStatus, 0);
		vTaskDelayUntil(&xLastWakeTime, PollingRate);
	}
}

void vApplicationIdleHook() {
	while (TRUE) {
	};
}

void vApplicationMallocFailedHook() {
	while (TRUE) {
	};
}

