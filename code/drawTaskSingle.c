/*
 * drawTaskSingle.c
 *
 *  Created on: Dec 2, 2019
 *      Author: lab_espl_stud04
 */

#include "includes.h"
#include "drawTaskSingle.h"

extern QueueHandle_t ButtonQueue;
extern QueueHandle_t StateQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

void drawTaskSingle(void * params) {
	struct buttons buttonStatus; // joystick queue input buffer
	const unsigned char next_state_signal_pause = PAUSE_MENU_STATE;
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE);

			// State machine input
			if (buttonCount(BUT_A)){
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			}
			if (buttonCount(BUT_B)){
				xQueueSend(StateQueue, &next_state_signal_pause, 100);
			}

            // Clear background
		    gdispClear(White);

			// Displaying text below figures
			char str[1][70] = {"SINGLE player, A for main menu, B for pause menu"};
			for (unsigned char i = 0; i < 1; i++)
							gdispDrawString(TEXT_X(str[i]) , TEXT_Y(i), str[i],	font1, Black);
		}
	}
}
