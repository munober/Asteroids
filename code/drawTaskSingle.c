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
#define NUM_POINTS_SMALL (sizeof(type_1)/sizeof(type_1[0]))

// Load font for ugfx
extern font_t font1;

// This is defines the players ship shape
static const point form[] = {
		{-3, 0},
		{0, -6},
		{3, 0},
};

// Asteroid shapes SMALL

static const point type_1[] = {
		{-5, 0},
		{0, -5},
		{5, 0},
		{3, 2},
		{4, -2}
};

static const point type_2[] = {
		{-3, 2},
		{1, -3},
		{5, 0},
		{3, 2},
		{4, -2}
};

static const point type_3[] = {
		{0, 2},
		{0, 0},
		{5, 0},
		{3, 0},
		{0, -2}
};

// Asteroid shapes MEDIUM

// Asteroid shapes LARGE

extern SemaphoreHandle_t DrawReady;

void drawTaskSingle(void * params) {

	unsigned int exeCount = 0;

	struct asteroid asteroid_1 = { { 0 } };
	asteroid_1.remain_hits = one;
	struct asteroid asteroid_2 = { { 0 } };
	asteroid_2.remain_hits = one;
	struct asteroid asteroid_3 = { { 0 } };
	asteroid_3.remain_hits = one;
	struct asteroid asteroid_4 = { { 0 } };
	asteroid_4.remain_hits = one;
	struct asteroid asteroid_5 = { { 0 } };
	asteroid_5.remain_hits = one;
	struct asteroid asteroid_6 = { { 0 } };
	asteroid_6.remain_hits = one;
	struct asteroid asteroid_7 = { { 0 } };
	asteroid_7.remain_hits = one;

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			exeCount++;

/*
* The following sets the movement of the asteroids. With the modulo operator it can be assured
* the asteroids return to the starting position and therefore move in a loop. The value after
* the modulo operator ('%') is with an offset so that the asteroid first fully moves out of the
* screen before returning to its start position.
 */
			// North-East movement of asteroid 1
			// Start position is (-5,180)
			asteroid_1.position.x = -5 + exeCount % 190;
			asteroid_1.position.y = 180 - exeCount % 190;

			// South-East movement of asteroid 2
			asteroid_2.position.x = -5 + exeCount % 250;
			asteroid_2.position.y = 0 + exeCount % 250;

			// South movement of asteroid 3
			asteroid_3.position.x = 240;
			asteroid_3.position.y = -5 + exeCount % 250;

			// West movement of asteroid 4
			asteroid_4.position.x = 320 - exeCount % 330;
			asteroid_4.position.y = 190;

			// West-South-West movement of asteroid 5
			asteroid_5.position.x = 320 - exeCount % 330;
			asteroid_5.position.y = 40 + (exeCount % 330)/2;

			// North-East movement of asteroid 6
			asteroid_6.position.x = 80 + exeCount % 245;
			asteroid_6.position.y = 240 - exeCount % 245;

			// North-West movement of asteroid 7
			asteroid_7.position.x = 280 - exeCount % 250;
			asteroid_7.position.y = 240 - exeCount % 250;

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
			gdispDrawPoly(asteroid_1.position.x, asteroid_1.position.y, type_1,
					NUM_POINTS_SMALL, White);

			// Asteroid 2
			gdispDrawPoly(asteroid_2.position.x, asteroid_2.position.y, type_1,
					NUM_POINTS_SMALL, White);

			// Asteroid 3
			gdispDrawPoly(asteroid_3.position.x, asteroid_3.position.y, type_1,
					NUM_POINTS_SMALL, White);

			// Asteroid 4
			gdispDrawPoly(asteroid_4.position.x, asteroid_4.position.y, type_2,
					NUM_POINTS_SMALL, White);

			// Asteroid 5
			gdispDrawPoly(asteroid_5.position.x, asteroid_5.position.y, type_2,
					NUM_POINTS_SMALL, White);

			// Asteroid 6
			gdispDrawPoly(asteroid_6.position.x, asteroid_6.position.y, type_3,
					NUM_POINTS_SMALL, White);

			// Asteroid 7
			gdispDrawPoly(asteroid_7.position.x, asteroid_7.position.y, type_3,
					NUM_POINTS_SMALL, White);
		}
	}
}
