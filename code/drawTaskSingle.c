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
extern QueueHandle_t LifeCountQueue;
extern QueueHandle_t HighScoresQueue;

//#define NUM_POINTS (sizeof(form)/sizeof(form[0]))
#define NUM_POINTS_SMALL (sizeof(type_1)/sizeof(type_1[0]))
#define NUM_POINTS_MEDIUM (sizeof(type_4)/sizeof(type_4[0]))
#define NUM_POINTS_LARGE (sizeof(type_7)/sizeof(type_7[0]))

#define HIT_LIMIT_SMALL		3 		//how close the asteroids have to get to the player to register a hit
#define HIT_LIMIT_MEDIUM	4
#define HIT_LIMIT_LARGE		5

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

static const point type_7[] = { { 1, 3 }, { 3, 4 }, { 4, 1 }, { 4, -3 },
		{ 0, -5 }, { -3, -4 }, { -5, -2 }, { -5, 1 }, { -3, 2 }, { -2, 5 } };
static const point type_8[] = { { 1, 3 }, { 3, 4 }, { 5, 0 }, { 3, -2 },
		{ 1, -3 }, { 0, -5 }, { -2, -3 }, { -5, -3 }, { -5, 1 }, { -3, 3 } };
static const point type_9[] = { { 0, 3 }, { 6, 1 }, { 2, 1 }, { 5, -3 },
		{ 1, 0 }, { 0, -5 }, { -1, 0 }, { -5, -3 }, { -2, 1 }, { -6, 1 } };

void drawTaskSingle(void * params) {
	const unsigned char next_state_signal_pause = PAUSE_MENU_STATE;
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	const unsigned char next_state_signal_highscoresinterface = HIGHSCORE_INTERFACE_STATE;
	char str[100]; // buffer for messages to draw to display
	char str2[100]; // another buffer for messages to draw to display
	unsigned int life_count = 3;
	unsigned int life_readin = 3;
	unsigned int restart_lives = 3;
	boolean life_count_lock = false;

	TickType_t hit_timestamp;
	TickType_t inertia_timer;
	inertia_timer = xTaskGetTickCount();
	const TickType_t delay_hit = 1000;
	const TickType_t inertia_threshold = 2000;

	unsigned int exeCount = 0;
	unsigned int thrustCount = 0;

	// Spawn player in display center
	struct direction direction;
	struct direction direction_old;
	struct players_ship player;
	struct player_input input;
	input.thrust = 0;
	input.angle = 0;
	player.position.x = DISPLAY_CENTER_X;
	player.position.y = DISPLAY_CENTER_Y;
	player.position_old.x = player.position.x;
	player.position_old.y = player.position.y;
	player.state = fine;

	// Initialize asteroids
	struct asteroid asteroid_1 = { { 0 } };
	asteroid_1.position.x = -10;
	asteroid_1.position.y = 180;
	asteroid_1.remain_hits = one;
	struct asteroid asteroid_2 = { { 0 } };
	asteroid_2.position.x = -10;
	asteroid_2.position.y = 0;
	asteroid_2.remain_hits = one;
	struct asteroid asteroid_3 = { { 0 } };
	asteroid_3.position.x = 240;
	asteroid_3.position.y = -10;
	asteroid_3.remain_hits = one;
	struct asteroid asteroid_4 = { { 0 } };
	asteroid_4.position.x = 320;
	asteroid_4.position.y = 190;
	asteroid_4.remain_hits = one;
	struct asteroid asteroid_5 = { { 0 } };
	asteroid_5.position.x = 320;
	asteroid_5.position.y = 40;
	asteroid_5.remain_hits = one;
	struct asteroid asteroid_6 = { { 0 } };
	asteroid_6.position.x = 80;
	asteroid_6.position.y = 240;
	asteroid_6.remain_hits = one;
	struct asteroid asteroid_7 = { { 0 } };
	asteroid_7.position.x = 280;
	asteroid_7.position.y = 240;
	asteroid_7.remain_hits = one;

	struct joystick_angle_pulse joystick_internal;
	float angle_float = 0;
	struct coord joy_direct;
	unsigned int moved = 0;

	float angle_x = 0;
	float angle_y = 0;

	// Player ship
	// This defines the initial shape of the player ship
	struct point form_orig[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	direction.x1 = 0;
	direction.y1 = -12;
	direction.x2 = 0;
	direction.y2 = 6;
	direction_old.x1 = 0;
	direction_old.y1 = -12;
	direction_old.x2 = 0;
	direction_old.y2 = 6;
	// These variable is changed with every tick by the joystick angle
	struct point form[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	unsigned int incr = 0;

	while (1) {
		// Reading life count down here.
		if(xQueueReceive(LifeCountQueue, &life_readin, 0) == pdTRUE){
			life_count = life_readin;
		}
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			// Handling button logic down here. Also thrust and angle.
			restart_lives = life_readin;
			if (life_count != 0) {
				if (buttonCount(BUT_E)) {
					xQueueSend(StateQueue, &next_state_signal_pause, 100);
				}
				if (buttonCountWithLiftup(BUT_A)) {
					thrustCount++;
				}
				if (thrustCount == 100) {
					thrustCount = 0;
				}
				if ((thrustCount % 2) == 1) {
					input.thrust = 1;
				}
				else if((thrustCount % 2) == 0){
					input.thrust = 0;
				}
			}

			if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE) {
				input.angle = joystick_internal.angle;
			}

//			Read joystick input directly in here
			joy_direct.x = (int16_t)(ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
			joy_direct.y = (int16_t)(255 - (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4));
			angle_x = (float)((int16_t)joy_direct.x-128);
			angle_y = (float)((int16_t)joy_direct.y-128);
			angle_float = 0;
			if (abs(joy_direct.x - 128) > 5 || abs(joy_direct.y - 128) > 5){
				if((angle_x != 0) && (angle_y != 0)){
					angle_float = (CONVERT_TO_DEG * atan2f(angle_y, angle_x)) + 90;
				}
			}

//			Make player show up at the other side of the screen when reaching screen border
			if(player.position.x >= DISPLAY_SIZE_X){
				player.position.x = 0;
			}
			else if(player.position.x <= 0){
				player.position.x = DISPLAY_SIZE_X;
			}
			if(player.position.y >= DISPLAY_SIZE_Y){
				player.position.y = 0;
			}
			else if(player.position.y <= 0){
				player.position.y = DISPLAY_SIZE_Y;
			}

//			Player movement input

			if(input.thrust){
				moved = 1;
				if(player.position.x <= DISPLAY_SIZE_X && player.position.y <= DISPLAY_SIZE_Y){
					player.position.x += (joy_direct.x - 128) / 32;
					player.position.y += (joy_direct.y - 128) / 32;
					if (joy_direct.x - 128 > 10 || joy_direct.y - 128 > 10){
						inertia_timer = xTaskGetTickCount();
					}
				}
			}

//			Player inertia old implementation
//			if(moved){
//				if(xTaskGetTickCount() - inertia_timer < inertia_threshold){
//					if((player.position.x - player.position_old.x) > 0){
//						player.position.x++;
//					}
//					else if((player.position.x - player.position_old.x) < 0){
//						player.position.x--;
//					}
//					if((player.position.y - player.position_old.y) > 0){
//						player.position.y++;
//					}
//					else if((player.position.y - player.position_old.y) < 0){
//						player.position.y--;
//					}
//				}
//				else{
//					player.position_old.x = player.position.x;
//					player.position_old.y = player.position.y;
//				}
//			}

//			Player inertia new implementation

			// Player ship rotation

			memcpy(&form, &form_orig, 3 * sizeof(struct point));
			for(incr = 0; incr < 3; incr++){
				form[incr].x = form_orig[incr].x * cos(angle_float * CONVERT_TO_RAD)
									- form_orig[incr].y * sin(angle_float * CONVERT_TO_RAD);
				form[incr].y = form_orig[incr].x * sin(angle_float * CONVERT_TO_RAD)
									+ form_orig[incr].y * cos(angle_float * CONVERT_TO_RAD);
			}

			// Get player ship direction
			direction.x1 = form[2].x;
			direction.y1 = form[2].y;
			direction.x2 = (form[1].x + form[3].x) / 2;
			direction.y2 = (form[1].y + form[3].y) / 2;

			if(moved){
				if((player.position.x - player.position_old.x) > 0){
				player.position.x++;
				}
				else if((player.position.x - player.position_old.x) < 0){
					player.position.x--;
				}
				if((player.position.y - player.position_old.y) > 0){
					player.position.y++;
				}
				else if((player.position.y - player.position_old.y) < 0){
					player.position.y--;
				}
			}
			else if((direction_old.x1 != direction.x1) || (direction_old.y1 != direction.y1)
					|| (direction_old.x2 != direction.x2) || (direction_old.y2 != direction.y2)){
				player.position_old.x = player.position.x;
				player.position_old.y = player.position.y;
			}
			direction_old.x1 = direction.x1;
			direction_old.x2 = direction.x2;
			direction_old.y1 = direction.y1;
			direction_old.y2 = direction.y2;

			exeCount++;
			/*
			 * The following sets the movement of the asteroids. Offset 10 pixel
			 */
			// North-East movement of asteroid 1
			// rand() % 231 returns a random number between 0 and 230
			asteroid_1.position.x = asteroid_1.position.x + 1;
			asteroid_1.position.y = asteroid_1.position.y - 1;
			if ((asteroid_1.position.x >= 330) || (asteroid_1.position.y <= -10)) {
				asteroid_1.position.x = -10;
				asteroid_1.position.y = rand() % 231;
			}

			// South-East movement of asteroid 2
			asteroid_2.position.x = asteroid_2.position.x + 1;
			asteroid_2.position.y = asteroid_2.position.y + 1;
			if ((asteroid_2.position.x >= 330) || (asteroid_2.position.y >= 250)) {
				asteroid_2.position.x = -10;
				asteroid_2.position.y = rand() % 231;
			}

			// South movement of asteroid 3
			asteroid_3.position.y = asteroid_3.position.y + 1;
			if (asteroid_3.position.y >= 250) {
				asteroid_3.position.x = rand() % 315;
				asteroid_3.position.y = -10;
			}

			// West movement of asteroid 4
			asteroid_4.position.x = asteroid_4.position.x - 1;
			if (asteroid_4.position.x <= -10) {
				asteroid_4.position.x = 320;
				asteroid_4.position.y = rand() % 231;
			}

			// West-South-West movement of asteroid 5
			asteroid_5.position.x = asteroid_5.position.x - 2;
			asteroid_5.position.y = asteroid_5.position.y + 1;
			if ((asteroid_5.position.x <= -10) || (asteroid_5.position.y >= 250)) {
				asteroid_5.position.x = 320;
				asteroid_5.position.y = rand() % 231;
			}

			// North-East movement of asteroid 6
			asteroid_6.position.x = asteroid_6.position.x + 1;
			asteroid_6.position.y = asteroid_6.position.y - 1;
			if ((asteroid_6.position.x >= 330) || (asteroid_6.position.y <= -10)) {
				asteroid_6.position.x = rand() % 315;
				asteroid_6.position.y = 240;
			}

			// North-West movement of asteroid 7
			asteroid_7.position.x = asteroid_7.position.x - 1;
			asteroid_7.position.y = asteroid_7.position.y - 1;
			if ((asteroid_7.position.x <= -10) || (asteroid_7.position.y <= -10)) {
				asteroid_7.position.x = rand() % 315;
				asteroid_7.position.y = 240;
			}

			/* Check if players ship was hit by asteroid
			 * Threshold zone is a square around the players ship center with 6px side length
			 */
			if ((abs(asteroid_1.position.x - player.position.x) <= HIT_LIMIT_SMALL)
					&& (abs(asteroid_1.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_2.position.x - player.position.x) <= HIT_LIMIT_SMALL)
					&& (abs(asteroid_2.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_3.position.x - player.position.x) <= HIT_LIMIT_SMALL)
					&& (abs(asteroid_3.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_4.position.x - player.position.x) <= HIT_LIMIT_SMALL)
					&& (abs(asteroid_4.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_5.position.x - player.position.x) <= HIT_LIMIT_SMALL)
					&& (abs(asteroid_5.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_6.position.x - player.position.x) <= HIT_LIMIT_SMALL)
					&& (abs(asteroid_6.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_7.position.x - player.position.x) <= HIT_LIMIT_SMALL)
					&& (abs(asteroid_7.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}

			gdispClear(Black);

			// Score board
			sprintf(str, "Score: 9000");
			gdispDrawString(5, 10, str, font1, White);

			// Life count
			sprintf(str, "Lives: %d", life_count);
			gdispDrawString(260, 10, str, font1, White);

			// Debug print line for angle and thrust
//			sprintf(str, "Angle: %d | Thrust: %d | 360: %d", input.angle, input.thrust, (uint16_t)(angle_float));
//			gdispDrawString(0, 230, str, font1, White);
//			sprintf(str2, "Axis X: %i | Axis Y: %i", joy_direct.x, joy_direct.y);
//			gdispDrawString(0, 220, str2, font1, White);


			// Players ship
			if (life_count != 0) {
				if (player.state == fine)
					gdispFillConvexPoly(player.position.x, player.position.y,
							form, (sizeof(form)/sizeof(form[0])), White);
				if (player.state == hit) {
					if (life_count_lock == false) {
						life_count--;
						life_count_lock = true;
					}
					if (xTaskGetTickCount() - hit_timestamp > delay_hit) {
						player.state = fine; // Reset the players ship if not yet game over
						life_count_lock = false; // Unlock the life counter
						player.position.x = DISPLAY_CENTER_X; // Reset player coordinates
						player.position.y = DISPLAY_CENTER_Y; // Reset player coordinates
						moved = 0; // Stop inertia until joystick input
					}
				}

				// Asteroid 1
				gdispDrawPoly(asteroid_1.position.x, asteroid_1.position.y,
						type_9, NUM_POINTS_LARGE, White);

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
					life_count = restart_lives;
					xQueueSend(StateQueue, &next_state_signal_highscoresinterface, 100);
				}
			}
		}
	}
}

