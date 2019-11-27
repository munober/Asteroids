/*
 *  Created on: Nov 27, 2019
 *      Author: Freddy
 *      This task draws the screen of the start menu. The first screen to appear.
 */

#include "drawTaskStartMenu.h"
#include "gfx.h"
#include "ESPL_functions.h"

// Load font for ugfx
extern font_t font1;

extern SemaphoreHandle_t DrawReady;

void drawTaskStartMenu(void * params) {

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			gdispClear(Black);

			char single [1][20] = {"Singleplayer"};
			char multi [1][20] = {"Multiplayer"};
			char settings [1][20] = {"Settings"};
			char cheats [1][20] = {"Cheats"};

			gdispDrawString(120, 30, single[1],	font1, Yellow);
			gdispDrawString(120, 90, multi[1],	font1, White);
			gdispDrawString(120, 150, settings[1],	font1, White);
			gdispDrawString(120, 210, cheats[1],	font1, White);

		}
	}
}
