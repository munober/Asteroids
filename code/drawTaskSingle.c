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

	struct asteroid asteroid_1 = {0};
	int move_a1_x = 0;
	int move_a1_y = 0;


	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			exeCount++;

			// North-East movement of asteroid 1
			move_a1_x = exeCount % 325;
			move_a1_y = exeCount % 185;

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
			gdispFillArea( -5 + move_a1_x, 180 - move_a1_y, 5, 5, White);


		}
	}
}
