/*
 * mainMenu.c
 *
 *  Created on: Dec 2, 2019
 *      Author: lab_espl_stud04
 */
#include "includes.h"
#include "mainMenu.h"

extern QueueHandle_t ButtonQueue;
extern QueueHandle_t StateQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

void mainMenu(void * params) {
	struct buttons buttonStatus; // joystick queue input buffer
	const unsigned char next_state_signal_pause = PAUSE_MENU_STATE;
	const unsigned char next_state_signal_single = SINGLE_PLAYER_STATE;

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE);

			// State machine input
			if (buttonCount(BUT_B)){
				xQueueSend(StateQueue, &next_state_signal_pause, 100);
			}
			if (buttonCount(BUT_C)){
				xQueueSend(StateQueue, &next_state_signal_single, 100);
			}

            // Clear background
		    gdispClear(White);

			// Displaying text below figures
			char str[1][70] = {"start menu, B for pause menu, C for single player"};
			for (unsigned char i = 0; i < 1; i++)
							gdispDrawString(TEXT_X(str[i]) , TEXT_Y(i), str[i],	font1, Black);
		}
	}
}
