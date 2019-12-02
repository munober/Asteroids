/*
 * drawTaskStartMenu.c
 *
 *  Created on: Dec 2, 2019
 *      Author: lab_espl_stud04
 */
#include "includes.h"
#include "drawTaskStartMenu.h"

extern QueueHandle_t ButtonQueue;
extern QueueHandle_t StateQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

void drawTaskStartMenu(void * params) {
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

			gdispClear(Black);

			char single [1][20] = {"Singleplayer"};
			char multi [1][20] = {"Multiplayer"};
			char settings [1][20] = {"Settings"};
			char cheats [1][20] = {"Cheats"};

			char str[1][70] = {"SINGLE player, A for main menu, B for pause menu"};

			for (unsigned char i = 0; i < 1; i++){
				gdispDrawString(120, 30, single[i],	font1, Yellow);
				gdispDrawString(120, 90, multi[i],	font1, White);
				gdispDrawString(120, 150, settings[i],	font1, White);
				gdispDrawString(120, 210, cheats[i],	font1, White);
				gdispDrawString(TEXT_X(str[i]) , 230, str[i],	font1, Black);
			}
		}
	}
}
