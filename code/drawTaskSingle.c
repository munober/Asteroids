/*
 * drawTaskSingle.c
 *
 *  Created on: Dec 2, 2019
 *      Author: lab_espl_stud04
 */

#include "includes.h"
#include "drawTaskSingle.h"
#include "math.h"
#include "determinePlayerPosition.h"

extern QueueHandle_t StateQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;
extern QueueHandle_t JoystickQueue;
extern QueueHandle_t JoystickAngle360Queue;

#define NUM_POINTS (sizeof(form)/sizeof(form[0]))
#define NUM_POINTS_SMALL (sizeof(type_1)/sizeof(type_1[0]))
#define NUM_POINTS_MEDIUM (sizeof(type_4)/sizeof(type_4[0]))

#define HIT_LIMIT				3 		//how close the asteroids have to get to the player to register a hit

// This defines the players ship shape
static const point form[] = { { -3, 3 }, { 0, -6 }, { 3, 3 }, };

// Asteroid shapes SMALL

static const point type_1[] = { { 0, 3 }, { 2, 1 }, { 1, -2 }, { -2, -2 }, { -2,
		1 } };

static const point type_2[] = { { 0, 3 }, { 2, 1 }, { 1, -2 }, { -2, -2 }, { -2,
		1 } };

static const point type_3[] = { { 0, 3 }, { 2, 1 }, { 1, -2 }, { -2, -2 }, { -2,
		1 } };

// Asteroid shapes MEDIUM

static const point type_4[] = { { 0, 2 }, { 2, 2 }, { 4, 0 }, { 3, -4 },
		{ 0, -5 }, { -3, -4 }, { -2, 1 } };

static const point type_5[] = { { 2, 3 }, { 3, 0 }, { 3, -3 }, { 0, -2 },
		{ -2, -4 }, { -3, -1 }, { -2, 3 } };

static const point type_6[] = { { 0, 3 }, { 3, 2 }, { 3, -1 }, { 2, -4 },
		{ -2, -2 }, { -3, 0 }, { -2, 3 } };

// Asteroid shapes LARGE

//static const point type_7[] = { { 0, 3 }, { 2, 1 }, { 1, -2 }, { -2, -2 }, { -2,
//		1 } };
//static const point type_8[] = { { 0, 3 }, { 2, 1 }, { 1, -2 }, { -2, -2 }, { -2,
//		1 } };
//static const point type_9[] = { { 0, 3 }, { 2, 1 }, { 1, -2 }, { -2, -2 }, { -2,
//		1 } };

//uint16_t determinePlayerPositionX(uint8_t thrust, uint16_t angle,
//		uint16_t current_x, uint16_t current_y);
//uint16_t determinePlayerPositionY(uint8_t thrust, uint16_t angle,
//		uint16_t current_x, uint16_t current_y);

void drawTaskSingle(void * params) {
	const unsigned char next_state_signal_pause = PAUSE_MENU_STATE;
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	char str[100]; // buffer for messages to draw to display
	unsigned int life_count = 3;
	boolean life_count_lock = false;

	TickType_t hit_timestamp;
	TickType_t thrust_reset_timer;
	thrust_reset_timer = xTaskGetTickCount();
	const TickType_t delay_hit = 1000;
	const TickType_t thrust_reset_threshold = 300;

	unsigned int exeCount = 0;

	// Spawn player in display center
	struct players_ship player;
	struct player_input input;
	input.thrust = 0;
	input.angle = 1;
	// Position will be handled by function determinePlayerPosition
	player.position.x = DISPLAY_CENTER_X;
	player.position.y = DISPLAY_CENTER_Y;
	player.state = fine;

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

	struct joystick_angle_pulse joystick_internal;
	float angle_float = 0;

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			// Handling button logic down here. Also thrust and angle.
			if (life_count != 0) {
				if (buttonCount(BUT_E)) {
					xQueueSend(StateQueue, &next_state_signal_pause, 100);
				}
				if (buttonCount(BUT_A)) {
					input.thrust = 1;
					thrust_reset_timer = xTaskGetTickCount();
				} else if (xTaskGetTickCount() - thrust_reset_timer
						>= thrust_reset_threshold) {
					input.thrust = 0;
				}
			}

			if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE) {
				input.angle = joystick_internal.angle;
			}

			while (xQueueReceive(JoystickAngle360Queue, &angle_float, 0) == pdTRUE);
//			struct coord_draw temp;
////			memcpy(&temp, &player.position, sizeof(struct coord_draw));
//			temp.x = player.position.x;
//			temp.y = player.position.y;
//			player.position.x += determinePlayerPositionX(&input.thrust,
//					&input.angle, &temp.x, &temp.y);
//			player.position.y += determinePlayerPositionY(&input.thrust,
//					&input.angle, &temp.x, &temp.y);

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
			asteroid_5.position.y = 40 + (exeCount % 330) / 2;

			// North-East movement of asteroid 6
			asteroid_6.position.x = 80 + exeCount % 245;
			asteroid_6.position.y = 240 - exeCount % 245;

			// North-West movement of asteroid 7
			asteroid_7.position.x = 280 - exeCount % 250;
			asteroid_7.position.y = 240 - exeCount % 250;

			/* Check if players ship was hit by asteroid
			 * Threshold zone is a square around the players ship center with 6px side length
			 */
			if ((abs(asteroid_1.position.x - player.position.x) <= HIT_LIMIT)
					&& (abs(asteroid_1.position.y - player.position.y)
							<= HIT_LIMIT)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_2.position.x - player.position.x) <= HIT_LIMIT)
					&& (abs(asteroid_2.position.y - player.position.y)
							<= HIT_LIMIT)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_3.position.x - player.position.x) <= HIT_LIMIT)
					&& (abs(asteroid_3.position.y - player.position.y)
							<= HIT_LIMIT)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_4.position.x - player.position.x) <= HIT_LIMIT)
					&& (abs(asteroid_4.position.y - player.position.y)
							<= HIT_LIMIT)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_5.position.x - player.position.x) <= HIT_LIMIT)
					&& (abs(asteroid_5.position.y - player.position.y)
							<= HIT_LIMIT)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_6.position.x - player.position.x) <= HIT_LIMIT)
					&& (abs(asteroid_6.position.y - player.position.y)
							<= HIT_LIMIT)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_7.position.x - player.position.x) <= HIT_LIMIT)
					&& (abs(asteroid_7.position.y - player.position.y)
							<= HIT_LIMIT)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}

			gdispClear(Black);

			// Score board
			sprintf(str, "Score: 9000");
			gdispDrawString(5, 10, str, font1, White);

			// Life count
			sprintf(str, "Lifes: %d", life_count);
			gdispDrawString(280, 10, str, font1, White);

			// Debug print line for angle and thrust
			sprintf(str, "Angle: %d | Thrust: %d | 360: %f", input.angle, input.thrust, angle_float);
			gdispDrawString(0, 230, str, font1, White);

			// Players ship
			if (life_count != 0) {
				if (player.state == fine)
					gdispFillConvexPoly(player.position.x, player.position.y,
							form, NUM_POINTS, White);
				if (player.state == hit) {
					if (life_count_lock == false) {
						life_count--;
						life_count_lock = true;
					}
					if (xTaskGetTickCount() - hit_timestamp > delay_hit) {
						player.state = fine; // Reset the players ship if not yet game over
						life_count_lock = false; // Unlock the life counter
					}
				}

				// Asteroid 1
				gdispDrawPoly(asteroid_1.position.x, asteroid_1.position.y,
						type_1, NUM_POINTS_SMALL, White);

				// Asteroid 2
				gdispDrawPoly(asteroid_2.position.x, asteroid_2.position.y,
						type_2, NUM_POINTS_SMALL, White);

				// Asteroid 3
				gdispDrawPoly(asteroid_3.position.x, asteroid_3.position.y,
						type_3, NUM_POINTS_SMALL, White);

				// Asteroid 4
				gdispDrawPoly(asteroid_4.position.x, asteroid_4.position.y,
						type_4, NUM_POINTS_MEDIUM, White);

				// Asteroid 5
				gdispDrawPoly(asteroid_5.position.x, asteroid_5.position.y,
						type_5, NUM_POINTS_MEDIUM, White);

				// Asteroid 6
				gdispDrawPoly(asteroid_6.position.x, asteroid_6.position.y,
						type_5, NUM_POINTS_MEDIUM, White);

				// Asteroid 7
				gdispDrawPoly(asteroid_7.position.x, asteroid_7.position.y,
						type_6, NUM_POINTS_MEDIUM, White);
			}

			// GAME OVER
			else if (life_count == 0) {
				gdispFillArea(70, DISPLAY_CENTER_Y - 2, 180, 15, White);
				sprintf(str, "GAME OVER. Press D to quit.");
				gdispDrawString(TEXT_X(str), DISPLAY_CENTER_Y, str, font1,
						Black);
				if (buttonCount(BUT_D)) {
					life_count = 3;
					xQueueSend(StateQueue, &next_state_signal_menu, 100);
				}
			}
		}
	}
}

