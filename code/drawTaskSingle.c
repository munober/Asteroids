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
#include "stdlib.h" // Library for rand-function
#include "time.h"	// Library for rand-function (seed)

extern QueueHandle_t StateQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;
extern SemaphoreHandle_t timerSignal;
extern QueueHandle_t JoystickQueue;
extern QueueHandle_t LifeCountQueue;
extern QueueHandle_t HighScoresQueue;

#define NUM_POINTS_SAUCER (sizeof(saucer_shape)/sizeof(saucer_shape[0]))
#define NUM_POINTS_SMALL (sizeof(type_1)/sizeof(type_1[0]))
//#define NUM_POINTS_MEDIUM (sizeof(type_4)/sizeof(type_4[0]))
//#define NUM_POINTS_LARGE (sizeof(type_7)/sizeof(type_7[0]))

#define HIT_LIMIT_SMALL		5 		//how close the asteroids have to get to the player to register a hit
//#define HIT_LIMIT_MEDIUM	4
//#define HIT_LIMIT_LARGE		5

// Asteroid shapes SMALL

const point type_1[] = { { 0, 8 }, { 5, 4 }, { 4, -5 }, { -5, -5 }, { -5,
		4 } };
const point type_2[] = { { 0, 8 }, { 8, 4 }, { 4, -5 }, { -5, -5 }, { -5,
		4 } };
const point type_3[] = { { 5, 6 }, { 4, -2 }, { 7, -4 }, { -3, -5 }, { -5,
		2 } };


// Asteroid shapes MEDIUM

//const point type_4[] = { { 0, 8 }, { 8, 8 }, { 10, 0 }, { 10, -12 },
//		{ 0, -12 }, { -8, -12 }, { -8, 5 } };
//const point type_5[] = { { 6, 10 }, { 10, 0 }, { 10, -10 }, { 0, -6 },
//		{ -6, -14 }, { -10, -4 }, { -6, 10 } };
//const point type_6[] = { { 0, 10 }, { 10, 6 }, { 6, -3 }, { 4, -10 },
//		{ -5, -5 }, { -7, 0 }, { -5, 7 } };
//
//// Asteroid shapes LARGE
//
//const point type_7[] = { { 4, 8 }, { 12, 14 }, { 14, 4 }, { 14, -12 },
//		{ 0, -18 }, { -12, -14 }, { -18, -8 }, { -18, 4 }, { -12, 8 }, { -8, 18 } };
//const point type_8[] = { { 4, 12 }, { 12, 14 }, { 18, 0 }, { 12, -8 },
//		{ 4, -12 }, { 0, -18 }, { -8, -12 }, { -18, -12 }, { -18, 4 }, { -12, 12 } };
//const point type_9[] = { { 0, 12 }, { 24, 4 }, { 8, 4 }, { 20, -12 },
//		{ 4, 0 }, { 0, -20 }, { -4, 0 }, { -20, -12 }, { -8, 4 }, { -24, 4 } };

// Saucer shape

const point saucer_shape[] = { { -10, 3 }, { -6, 6 }, { 6, 6 }, { 10, 3 }, { -10, 3 },
		{ -6, 0 }, { 6, 0 }, { 10, 3 }, { 6, 0 }, { 4, -5 }, { -4, -5 },
		{ -6, 0 } };

/* Saucer routes:
 * There are 6 different routes, each route has 2 points where the route makes a turn -> 3 y-positions
 */
const int16_t saucer_routes[6][3] = { { 30, 120, 30 }, { 30, 120, 210 }, { 120,
		30, 120 }, { 120, 210, 120 }, { 210, 120, 210 }, { 210, 120, 30 } };

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
	int time_passed = -1; // Simple clock at top of screen

	/* This the only random value generated.
	 * This is used to only need to send 1 variable via UART.
	 */
	srand(xTaskGetTickCount());
	int16_t super_random = rand() % 241;

	// All asteroid shapes in one array: From this array we later choose which shape to draw
		const point* shapes_small[3] = {type_1, type_2, type_3};
	//	const point* shapes_medium[3] = {type_4, type_5, type_6};
	//	const point* shapes_large[3] = {type_7, type_8, type_9};
	//	const point** shapes_all[3] = {shapes_small, shapes_medium, shapes_large};

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
	player.asteroids_to_destroy = 20; // In the first level there are 20 small asteroids to destroy

	// Initialize asteroids: max. 10 asteroids are on screen at once
	// asteroid shape is either 0, 1 or 2
	struct asteroid asteroid_1 = { { 0 } };
	asteroid_1.spawn_position.x = -10;
	asteroid_1.spawn_position.y = 180;
	asteroid_1.position.x = -10;
	asteroid_1.position.y = 180;
	asteroid_1.remain_hits = one;
	asteroid_1.shape = super_random % 3;
	struct asteroid asteroid_2 = { { 0 } };
	asteroid_2.spawn_position.x = -10;
	asteroid_2.spawn_position.y = 0;
	asteroid_2.position.x = -10;
	asteroid_2.position.y = 0;
	asteroid_2.remain_hits = one;
	asteroid_2.shape = (super_random + 1) % 3;
	struct asteroid asteroid_3 = { { 0 } };
	asteroid_3.spawn_position.x = 240;
	asteroid_3.spawn_position.y = -10;
	asteroid_3.position.x = 240;
	asteroid_3.position.y = -10;
	asteroid_3.remain_hits = one;
	asteroid_3.shape = (super_random + 2) % 3;
	struct asteroid asteroid_4 = { { 0 } };
	asteroid_4.spawn_position.x = 320;
	asteroid_4.spawn_position.y = 190;
	asteroid_4.position.x = 320;
	asteroid_4.position.y = 190;
	asteroid_4.remain_hits = one;
	asteroid_4.shape = super_random % 3;
	struct asteroid asteroid_5 = { { 0 } };
	asteroid_5.spawn_position.x = 320;
	asteroid_5.spawn_position.y = 40;
	asteroid_5.position.x = 320;
	asteroid_5.position.y = 40;
	asteroid_5.remain_hits = one;
	asteroid_5.shape = (super_random + 1) % 3;
	struct asteroid asteroid_6 = { { 0 } };
	asteroid_6.spawn_position.x = 80;
	asteroid_6.spawn_position.y = 240;
	asteroid_6.position.x = 80;
	asteroid_6.position.y = 240;
	asteroid_6.remain_hits = one;
	asteroid_6.shape = (super_random + 2) % 3;
	struct asteroid asteroid_7 = { { 0 } };
	asteroid_7.spawn_position.x = 280;
	asteroid_7.spawn_position.y = 240;
	asteroid_7.position.x = 280;
	asteroid_7.position.y = 240;
	asteroid_7.remain_hits = one;
	asteroid_7.shape = super_random % 3;
	struct asteroid asteroid_8 = { { 0 } };
	asteroid_8.spawn_position.x = 180;
	asteroid_8.spawn_position.y = -10;
	asteroid_8.position.x = 180;
	asteroid_8.position.y = -10;
	asteroid_8.remain_hits = one;
	asteroid_8.shape = (super_random + 1) % 3;
	struct asteroid asteroid_9 = { { 0 } };
	asteroid_9.spawn_position.x = 70;
	asteroid_9.spawn_position.y = -10;
	asteroid_9.position.x = 70;
	asteroid_9.position.y = -10;
	asteroid_9.remain_hits = one;
	asteroid_9.shape = (super_random + 2) % 3;
	struct asteroid asteroid_10 = { { 0 } };
	asteroid_10.spawn_position.x = 320;
	asteroid_10.spawn_position.y = 180;
	asteroid_10.position.x = 320;
	asteroid_10.position.y = 180;
	asteroid_10.remain_hits = one;
	asteroid_10.shape = super_random % 3;

	// Put them asteroids inside an array
	struct asteroid all_asteroids[10] = { asteroid_1, asteroid_2, asteroid_3,
			asteroid_4, asteroid_5, asteroid_6, asteroid_7, asteroid_8,
			asteroid_9, asteroid_10 };

	// Initialize Saucer
	struct saucer saucer_1 = { { 0 } };
	saucer_1.route_number = super_random % 6;
	saucer_1.position.x = -10;
	saucer_1.position.y = saucer_routes[saucer_1.route_number][0];
	saucer_1.turning = false;
	struct saucer saucer_2 = { { 0 } };
	saucer_2.route_number = (super_random + 3) % 6;
	saucer_2.position.x = -10;
	saucer_2.position.y = saucer_routes[saucer_2.route_number][0];
	saucer_2.turning = false;

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
			 * Here we re-spawn asteroids if the player still has to destroy more than 10 asteroids.
			 * If there are only 10 or less asteroids left to destroy, no more new asteroids will be spawned.
			 */
//			if (/*  destroyed an asteroid == true*/ && (player.asteroids_to_destroy > 10)) {
//				// Detect which asteroid was destroyed
//				for (int i = 0; i <= 9; i++) {
//					if (all_asteroids[i].remain_hits == none)
//						break;
//				}
//
//				// Re-spawn the asteroid_i
//				*all_asteroids[i].remain_hits = one;
//				*all_asteroids[i].shape = rand() % 3;
//				*all_asteroids[i].position.x = *all_asteroids[i].spawn_position.x;
//				*all_asteroids[i].position.y = *all_asteroids[i].spawn_position.y;
//			}

			// This creates the seed for all the following rand-Functions
			srand(super_random);

			/*
			 * ASTEROIDS
			 * The following sets the movement of the asteroids. Offset 10 pixel (10 pixel off screen)
			 */
			// North-East movement of asteroid 1
			// rand() % 231 returns a random number between 0 and 230
			asteroid_1.position.x = asteroid_1.position.x + 1;
			asteroid_1.position.y = asteroid_1.position.y - 1;
			if ((asteroid_1.position.x >= 330)
					|| (asteroid_1.position.y <= -10)) {
				asteroid_1.position.x = -10;
				asteroid_1.position.y = rand() % 231;
			}

			// South-East movement of asteroid 2
			asteroid_2.position.x = asteroid_2.position.x + 1;
			asteroid_2.position.y = asteroid_2.position.y + 1;
			if ((asteroid_2.position.x >= 330)
					|| (asteroid_2.position.y >= 250)) {
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
			if ((asteroid_5.position.x <= -10)
					|| (asteroid_5.position.y >= 250)) {
				asteroid_5.position.x = 320;
				asteroid_5.position.y = rand() % 231;
			}

			// North-East movement of asteroid 6
			asteroid_6.position.x = asteroid_6.position.x + 1;
			asteroid_6.position.y = asteroid_6.position.y - 1;
			if ((asteroid_6.position.x >= 330)
					|| (asteroid_6.position.y <= -10)) {
				asteroid_6.position.x = rand() % 315;
				asteroid_6.position.y = 240;
			}

			// North-West movement of asteroid 7
			asteroid_7.position.x = asteroid_7.position.x - 1;
			asteroid_7.position.y = asteroid_7.position.y - 1;
			if ((asteroid_7.position.x <= -10)
					|| (asteroid_7.position.y <= -10)) {
				asteroid_7.position.x = rand() % 315;
				asteroid_7.position.y = 240;
			}

			// South-South-West movement of asteroid 8
			asteroid_8.position.x = asteroid_8.position.x - 1;
			asteroid_8.position.y = asteroid_8.position.y + 2;
			if ((asteroid_8.position.x <= -10)
					|| (asteroid_8.position.y >= 250)) {
				asteroid_8.position.x = 60 + rand() % 241;
				asteroid_8.position.y = -10;
			}

			// South-South-East movement of asteroid 9
			asteroid_9.position.x = asteroid_9.position.x + 1;
			asteroid_9.position.y = asteroid_9.position.y + 2;
			if ((asteroid_9.position.x >= 330)
					|| (asteroid_9.position.y >= 250)) {
				asteroid_9.position.x = rand() % 241;
				asteroid_9.position.y = -10;
			}

			// West-North-West movement of asteroid 10
			asteroid_10.position.x = asteroid_10.position.x - 2;
			asteroid_10.position.y = asteroid_10.position.y - 1;
			if ((asteroid_10.position.x <= -10)
					|| (asteroid_10.position.y <= -10)) {
				asteroid_10.position.x = 320;
				asteroid_10.position.y = rand() % 241;
			}

			/*
			 * SAUCERS
			 * The following sets the movement of the saucers. Offset 10 pixel (10 pixel off screen)
			 */
			// SAUCER #1, spawns after 20 sec
			if (time_passed > 20) {
				if (saucer_1.position.x == 100) {
					saucer_1.turn_number = 1;
					saucer_1.turning = true;
				}

				if (saucer_1.position.x == 200) {
					saucer_1.turn_number = 2;
					saucer_1.turning = true;
				}

				if (saucer_1.position.x >= 330) {
					saucer_1.route_number = rand() % 6; // Assign new random route
					saucer_1.position.x = -10;
					saucer_1.position.y =
							saucer_routes[saucer_1.route_number][0];
				}

				if (saucer_1.turning == true) {
					if (saucer_routes[saucer_1.route_number][saucer_1.turn_number]
							> saucer_routes[saucer_1.route_number][saucer_1.turn_number
									- 1]) {
						saucer_1.position.x++;
						saucer_1.position.y = saucer_1.position.y + 2;
					} else {
						saucer_1.position.x++;
						saucer_1.position.y = saucer_1.position.y - 2;
					}
					if (saucer_1.position.y
							== saucer_routes[saucer_1.route_number][saucer_1.turn_number])
						saucer_1.turning = false; // The saucer has reached its new y-position
				} else {
					saucer_1.position.x++;
				}
			}

			// SAUCER #2, spawns after 35 sec
			if (time_passed > 35) {
				if (saucer_2.position.x == 100) {
					saucer_2.turn_number = 1;
					saucer_2.turning = true;
				}

				if (saucer_2.position.x == 200) {
					saucer_2.turn_number = 2;
					saucer_2.turning = true;
				}

				if (saucer_2.position.x >= 330) {
					saucer_2.route_number = (rand() + 3) % 6; // Assign new random route
					saucer_2.position.x = -10;
					saucer_2.position.y =
							saucer_routes[saucer_2.route_number][0];
				}

				if (saucer_2.turning == true) {
					if (saucer_routes[saucer_2.route_number][saucer_2.turn_number]
							> saucer_routes[saucer_2.route_number][saucer_2.turn_number
									- 1]) {
						saucer_2.position.x++;
						saucer_2.position.y = saucer_2.position.y + 2;
					} else {
						saucer_2.position.x++;
						saucer_2.position.y = saucer_2.position.y - 2;
					}
					if (saucer_2.position.y
							== saucer_routes[saucer_2.route_number][saucer_2.turn_number])
						saucer_2.turning = false; // The saucer has reached its new y-position
				} else {
					saucer_2.position.x++;
				}
			}

			/* Check if players ship was hit by asteroid
			 * Threshold zone is a square around the players ship center with 6px side length
			 */
			if ((abs(asteroid_1.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_1.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_2.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_2.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_3.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_3.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_4.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_4.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_5.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_5.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_6.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_6.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_7.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_7.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_8.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_8.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_9.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_9.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}
			if ((abs(asteroid_10.position.x - player.position.x)
					<= HIT_LIMIT_SMALL)
					&& (abs(asteroid_10.position.y - player.position.y)
							<= HIT_LIMIT_SMALL)) {
				player.state = hit;
				hit_timestamp = xTaskGetTickCount();
			}


			gdispClear(Black);

			// Simple clock at top of screen
		    if (xSemaphoreTake(timerSignal, 0) == pdTRUE)
		    	time_passed++;
			sprintf(str2, "%d sec", time_passed);
			gdispDrawString(DISPLAY_CENTER_X - 5, 10, str2, font1, White);

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
				if (asteroid_1.remain_hits > 0)
				gdispDrawPoly(asteroid_1.position.x, asteroid_1.position.y,
						shapes_small[asteroid_1.shape], NUM_POINTS_SMALL, White);

				// Asteroid 2
				if (asteroid_2.remain_hits > 0)
				gdispDrawPoly(asteroid_2.position.x, asteroid_2.position.y,
						shapes_small[asteroid_2.shape], NUM_POINTS_SMALL, White);

				// Asteroid 3
				if (asteroid_3.remain_hits > 0)
				gdispDrawPoly(asteroid_3.position.x, asteroid_3.position.y,
						shapes_small[asteroid_3.shape], NUM_POINTS_SMALL, White);

				// Asteroid 4
				if (asteroid_4.remain_hits > 0)
				gdispDrawPoly(asteroid_4.position.x, asteroid_4.position.y,
						shapes_small[asteroid_4.shape], NUM_POINTS_SMALL, White);

				// Asteroid 5
				if (asteroid_5.remain_hits > 0)
				gdispDrawPoly(asteroid_5.position.x, asteroid_5.position.y,
						shapes_small[asteroid_5.shape], NUM_POINTS_SMALL, White);

				// Asteroid 6
				if (asteroid_6.remain_hits > 0)
				gdispDrawPoly(asteroid_6.position.x, asteroid_6.position.y,
						shapes_small[asteroid_6.shape], NUM_POINTS_SMALL, White);

				// Asteroid 7
				if (asteroid_7.remain_hits > 0)
				gdispDrawPoly(asteroid_7.position.x, asteroid_7.position.y,
						shapes_small[asteroid_7.shape], NUM_POINTS_SMALL, White);

				// Asteroid 8
				if (asteroid_8.remain_hits > 0)
				gdispDrawPoly(asteroid_8.position.x, asteroid_8.position.y,
						shapes_small[asteroid_8.shape], NUM_POINTS_SMALL, White);

				// Asteroid 9
				if (asteroid_9.remain_hits > 0)
				gdispDrawPoly(asteroid_9.position.x, asteroid_9.position.y,
						shapes_small[asteroid_9.shape], NUM_POINTS_SMALL, White);

				// Asteroid 10
				if (asteroid_10.remain_hits > 0)
				gdispDrawPoly(asteroid_10.position.x, asteroid_10.position.y,
						shapes_small[asteroid_10.shape], NUM_POINTS_SMALL, White);

				// Saucer 1
				gdispDrawPoly(saucer_1.position.x, saucer_1.position.y,
						saucer_shape, NUM_POINTS_SAUCER, White);

				// Saucer 2
				gdispDrawPoly(saucer_2.position.x, saucer_2.position.y,
						saucer_shape, NUM_POINTS_SAUCER, White);
			}

			// GAME OVER
			else if (life_count == 0) {
				gdispFillArea(70, DISPLAY_CENTER_Y - 2, 180, 15, White);
				sprintf(str, "GAME OVER. Press D to continue.");
				gdispDrawString(TEXT_X(str), DISPLAY_CENTER_Y, str, font1,
						Black);
				if (buttonCount(BUT_D)) {
					life_count = restart_lives;
					xQueueSend(StateQueue, &next_state_signal_highscoresinterface, 100);
				}
			}
		} // Block until screen is ready
	} // While-loop
} // Task

//This "Timer" Task sends a signal every second
void timer(void * params) {
	const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

	while (1) {
	xSemaphoreGive(timerSignal);
	vTaskDelay(xDelay);
	}
}
