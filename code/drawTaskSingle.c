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

#define NUM_POINTS (sizeof(form)/sizeof(form[0]))

// This is defines the players ship shape
static const point form[] = {
		{-3, 0},
		{0, -6},
		{3, 0},
};


void drawTaskSingle(void * params) {
	struct buttons buttonStatus; // joystick queue input buffer
	const unsigned char next_state_signal_pause = PAUSE_MENU_STATE;
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	unsigned int exeCount = 0;

	struct asteroid asteroid_1 = { { 0 } };
	struct asteroid asteroid_2 = { { 0 } };
	struct asteroid asteroid_3 = { { 0 } };
	struct asteroid asteroid_4 = { { 0 } };
	struct asteroid asteroid_5 = { { 0 } };

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE);
			if (buttonCount(BUT_A)){
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			}
			if (buttonCount(BUT_B)){
				xQueueSend(StateQueue, &next_state_signal_pause, 100);
			}

			//Freddy code
			exeCount++;
			// North-East movement of asteroid 1
			// Start position is (-5,180)
			asteroid_1.position.x = -5 + exeCount % 185;
			asteroid_1.position.y = 180 - exeCount % 185;

			// South-East movement of asteroid 2
			asteroid_2.position.x = -5 + exeCount % 245;
			asteroid_2.position.y = 0 + exeCount % 245;

			// South movement of asteroid 3
			asteroid_3.position.x = 240;
			asteroid_3.position.y = -5 + exeCount % 245;

			// West movement of asteroid 4
			asteroid_4.position.x = 320 - exeCount % 325;
			asteroid_4.position.y = 190;

			// West-South-West movement of asteroid 5
			asteroid_5.position.x = 320 - exeCount % 325;
			asteroid_5.position.y = 40 + (exeCount % 325)/2;

			gdispClear(Black);

			// Score board
			char score [1][20] = {"9000"};
			gdispDrawString(0, 10, score[1],	font1, White);

			// Life count
			char lifes [1][20] = {"Lifes: 3"};
			gdispDrawString(290, 10, lifes[1],	font1, White);

			// Players ship
			gdispFillConvexPoly(DISPLAY_CENTER_X, DISPLAY_CENTER_Y, form, NUM_POINTS, White);

			// Asteroid 1
			gdispFillArea(asteroid_1.position.x, asteroid_1.position.y, 5, 5,
					White);

			// Asteroid 2
			gdispFillArea(asteroid_2.position.x, asteroid_2.position.y, 5, 5,
					White);

			// Asteroid 2
			gdispFillArea(asteroid_3.position.x, asteroid_3.position.y, 5, 5,
					White);

			// Asteroid 4
			gdispFillArea(asteroid_4.position.x, asteroid_4.position.y, 5, 5,
					White);

			// Asteroid 5
			gdispFillArea(asteroid_5.position.x, asteroid_5.position.y, 5, 5,
					White);

		}
	}
}
