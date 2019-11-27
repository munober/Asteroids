/*
 *  Created on: Nov 27, 2019
 *      Author: Freddy
 *      This task draws the screen of the pause menu
 */

#include "drawTaskPause.h"
#include "gfx.h"
#include "ESPL_functions.h"

// Load font for ugfx
extern font_t font1;

extern SemaphoreHandle_t DrawReady;

void drawTaskPause(void * params) {

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			gdispClear(Black);

			char resume [1][20] = {"Resume"};
			char quit [1][20] = {"Quit"};

			gdispDrawString(120, 90, resume[1],	font1, Yellow);
			gdispDrawString(120, 150, quit[1],	font1, White);

		}
	}
}
