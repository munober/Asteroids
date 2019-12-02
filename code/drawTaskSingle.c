/*
 *  Created on: Nov 27, 2019
 *      Author: Freddy
 *      This task draws the screen of the single player mode
 */

#include "drawTaskSingle.h"
#include "gfx.h"
#include "ESPL_functions.h"

#define DISPLAY_SIZE_X  320
#define DISPLAY_SIZE_Y  240
#define DISPLAY_CENTER_X DISPLAY_SIZE_X/2
#define DISPLAY_CENTER_Y DISPLAY_SIZE_Y/2

#define NUM_POINTS (sizeof(form)/sizeof(form[0]))

// Load font for ugfx
extern font_t font1;

// This is defines the players ship shape
static const point form[] = {
		{-3, 0},
		{0, -6},
		{3, 0},
};

extern SemaphoreHandle_t DrawReady;

void drawTaskSingle(void * params) {

	unsigned int exeCount = 0;

	struct asteroid asteroid_1 = { { 0 } };
	struct asteroid asteroid_2 = { { 0 } };
	struct asteroid asteroid_3 = { { 0 } };
	struct asteroid asteroid_4 = { { 0 } };
	struct asteroid asteroid_5 = { { 0 } };

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

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
