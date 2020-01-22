/*
 * drawTaskSingleLevel3.c
 *
 *  Created on: Dec 18, 2019
 *      Authors: Teodor Fratiloiu
 * 				 Frederik Zumegen
 */

#include "includes.h"
#include "drawTaskSingleLevel3.h"
#include "math.h"
#include "stdlib.h" // Library for rand-function
#include "time.h"	// Library for rand-function (seed)

extern QueueHandle_t StateQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;
extern QueueHandle_t JoystickQueue;
extern QueueHandle_t LifeCountQueue3;
extern QueueHandle_t HighScoresQueue;
extern QueueHandle_t StartingScoreQueue;
extern QueueHandle_t FPSQueue;

#define NUM_POINTS_SAUCER (sizeof(saucer_shape)/sizeof(saucer_shape[0]))
#define NUM_POINTS_SMALL (sizeof(type_1)/sizeof(type_1[0]))
#define NUM_POINTS_MEDIUM (sizeof(type_4)/sizeof(type_4[0]))
#define NUM_POINTS_LARGE (sizeof(type_7)/sizeof(type_7[0]))

void drawTaskSingleLevel3 (void * params){
	char user_help[1][70];
	// Asteroid shapes SMALL
	const point type_1[] = { { 0, 8 }, { 5, 4 }, { 4, -5 }, { -5, -5 }, { -5, 4 } };
	const point type_2[] = { { 0, 8 }, { 8, 4 }, { 4, -5 }, { -5, -5 }, { -5, 4 } };
	const point type_3[] = { { 5, 6 }, { 4, -2 }, { 7, -4 }, { -3, -5 }, { -5, 2 } };

	// Asteroid shapes MEDIUM
	const point type_4[] = { { 0, 8 }, { 8, 8 }, { 10, 0 }, { 10, -12 },
			{ 0, -12 }, { -8, -12 }, { -8, 5 } };
	const point type_5[] = { { 6, 10 }, { 10, 0 }, { 10, -10 }, { 0, -6 },
			{ -6, -14 }, { -10, -4 }, { -6, 10 } };
	const point type_6[] = { { 0, 10 }, { 10, 6 }, { 6, -3 }, { 4, -10 },
			{ -5, -5 }, { -7, 0 }, { -5, 7 } };

	// Asteroid shapes LARGE
	const point type_7[] = { { 4, 8 }, { 12, 14 }, { 14, 4 }, { 14, -12 },
			{ 0, -18 }, { -12, -14 }, { -18, -8 }, { -18, 4 }, { -12, 8 }, { -8, 18 } };
	const point type_8[] = { { 4, 12 }, { 12, 14 }, { 18, 0 }, { 12, -8 },
			{ 4, -12 }, { 0, -18 }, { -8, -12 }, { -18, -12 }, { -18, 4 }, { -12, 12 } };
	const point type_9[] = { { 0, 12 }, { 24, 4 }, { 8, 4 }, { 20, -12 },
			{ 4, 0 }, { 0, -20 }, { -4, 0 }, { -20, -12 }, { -8, 4 }, { -24, 4 } };

	// Saucer shape
	const point saucer_shape[] = { { -10, 3 }, { -6, 6 }, { 6, 6 }, { 10, 3 }, { -10, 3 },
			{ -6, 0 }, { 6, 0 }, { 10, 3 }, { 6, 0 }, { 4, -5 }, { -4, -5 },
			{ -6, 0 } };

	/* Saucer routes:
	 * There are 6 different routes, each route has 2 points where the route makes a turn -> 3 y-positions
	 */
	const int16_t saucer_routes[6][3] = { { 30, 120, 30 }, { 30, 120, 210 }, { 120,
			30, 120 }, { 120, 210, 120 }, { 210, 120, 210 }, { 210, 120, 30 } };

//	Next possible states
	const unsigned char next_state_signal_highscoresinterface = HIGHSCORE_INTERFACE_STATE;

	//	A few buffer, empty strings to print to
	char str[100]; // buffer for messages to draw to display
	char str2[100]; // another buffer for messages to draw to display
	

	unsigned int exeCount = 0;
	gamestart3: // Will return here if game over
	if(exeCount != 0){
		xQueueSend(StateQueue, &next_state_signal_highscoresinterface, 100);
	}
	boolean no_extra_life = true;
	TickType_t new_life_timer = xTaskGetTickCount();

//	Variables to store the number of lives
	unsigned int life_count = STARTING_LIVES_LEVEL_THREE;	// For standard game mode
	unsigned int life_readin = 3;	// Will be filled with queue readin
	unsigned int restart_lives = 3;	// Lives to be had when game is restarted
	boolean life_count_lock = false;	// Used for delays when player is hit
	int time_passed = 0; // Simple clock at top of screen
	boolean infinite_respawn = true;

	boolean one_asteroid_hit_small = false;
	int16_t score = LEVEL_THREE_SCORE_THRESHOLD;
	xQueueReceive(StartingScoreQueue, &score, 0);

	/* This the only random value generated.
	 * This is used to only need to send 1 variable via UART.
	 */
	srand(xTaskGetTickCount());
	int16_t super_random = rand() % 241;

	// All asteroid shapes in one array: From this array we later choose which shape to draw
	const point* shapes_small[3] = {type_1, type_2, type_3};
	const point* shapes_medium[3] = {type_4, type_5, type_6};
	const point* shapes_large[3] = {type_7, type_8, type_9};
	const point** shapes_all[3] = {shapes_small, shapes_medium, shapes_large};

	TickType_t hit_timestamp;
	TickType_t hit_saucer_timestamp = xTaskGetTickCount();
	const TickType_t delay_hit = 1000;
	const TickType_t shot_delay = 1000;

	// Timer stuff
	const TickType_t one_second = 1000 / portTICK_PERIOD_MS;
	const TickType_t one_and_a_half_seconds = 1500 / portTICK_PERIOD_MS;
	const TickType_t two_seconds = 2000 / portTICK_PERIOD_MS;
	TickType_t lastTime_1 = xTaskGetTickCount();
	TickType_t lastTime_2 = xTaskGetTickCount();
	TickType_t lastTime_3 = xTaskGetTickCount();

	unsigned int thrustCount = 0;
	int i = 0;
	int j = 0;

	boolean state_pause = false;

// 	Spawn player in display center
	struct direction direction;
	struct players_ship player;
	struct player_input input;
	input.thrust = 0;
	input.angle = 0;
	input.shots_fired = 0;
	player.position.x = DISPLAY_CENTER_X;
	player.position.y = DISPLAY_CENTER_Y;
	player.position_old.x = player.position.x;
	player.position_old.y = player.position.y;
	player.state = fine;
//	Number of asteroids to be destroyed in this level
	int asteroids_to_destroy_small = TO_DESTROY_LEVEL_3_SMALL;
	int asteroids_to_destroy_medium = TO_DESTROY_LEVEL_3_MEDIUM;
	int asteroids_to_destroy_large = TO_DESTROY_LEVEL_3_LARGE;

	// Initialize asteroids: Up to 16 asteroids can be on screen at once
	// offset: 50px for large (a1-a8); 20px for medium (a9-a16)
	// asteroid shape is either 0, 1 or 2

	struct asteroid asteroid_1 = { { 0 } };
	asteroid_1.spawn_position.x = -50;
	asteroid_1.spawn_position.y = 180;
	asteroid_1.spawn_side = left;
	asteroid_1.position.x = -50;
	asteroid_1.position.y = 180;
	asteroid_1.remain_hits = three;
	asteroid_1.shape = super_random % 3;
	asteroid_1.position_locked = false;
	asteroid_1.initial_orientation = NE;
	asteroid_1.orientation = NE;
	struct asteroid asteroid_2 = { { 0 } };
	asteroid_2.spawn_position.x = -50;
	asteroid_2.spawn_position.y = 0;
	asteroid_2.spawn_side = left;
	asteroid_2.position.x = -50;
	asteroid_2.position.y = 0;
	asteroid_2.remain_hits = one;
	asteroid_2.shape = (super_random + 1) % 3;
	asteroid_2.position_locked = true;
	asteroid_2.initial_orientation = SE;
	asteroid_2.orientation = SE;
	struct asteroid asteroid_3 = { { 0 } };
	asteroid_3.spawn_position.x = 240;
	asteroid_3.spawn_position.y = -50;
	asteroid_3.spawn_side = up;
	asteroid_3.position.x = 240;
	asteroid_3.position.y = -50;
	asteroid_3.remain_hits = two;
	asteroid_3.shape = (super_random + 2) % 3;
	asteroid_3.position_locked = true;
	asteroid_3.initial_orientation = S;
	asteroid_3.orientation = S;
	struct asteroid asteroid_4 = { { 0 } };
	asteroid_4.spawn_position.x = 370;
	asteroid_4.spawn_position.y = 190;
	asteroid_4.spawn_side = right;
	asteroid_4.position.x = 370;
	asteroid_4.position.y = 190;
	asteroid_4.remain_hits = one;
	asteroid_4.shape = super_random % 3;
	asteroid_4.position_locked = true;
	asteroid_4.initial_orientation = W;
	asteroid_4.orientation = W;
	struct asteroid asteroid_5 = { { 0 } };
	asteroid_5.spawn_position.x = 370;
	asteroid_5.spawn_position.y = 40;
	asteroid_5.spawn_side = right;
	asteroid_5.position.x = 370;
	asteroid_5.position.y = 40;
	asteroid_5.remain_hits = three;
	asteroid_5.shape = (super_random + 1) % 3;
	asteroid_5.position_locked = false;
	asteroid_5.initial_orientation = W;
	asteroid_5.orientation = W;
	struct asteroid asteroid_6 = { { 0 } };
	asteroid_6.spawn_position.x = 80;
	asteroid_6.spawn_position.y = 290;
	asteroid_6.spawn_side = down;
	asteroid_6.position.x = 80;
	asteroid_6.position.y = 290;
	asteroid_6.remain_hits = one;
	asteroid_6.shape = (super_random + 2) % 3;
	asteroid_6.position_locked = true;
	asteroid_6.initial_orientation = NE;
	asteroid_6.orientation = NE;
	struct asteroid asteroid_7 = { { 0 } };
	asteroid_7.spawn_position.x = 280;
	asteroid_7.spawn_position.y = 290;
	asteroid_7.spawn_side = down;
	asteroid_7.position.x = 280;
	asteroid_7.position.y = 290;
	asteroid_7.remain_hits = two;
	asteroid_7.shape = super_random % 3;
	asteroid_7.position_locked = true;
	asteroid_7.initial_orientation = NW;
	asteroid_7.orientation = NW;
	struct asteroid asteroid_8 = { { 0 } };
	asteroid_8.spawn_position.x = 180;
	asteroid_8.spawn_position.y = -50;
	asteroid_8.spawn_side = up;
	asteroid_8.position.x = 180;
	asteroid_8.position.y = -50;
	asteroid_8.remain_hits = one;
	asteroid_8.shape = (super_random + 1) % 3;
	asteroid_8.position_locked = true;
	asteroid_8.initial_orientation = SW;
	asteroid_8.orientation = SW;
	struct asteroid asteroid_9 = { { 0 } };
	asteroid_9.spawn_position.x = 70;
	asteroid_9.spawn_position.y = -20;
	asteroid_9.spawn_side = up;
	asteroid_9.position.x = 70;
	asteroid_9.position.y = -20;
	asteroid_9.remain_hits = two;
	asteroid_9.shape = (super_random + 2) % 3;
	asteroid_9.position_locked = false;
	asteroid_9.initial_orientation = SE;
	asteroid_9.orientation = SE;
	struct asteroid asteroid_10 = { { 0 } };
	asteroid_10.spawn_position.x = 340;
	asteroid_10.spawn_position.y = 180;
	asteroid_10.spawn_side = right;
	asteroid_10.position.x = 340;
	asteroid_10.position.y = 180;
	asteroid_10.remain_hits = one;
	asteroid_10.shape = super_random % 3;
	asteroid_10.position_locked = true;
	asteroid_10.initial_orientation = NW;
	asteroid_10.orientation = NW;
	struct asteroid asteroid_11 = { { 0 } };
	asteroid_11.spawn_position.x = -20;
	asteroid_11.spawn_position.y = 100;
	asteroid_11.spawn_side = left;
	asteroid_11.position.x = -20;
	asteroid_11.position.y = 100;
	asteroid_11.remain_hits = two;
	asteroid_11.shape = super_random % 3;
	asteroid_11.position_locked = false;
	asteroid_11.initial_orientation = SE;
	asteroid_11.orientation = SE;
	struct asteroid asteroid_12 = { { 0 } };
	asteroid_12.spawn_position.x = -20;
	asteroid_12.spawn_position.y = 220;
	asteroid_12.spawn_side = left;
	asteroid_12.position.x = -20;
	asteroid_12.position.y = 220;
	asteroid_12.remain_hits = one;
	asteroid_12.shape = super_random % 3;
	asteroid_12.position_locked = true;
	asteroid_12.initial_orientation = NE;
	asteroid_12.orientation = NE;
	struct asteroid asteroid_13 = { { 0 } };
	asteroid_13.spawn_position.x = 30;
	asteroid_13.spawn_position.y = 260;
	asteroid_13.spawn_side = down;
	asteroid_13.position.x = 30;
	asteroid_13.position.y = 260;
	asteroid_13.remain_hits = two;
	asteroid_13.shape = super_random % 3;
	asteroid_13.position_locked = false;
	asteroid_13.initial_orientation = N;
	asteroid_13.orientation = N;
	struct asteroid asteroid_14 = { { 0 } };
	asteroid_14.spawn_position.x = 190;
	asteroid_14.spawn_position.y = 260;
	asteroid_14.spawn_side = down;
	asteroid_14.position.x = 190;
	asteroid_14.position.y = 260;
	asteroid_14.remain_hits = one;
	asteroid_14.shape = super_random % 3;
	asteroid_14.position_locked = true;
	asteroid_14.initial_orientation = NE;
	asteroid_14.orientation = NE;
	struct asteroid asteroid_15 = { { 0 } };
	asteroid_15.spawn_position.x = 110;
	asteroid_15.spawn_position.y = -20;
	asteroid_15.spawn_side = up;
	asteroid_15.position.x = 110;
	asteroid_15.position.y = -20;
	asteroid_15.remain_hits = two;
	asteroid_15.shape = super_random % 3;
	asteroid_15.position_locked = false;
	asteroid_15.initial_orientation = SW;
	asteroid_15.orientation = SW;
	struct asteroid asteroid_16 = { { 0 } };
	asteroid_16.spawn_position.x = 340;
	asteroid_16.spawn_position.y = 120;
	asteroid_16.spawn_side = right;
	asteroid_16.position.x = 340;
	asteroid_16.position.y = 120;
	asteroid_16.remain_hits = one;
	asteroid_16.shape = super_random % 3;
	asteroid_16.position_locked = true;
	asteroid_16.initial_orientation = NW;
	asteroid_16.orientation = NW;

	// Putting them asteroid variables inside a single array
	struct asteroid* all_asteroids[16] = { &asteroid_1, &asteroid_2,
			&asteroid_3, &asteroid_4, &asteroid_5, &asteroid_6, &asteroid_7,
			&asteroid_8, &asteroid_9, &asteroid_10, &asteroid_11, &asteroid_12,
			&asteroid_13, &asteroid_14, &asteroid_15, &asteroid_16 };

	// Function to set the asteroids spawn position based on its spawn side
	void setSpawnPosition(sides spawn_side, int i, uint32_t random) {
		switch (spawn_side) {
		case left: 	all_asteroids[i]->position.x = all_asteroids[i]->spawn_position.x;
					all_asteroids[i]->position.y = 80 + random % 140; break;
		case right:	all_asteroids[i]->position.x = all_asteroids[i]->spawn_position.x;
					all_asteroids[i]->position.y = 20 + random % 200; break;
		case up:	all_asteroids[i]->position.x = 20 + random % 280;
					all_asteroids[i]->position.y = all_asteroids[i]->spawn_position.y; break;
		case down:	all_asteroids[i]->position.x = 20 + random % 280;
					all_asteroids[i]->position.y = all_asteroids[i]->spawn_position.y; break;
		}
	}

	// Initialize flying saucers
	struct saucer saucer_1 = { { 0 } };
	saucer_1.route_number = super_random % 6;
	saucer_1.position.x = -15;
	saucer_1.position.y = saucer_routes[saucer_1.route_number][0];
	saucer_1.turning = false;
	saucer_1.remain_hits = two;
	saucer_1.shot_number = 0;
	for (i = 0; i <= 9; i++) {
		saucer_1.shot_fired[i] = false;
		saucer_1.shots[i].x = 0;
		saucer_1.shots[i].y = 0;
	}
	struct saucer saucer_2 = { { 0 } };
	saucer_2.route_number = (super_random + 3) % 6;
	saucer_2.position.x = -15;
	saucer_2.position.y = saucer_routes[saucer_2.route_number][0];
	saucer_2.turning = false;
	saucer_2.remain_hits = two;
	saucer_2.shot_number = 0;
	for (i = 0; i <= 9; i++) {
		saucer_2.shot_fired[i] = false;
		saucer_2.shots[i].x = 0;
		saucer_2.shots[i].y = 0;
	}

	// Putting them saucy saucers inside an array
	struct saucer* the_saucers[2] = { &saucer_1, &saucer_2 };

	struct joystick_angle_pulse joystick_internal;
	float angle_float_goal = 0;
	float angle_float_current = 0;
	struct coord joy_direct;
	struct coord joy_direct_old;
	unsigned int moved = 0;

	char heading_direction;
	struct coord_flt inertia_speed;
	struct coord_flt inertia_speed_final;
	TickType_t inertia_start;

	float angle_x = 0;
	float angle_y = 0;

// 	Initial shape and heading of the player ship
	struct point form_orig[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	direction.x1 = 0;
	direction.y1 = -12;
	direction.x2 = 0;
	direction.y2 = 6;

// 	This variable is changed with every tick by the joystick angle for player ship rotation
	struct point form[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	int16_t incr = 0;
	int16_t incr2 = 0;

//	Fired blaster cannon shots
	// Initializations
	struct shot shots[100] = { { 0 } };
	void initialize_single_shot(int i){
		shots[i].position.x = 0;
		shots[i].position.y = 0;
		shots[i].angle = 0;
		shots[i].status = hide;
	}
	unsigned char timer_1sec = 0;
	unsigned char timer_1halfsec = 0;
	unsigned char timer_2sec = 0;
	uint16_t fps_num;
	char fps_print[1][50];

	while (1) {
// 		Reading desired life count from cheats menu

		if(xQueueReceive(LifeCountQueue3, &life_readin, 0) == pdTRUE){
			life_count += life_readin;
		}

		xQueueReceive(FPSQueue, &fps_num, 0);
		sprintf(fps_print, "FPS: %i", fps_num);

// 		Timer logic
		if ((xTaskGetTickCount() - lastTime_1) >= one_second) {
			timer_1sec = 1;
			lastTime_1 = xTaskGetTickCount();
		}
		else
			timer_1sec = 0;
		if ((xTaskGetTickCount() - lastTime_2) >= one_and_a_half_seconds) {
			timer_1halfsec = 1;
			lastTime_2 = xTaskGetTickCount();
		}
		else
			timer_1halfsec = 0;

		if ((xTaskGetTickCount() - lastTime_3) >= two_seconds) {
			timer_2sec = 1;
			lastTime_3 = xTaskGetTickCount();
		}
		else
			timer_2sec = 0;

//		Starting drawing when screen is ready
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {

//			Button logic, thrust and angle
			restart_lives = life_readin;
			if (life_count != 0) {
				if (buttonCount(BUT_E)) {
					state_pause = !state_pause;
//					xQueueSend(StateQueue, &next_state_signal_pause, 100);
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
			if(state_pause == false){
				if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE) {
					input.angle = joystick_internal.angle;
				}

				//			Joystick input
							joy_direct.x = (int16_t)(ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
							joy_direct.y = (int16_t)(255 - (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4));

				//			Local Player movement
							if((joy_direct.x > 132) || (joy_direct.x < 124) || (joy_direct.y > 132) || (joy_direct.y < 124)){
								moved = 1;
								inertia_start = xTaskGetTickCount();
							}
							else{
								moved = 0;
							}

							if(moved){
								if(player.position.x <= DISPLAY_SIZE_X && player.position.y <= DISPLAY_SIZE_Y){
									if(abs(joy_direct_old.x - joy_direct.x) / SPEED_SCALING_FACTOR <= PLAYER_SPEED_MAX_X){
										player.speed_goal.x = (joy_direct.x - 128) / SPEED_SCALING_FACTOR;
									}
									if(abs(joy_direct_old.y - joy_direct.y) / SPEED_SCALING_FACTOR <= PLAYER_SPEED_MAX_Y){
										player.speed_goal.y = (joy_direct.y - 128) / SPEED_SCALING_FACTOR;
									}
									if(abs(player.speed_current.x - player.speed_goal.x) >= 0.1){
										if(player.speed_current.x > player.speed_goal.x){
											player.speed_current.x -= PLAYER_STANDARD_ACCELERATION_X;
										}
										if(player.speed_current.x < player.speed_goal.x){
											player.speed_current.x += PLAYER_STANDARD_ACCELERATION_X;
										}
									}
									if(abs(player.speed_current.y - player.speed_goal.y) >= 0.1){
										if(player.speed_current.y > player.speed_goal.y){
											player.speed_current.y -= PLAYER_STANDARD_ACCELERATION_Y;
										}
										if(player.speed_current.y < player.speed_goal.y){
											player.speed_current.y += PLAYER_STANDARD_ACCELERATION_Y;
										}
									}
									player.position.x += player.speed_current.x;
									player.position.y += player.speed_current.y;
									if((player.position_old.x - player.position.x) > 0){
										if((player.position_old.y - player.position.y) > 0){
											heading_direction = HEADING_ANGLE_NW;
										}
										else if((player.position_old.y - player.position.y) < 0){
											heading_direction = HEADING_ANGLE_NE;
										}
										else{
											heading_direction = HEADING_ANGLE_N;
										}
									}
									else if((player.position_old.x - player.position.x) < 0){
										if((player.position_old.y - player.position.y) > 0){
											heading_direction = HEADING_ANGLE_SW;
										}
										else if((player.position_old.y - player.position.y) < 0){
											heading_direction = HEADING_ANGLE_SE;
										}
										else{
											heading_direction = HEADING_ANGLE_S;
										}
									}
									else{
										if((player.position_old.y - player.position.y) > 0){
											heading_direction = HEADING_ANGLE_W;
										}
										else if((player.position_old.y - player.position.y) < 0){
											heading_direction = HEADING_ANGLE_E;
										}
									}
									player.position_old.x = player.position.x;
									player.position_old.y = player.position.y;
								}
								inertia_speed.x = abs(player.speed_current.x);
								inertia_speed.y = abs(player.speed_current.y);

								inertia_speed_final.x = INERTIA_MIN_SPEED_X;
								inertia_speed_final.y = INERTIA_MIN_SPEED_Y;
								if((abs(joy_direct_old.x - joy_direct.x) / 16) <= INERTIA_MIN_SPEED_X){
									inertia_speed_final.x += ((abs(joy_direct_old.x - joy_direct.x)) / SPEED_SCALING_FACTOR);
								}
								if((abs(joy_direct_old.y - joy_direct.y) / 16) <= INERTIA_MIN_SPEED_Y){
									inertia_speed_final.y += ((abs(joy_direct_old.y - joy_direct.y)) / SPEED_SCALING_FACTOR);
								}

				//				Getting joystick angle
								angle_x = (float)((int16_t)joy_direct.x-128);
								angle_y = (float)((int16_t)joy_direct.y-128);
								angle_float_goal = 0;
								if (abs(joy_direct.x - 128) > 5 || abs(joy_direct.y - 128) > 5){
									if((angle_x != 0) && (angle_y != 0)){
										if(angle_y != 128){
											angle_float_goal = (CONVERT_TO_DEG * atan2f(angle_y, angle_x)) + 270;
										}
										else{
											angle_float_goal = 0;
										}
									}
									else{
										angle_float_goal = 0;
									}
								}
							}

							if(!moved){
								switch(heading_direction){
									case HEADING_ANGLE_N:
										player.position.x -= inertia_speed.x;
										break;
									case HEADING_ANGLE_S:
										player.position.x += inertia_speed.x;
										break;
									case HEADING_ANGLE_E:
										player.position.y += inertia_speed.y;
										break;
									case HEADING_ANGLE_W:
										player.position.y -= inertia_speed.y;
										break;
									case HEADING_ANGLE_NE:
										player.position.y += inertia_speed.y;
										player.position.x -= inertia_speed.x;
										break;
									case HEADING_ANGLE_NW:
										player.position.y -= inertia_speed.y;
										player.position.x -= inertia_speed.x;
										break;
									case HEADING_ANGLE_SE:
										player.position.y += inertia_speed.y;
										player.position.x += inertia_speed.x;
										break;
									case HEADING_ANGLE_SW:
										player.position.y -= inertia_speed.y;
										player.position.x += inertia_speed.x;
										break;
								}
							}

							if(inertia_speed.x > inertia_speed_final.x){
								if((xTaskGetTickCount() - inertia_start) % INERTIA_TIME_INCREMENT == 0){
									inertia_speed.x -= INERTIA_DECELERATE_X;
								}
							}
							if(inertia_speed.y > inertia_speed_final.y){
								if((xTaskGetTickCount() - inertia_start) % INERTIA_TIME_INCREMENT == 0){
									inertia_speed.y -= INERTIA_DECELERATE_Y;
								}
							}

				//			Make player show up at the other side of the screen when reaching screen border
							if(player.position.x >= DISPLAY_SIZE_X){
								player.position.x = 0;
								player.position.y = rand() % 241;
							}
							else if(player.position.x <= 0){
								player.position.x = DISPLAY_SIZE_X;
								player.position.y = rand() % 241;
							}
							if(player.position.y >= DISPLAY_SIZE_Y){
								player.position.y = 0;
								player.position.x = rand() % 321;
							}
							else if(player.position.y <= 0){
								player.position.y = DISPLAY_SIZE_Y;
								player.position.x = rand() % 321;
							}

				//			Doing actual player ship rotation here
							memcpy(&form, &form_orig, 3 * sizeof(struct point));
							if(abs(angle_float_current - angle_float_goal) >= 2){
								if(abs(angle_float_current - angle_float_goal) < 180){
									if(angle_float_current > angle_float_goal){
										angle_float_current += ROTATION_SPEED;
									}
									if(angle_float_current < angle_float_goal){
										angle_float_current -= ROTATION_SPEED;
									}
								}
								else if(abs(angle_float_current - angle_float_goal) >= 180){
									if(angle_float_current > angle_float_goal){
										angle_float_current -= ROTATION_SPEED;
									}
									if(angle_float_current < angle_float_goal){
										angle_float_current += ROTATION_SPEED;
									}
								}
							}
							for(incr = 0; incr < 3; incr++){
								form[incr].x = form_orig[incr].x * cos(angle_float_current * CONVERT_TO_RAD)
													- form_orig[incr].y * sin(angle_float_current * CONVERT_TO_RAD);
								form[incr].y = form_orig[incr].x * sin(angle_float_current * CONVERT_TO_RAD)
													+ form_orig[incr].y * cos(angle_float_current * CONVERT_TO_RAD);
							}
							memcpy(&joy_direct_old, &joy_direct, sizeof(struct coord)); // Used for joystick coord difference

	//			Handling cannon shot firing
	//			Spawning new cannon shots on player input
				if(buttonCountWithLiftup(BUT_B)){
					shots[input.shots_fired].status = spawn;
					shots[input.shots_fired].position.x = player.position.x;
					shots[input.shots_fired].position.y = player.position.y;
					shots[input.shots_fired].angle = joystick_internal.angle;
					input.shots_fired++;
				}

	//			Making fired shots disappear when reaching the screen edge
				for(incr = 0; incr < input.shots_fired; incr++){
					if(shots[incr].position.x >= DISPLAY_SIZE_X){
						input.shots_fired--;
						for(incr2 = incr; incr2 < input.shots_fired; incr2++){
							memcpy(&shots[incr2], &shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_single_shot(input.shots_fired + 1);
					}
					else if(shots[incr].position.x <= 0){
						input.shots_fired--;
						for(incr2 = incr; incr2 < input.shots_fired; incr2++){
							memcpy(&shots[incr2], &shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_single_shot(input.shots_fired + 1);
					}

					if(shots[incr].position.y >= DISPLAY_SIZE_Y){
						input.shots_fired--;
						for(incr2 = incr; incr2 < input.shots_fired; incr2++){
							memcpy(&shots[incr2], &shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_single_shot(input.shots_fired + 1);
					}
					else if(shots[incr].position.y <= 0){
						input.shots_fired--;
						for(incr2 = incr; incr2 < input.shots_fired; incr2++){
							memcpy(&shots[incr2], &shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_single_shot(input.shots_fired + 1);
					}
				}
	//			Handling movement of fired shots
				for(incr = 0; incr < input.shots_fired; incr++){
					switch(shots[incr].angle){
					case JOYSTICK_ANGLE_N:
						shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_S:
						shots[incr].position.y += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_E:
						shots[incr].position.x += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_W:
						shots[incr].position.x -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_NW:
						shots[incr].position.x -= LASER_BLASTER_SPEED;
						shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_NE:
						shots[incr].position.x += LASER_BLASTER_SPEED;
						shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_SW:
						shots[incr].position.x -= LASER_BLASTER_SPEED;
						shots[incr].position.y += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_SE:
						shots[incr].position.x += LASER_BLASTER_SPEED;
						shots[incr].position.y += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_NULL:
						shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					}
				}

				exeCount++;

	//			Re-spawning asteroids
				if (one_asteroid_hit_small == true) {
					for (i = 0; i <= 15; i+=2) {
						// Only the following i we re-spawn
						if (i == 0 || i == 4 || i == 8 || i == 10 || i == 12 || i == 14) {
							if (all_asteroids[i]->remain_hits == none)
								break;
						}
					}

					if ((i == 0 || i == 4) && asteroids_to_destroy_large > 0) {
						all_asteroids[i]->remain_hits = three;
						all_asteroids[i]->shape = rand() % 3;
						all_asteroids[i]->position.x = all_asteroids[i]->spawn_position.x;
						all_asteroids[i]->position.y = all_asteroids[i]->spawn_position.y;
						all_asteroids[i]->position_locked = false;
					}

					if ((i == 8 || i == 10 || i == 12 || i == 14) && asteroids_to_destroy_medium > 0) {
						all_asteroids[i]->remain_hits = two;
						all_asteroids[i]->shape = rand() % 3;
						all_asteroids[i]->position.x = all_asteroids[i]->spawn_position.x;
						all_asteroids[i]->position.y = all_asteroids[i]->spawn_position.y;
						all_asteroids[i]->position_locked = false;
					}
					one_asteroid_hit_small = false;
				}

				// Re-spawn infinitively if infinite_respawn == true
				if (infinite_respawn == true && (asteroids_to_destroy_large < 2 || asteroids_to_destroy_medium < 2)) {
					asteroids_to_destroy_medium = 99;
					asteroids_to_destroy_large = 99;
				}

				// This creates the seed for all the following rand-Functions
				srand((uint32_t)time_passed);

				/*
				 * ASTEROIDS
				 * The following sets the movement of the asteroids. There are 8 possible directions.
				 */
				for (i = 0; i <= 15; i++) {
					if (all_asteroids[i]->position_locked == false) {
						switch (all_asteroids[i]->orientation) {
						case N:
							all_asteroids[i]->position.y = all_asteroids[i]->position.y	- (1 * ASTEROID_SPEED);
							break;
						case NE:
							all_asteroids[i]->position.x = all_asteroids[i]->position.x	+ (1 * ASTEROID_SPEED);
							all_asteroids[i]->position.y = all_asteroids[i]->position.y	- (1 * ASTEROID_SPEED);
							break;
						case E:
							all_asteroids[i]->position.x = all_asteroids[i]->position.x	+ (1 * ASTEROID_SPEED);
							break;
						case SE:
							all_asteroids[i]->position.x = all_asteroids[i]->position.x	+ (1 * ASTEROID_SPEED);
							all_asteroids[i]->position.y = all_asteroids[i]->position.y	+ (1 * ASTEROID_SPEED);
							break;
						case S:
							all_asteroids[i]->position.y = all_asteroids[i]->position.y	+ (1 * ASTEROID_SPEED);
							break;
						case SW:
							all_asteroids[i]->position.x = all_asteroids[i]->position.x	- (1 * ASTEROID_SPEED);
							all_asteroids[i]->position.y = all_asteroids[i]->position.y	+ (1 * ASTEROID_SPEED);
							break;
						case W:
							all_asteroids[i]->position.x = all_asteroids[i]->position.x	- (1 * ASTEROID_SPEED);
							break;
						case NW:
							all_asteroids[i]->position.x = all_asteroids[i]->position.x	- (1 * ASTEROID_SPEED);
							all_asteroids[i]->position.y = all_asteroids[i]->position.y - (1 * ASTEROID_SPEED);
							break;
						case no_ort: break;
						}
					}
				}

				// CATCH off-screen asteroids
				for (i = 0; i <= 15; i++) {
					switch (all_asteroids[i]->orientation) {
					case no_ort: break;
					case N:	if (all_asteroids[i]->position.y <= -50) {
								setSpawnPosition(all_asteroids[i]->spawn_side, i, rand());
								all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
								} break;
					case NE: if ((all_asteroids[i]->position.x >= 370)
									|| (all_asteroids[i]->position.y <= -50)) {
								setSpawnPosition(all_asteroids[i]->spawn_side, i, rand());
								all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
								} break;
					case E: if (all_asteroids[i]->position.x >= 370) {
								setSpawnPosition(all_asteroids[i]->spawn_side, i, rand());
								all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
								} break;
					case SE: if ((all_asteroids[i]->position.x >= 370)
									|| (all_asteroids[i]->position.y >= 290)) {
								setSpawnPosition(all_asteroids[i]->spawn_side, i, rand());
								all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
								} break;
					case S: if (all_asteroids[i]->position.y >= 290) {
								setSpawnPosition(all_asteroids[i]->spawn_side, i, rand());
								all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
								} break;
					case SW: if ((all_asteroids[i]->position.x <= -50)
									|| (all_asteroids[i]->position.y >= 290)) {
								setSpawnPosition(all_asteroids[i]->spawn_side, i, rand());
								all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
								} break;
					case W: if (all_asteroids[i]->position.x <= -50) {
								setSpawnPosition(all_asteroids[i]->spawn_side, i, rand());
								all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
								} break;
					case NW:if ((all_asteroids[i]->position.x <= -50)
									|| (all_asteroids[i]->position.y <= -50)) {
								setSpawnPosition(all_asteroids[i]->spawn_side, i, rand());
								all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
								} break;
					}
				}

				/*
				 * SAUCERS
				 * The following sets the movement of the saucers. Offset 10 pixel (10 pixel off screen)
				 */
				// SAUCER #1, spawns after 20 sec
				if ((time_passed > SAUCER_1_SPAWN_TIME) && (saucer_1.remain_hits != none)) {
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
								> saucer_routes[saucer_1.route_number][saucer_1.turn_number	- 1]) {
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

				// SACUER #1 fire
				if ((timer_1halfsec == 1) && (time_passed > SAUCER_1_SPAWN_TIME) && (saucer_1.remain_hits != none)) {
					if ((player.position.y - saucer_1.position.y != 0) && (player.position.x - saucer_1.position.x != 0)) {
						saucer_1.ratios[saucer_1.shot_number] = abs((player.position.x - saucer_1.position.x) / (player.position.y - saucer_1.position.y));
					}
					else {
						saucer_1.ratios[saucer_1.shot_number] = 1;
					}

					// Get shot ready
					saucer_1.shots[saucer_1.shot_number].x = saucer_1.position.x;
					saucer_1.shots[saucer_1.shot_number].y = saucer_1.position.y;
					saucer_1.shot_fired[saucer_1.shot_number] = true;

					// Determine in which quarter-direction the shot moves (Up and left, Up and right, Down and left or Down and right)
					if (((player.position.x - saucer_1.position.x) < 0) && ((player.position.y - saucer_1.position.y) < 0))
						saucer_1.shot_direction[saucer_1.shot_number] = up_and_left;
					if (((player.position.x - saucer_1.position.x) > 0) && ((player.position.y - saucer_1.position.y) < 0))
						saucer_1.shot_direction[saucer_1.shot_number] = up_and_right;
					if (((player.position.x - saucer_1.position.x) > 0) && ((player.position.y - saucer_1.position.y) > 0))
						saucer_1.shot_direction[saucer_1.shot_number] = down_and_right;
					if (((player.position.x - saucer_1.position.x) < 0) && ((player.position.y - saucer_1.position.y) > 0))
						saucer_1.shot_direction[saucer_1.shot_number] = down_and_left;

					saucer_1.shot_number = (saucer_1.shot_number + 1) % 10; // Get the next shot ready
				}

				// SAUCER #2, spawns after 35 sec
				if ((time_passed > SAUCER_2_SPAWN_TIME) && (saucer_2.remain_hits != none)) {
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
								> saucer_routes[saucer_2.route_number][saucer_2.turn_number	- 1]) {
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

				// SACUER #2 fire
				if ((timer_2sec == 1) && (time_passed > SAUCER_2_SPAWN_TIME) && (saucer_2.remain_hits != none)) {
					if ((player.position.y - saucer_2.position.y != 0) && (player.position.x - saucer_2.position.x != 0)) {
						saucer_2.ratios[saucer_2.shot_number] = abs((player.position.x - saucer_2.position.x) / (player.position.y - saucer_2.position.y));
					}
					else {
						saucer_2.ratios[saucer_2.shot_number] = 1;
					}

					// Get shot ready
					saucer_2.shots[saucer_2.shot_number].x = saucer_2.position.x;
					saucer_2.shots[saucer_2.shot_number].y = saucer_2.position.y;
					saucer_2.shot_fired[saucer_2.shot_number] = true;

					// Determine in which quarter-direction the shot moves (Up and left, Up and right, Down and left or Down and right)
					if (((player.position.x - saucer_2.position.x) < 0) && ((player.position.y - saucer_2.position.y) < 0))
						saucer_2.shot_direction[saucer_2.shot_number] = up_and_left;
					if (((player.position.x - saucer_2.position.x) > 0) && ((player.position.y - saucer_2.position.y) < 0))
						saucer_2.shot_direction[saucer_2.shot_number] = up_and_right;
					if (((player.position.x - saucer_2.position.x) > 0) && ((player.position.y - saucer_2.position.y) > 0))
						saucer_2.shot_direction[saucer_2.shot_number] = down_and_right;
					if (((player.position.x - saucer_2.position.x) < 0) && ((player.position.y - saucer_2.position.y) > 0))
						saucer_2.shot_direction[saucer_2.shot_number] = down_and_left;

					saucer_2.shot_number = (saucer_2.shot_number + 1) % 10; // Get the next shot ready
				}

				// SAUCER #1 and #2 shots movement
				// ratio is multiplied to x increment only, because ratio = x/y. y increment is always 2.
				for (i = 0; i <= 1; i++) {
					if (the_saucers[i]->remain_hits != none) {
						for (j = 0; j <= 9; j++) {
							if (the_saucers[i]->shot_fired[j] == true) {
								if (the_saucers[i]->ratios[j] > 3.0)
									the_saucers[i]->ratios[j] = 1;
								switch (the_saucers[i]->shot_direction[j]) {
								case up_and_left:
									the_saucers[i]->shots[j].x = the_saucers[i]->shots[j].x
													- the_saucers[i]->ratios[j] * SAUCER_BLASTER_SPEED;
									the_saucers[i]->shots[j].y = the_saucers[i]->shots[j].y - SAUCER_BLASTER_SPEED;
									break;
								case up_and_right:
									the_saucers[i]->shots[j].x = the_saucers[i]->shots[j].x
													+ the_saucers[i]->ratios[j] * SAUCER_BLASTER_SPEED;
									the_saucers[i]->shots[j].y = the_saucers[i]->shots[j].y - SAUCER_BLASTER_SPEED;
									break;
								case down_and_right:
									the_saucers[i]->shots[j].x = the_saucers[i]->shots[j].x
													+ the_saucers[i]->ratios[j] * SAUCER_BLASTER_SPEED;
									the_saucers[i]->shots[j].y = the_saucers[i]->shots[j].y + SAUCER_BLASTER_SPEED;
									break;
								case down_and_left:
									the_saucers[i]->shots[j].x = the_saucers[i]->shots[j].x
													- the_saucers[i]->ratios[j] * SAUCER_BLASTER_SPEED;
									the_saucers[i]->shots[j].y = the_saucers[i]->shots[j].y + SAUCER_BLASTER_SPEED;
									break;
								}
							}
						}
					}
				}

				// SACUER #1 and #2 catch shots which have moved off the screen; offset: 5 pxl
				for (i = 0; i <= 1; i++) {
					if (the_saucers[i]->remain_hits != none) {
						for (j = 0; j <= 9; j++) {
							if ((the_saucers[i]->shots[j].x > 325) || (the_saucers[i]->shots[j].x < -5)) {
								the_saucers[i]->shots[j].x = 0;
								the_saucers[i]->shots[j].y = 0;
								the_saucers[i]->shot_fired[j] = false;
							}
							if ((the_saucers[i]->shots[j].y > 245) || (the_saucers[i]->shots[j].y < -5)) {
								the_saucers[i]->shots[j].x = 0;
								the_saucers[i]->shots[j].y = 0;
								the_saucers[i]->shot_fired[j] = false;
							}
						}
					}
				}



				/* Check if the player's ship was hit by asteroid */
				for (i = 0; i <= 15; i++) {
					if(all_asteroids[i]->remain_hits == one){
						if ((abs(all_asteroids[i]->position.x - player.position.x) <= HIT_LIMIT_SMALL)
						&& (abs(all_asteroids[i]->position.y - player.position.y) <= HIT_LIMIT_SMALL)) {
							player.state = hit;
							hit_timestamp = xTaskGetTickCount();
						}
					}
					if(all_asteroids[i]->remain_hits == two){
						if ((abs(all_asteroids[i]->position.x - player.position.x) <= HIT_LIMIT_MEDIUM)
						&& (abs(all_asteroids[i]->position.y - player.position.y) <= HIT_LIMIT_MEDIUM)) {
							player.state = hit;
							hit_timestamp = xTaskGetTickCount();
						}
					}
					if(all_asteroids[i]->remain_hits == three){
						if ((abs(all_asteroids[i]->position.x - player.position.x) <= HIT_LIMIT_LARGE)
						&& (abs(all_asteroids[i]->position.y - player.position.y) <= HIT_LIMIT_LARGE)) {
							player.state = hit;
							hit_timestamp = xTaskGetTickCount();
						}
					}
				}

				/* Check if asteroids were hit by shot cannon blaster laser thigs
				 * Threshold zone is a square around the asteroid center.
				 * Square side length: small: 6px; medium: 8px; large: 12px
				 */
				// asteroid_2, _4, _6, _8, _10, _12, _14 and asteroid_16 can only be small
				for(incr = 0; incr < input.shots_fired; incr++){
					for (i = 1; i <= 15; i+=2){
						if ((abs(all_asteroids[i]->position.x - shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
							&& (abs(all_asteroids[i]->position.y - shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)
							&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)
							&& (all_asteroids[i]->remain_hits == one)) {
								all_asteroids[i]->position.x = -50;
								all_asteroids[i]->position.y = -50;
								all_asteroids[i]->position_locked = true;
								all_asteroids[i]->remain_hits = none;
								score += POINTS_ASTEROID_SMALL;
								asteroids_to_destroy_small--;
								one_asteroid_hit_small = true;
						}
					}
				}

				// asteroid_3, asteroid_7, asteroid_9, asteroid_11, asteroid_13 and asteroid_15 can be medium or small
				for(incr = 0; incr < input.shots_fired; incr++) {
					for (i = 2; i <= 15; i+=2) {
						if (i != 4) {
							if ((abs(all_asteroids[i]->position.x - shots[incr].position.x) <= HIT_LIMIT_SHOT_MEDIUM)
									&& (abs(all_asteroids[i]->position.y - shots[incr].position.y) <= HIT_LIMIT_SHOT_MEDIUM)
									&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)
									&& (all_asteroids[i]->remain_hits == two)) {
								asteroids_to_destroy_medium--;
								score += POINTS_ASTEROID_MEDIUM;
								all_asteroids[i]->hit_timestamp = xTaskGetTickCount();
								all_asteroids[i+1]->hit_timestamp = xTaskGetTickCount();

								switch (i) {
								case 0: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = NW;
										all_asteroids[i+1]->orientation = SE; break;
								case 2: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = W;
										all_asteroids[i+1]->orientation = E; break;
								case 4: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = N;
										all_asteroids[i+1]->orientation = S; break;
								case 6: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = NE;
										all_asteroids[i+1]->orientation = SW; break;
								case 8: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = E;
										all_asteroids[i+1]->orientation = W; break;
								case 10: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = NE;
										all_asteroids[i+1]->orientation = SW; break;
								case 12: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = W;
										all_asteroids[i+1]->orientation = E; break;
								case 14: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = NW;
										all_asteroids[i+1]->orientation = SE; break;
								}
							}
							else if ((abs(all_asteroids[i]->position.x - shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
									&& (abs(all_asteroids[i]->position.y - shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)
									&& (all_asteroids[i]->remain_hits == one)
									&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)) {
								all_asteroids[i]->position.x = -50;
								all_asteroids[i]->position.y = -50;
								all_asteroids[i]->position_locked = true;
								all_asteroids[i]->remain_hits = none;
								score += POINTS_ASTEROID_SMALL;
								asteroids_to_destroy_small--;
								one_asteroid_hit_small = true;
							}
						}
					}
				}

				// asteroid_1 and asteroid_5 can be all sizes: S, M and L
				for(incr = 0; incr < input.shots_fired; incr++) {
					for (i = 0; i <= 7; i+=4) {
						if ((abs(all_asteroids[i]->position.x - shots[incr].position.x) <= HIT_LIMIT_SHOT_LARGE)
								&& (abs(all_asteroids[i]->position.y - shots[incr].position.y) <= HIT_LIMIT_SHOT_LARGE)
								&& (all_asteroids[i]->remain_hits == three)) {
							asteroids_to_destroy_large--;
							score += POINTS_ASTEROID_LARGE;
							all_asteroids[i]->hit_timestamp = xTaskGetTickCount();
							all_asteroids[i+2]->hit_timestamp = xTaskGetTickCount();


							switch (i) {
							case 0: all_asteroids[i]->remain_hits = two;
									all_asteroids[i+2]->remain_hits = two;
									all_asteroids[i+2]->position_locked = false;
									all_asteroids[i+2]->position.x = all_asteroids[i]->position.x;
									all_asteroids[i+2]->position.y = all_asteroids[i]->position.y;
									all_asteroids[i]->orientation = NW;
									all_asteroids[i+2]->orientation = SE; break;
							case 4: all_asteroids[i]->remain_hits = two;
									all_asteroids[i+2]->remain_hits = two;
									all_asteroids[i+2]->position_locked = false;
									all_asteroids[i+2]->position.x = all_asteroids[i]->position.x;
									all_asteroids[i+2]->position.y = all_asteroids[i]->position.y;
									all_asteroids[i]->orientation = N;
									all_asteroids[i+2]->orientation = S; break;
							}
						}
						else if ((abs(all_asteroids[i]->position.x - shots[incr].position.x) <= HIT_LIMIT_SHOT_MEDIUM)
									&& (abs(all_asteroids[i]->position.y - shots[incr].position.y) <= HIT_LIMIT_SHOT_MEDIUM)
									&& (all_asteroids[i]->remain_hits == two)
									&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)) {
								asteroids_to_destroy_medium--;
								score += POINTS_ASTEROID_MEDIUM;
								all_asteroids[i]->hit_timestamp = xTaskGetTickCount();
								all_asteroids[i+1]->hit_timestamp = xTaskGetTickCount();

								switch (i) {
								case 0: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = NW;
										all_asteroids[i+1]->orientation = SE; break;
								case 2: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = W;
										all_asteroids[i+1]->orientation = E; break;
								case 4: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = N;
										all_asteroids[i+1]->orientation = S; break;
								case 6: all_asteroids[i]->remain_hits = one;
										all_asteroids[i+1]->remain_hits = one;
										all_asteroids[i+1]->position_locked = false;
										all_asteroids[i+1]->position.x = all_asteroids[i]->position.x;
										all_asteroids[i+1]->position.y = all_asteroids[i]->position.y;
										all_asteroids[i]->orientation = NE;
										all_asteroids[i+1]->orientation = SW; break;
								}
						}
						else if ((abs(all_asteroids[i]->position.x - shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
								&& (abs(all_asteroids[i]->position.y - shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)
								&& (all_asteroids[i]->remain_hits == one)
								&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)) {
							all_asteroids[i]->position.x = -50;
							all_asteroids[i]->position.y = -50;
							all_asteroids[i]->position_locked = true;
							all_asteroids[i]->remain_hits = none;
							score += POINTS_ASTEROID_SMALL;
							asteroids_to_destroy_small--;
							one_asteroid_hit_small = true;
						}
					}
				}

				/* Check if player was hit by saucer fire
				 * Threshold zone is a square around the players ship center with 6px side length
				 */
				for (i = 0; i <= 1; i++) {
					for (j = 0; j <= 9; j++) {
						if ((abs(the_saucers[i]->shots[j].x - player.position.x) <= HIT_LIMIT_SHOT_SMALL)
								&& (abs(the_saucers[i]->shots[j].y - player.position.y) <= HIT_LIMIT_SHOT_SMALL)) {
							player.state = hit;
							hit_timestamp = xTaskGetTickCount();
						}
					}
				}

				/* Check if saucer was hit by players laser blaster
				 * Threshold zone is a square around the saucer center with 5px side length
				 */
				for (incr = 0; incr < input.shots_fired; incr++) {
					for (i = 0; i <= 1; i++) {
						if ((abs( the_saucers[i]->position.x - shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
								&& (abs(the_saucers[i]->position.y - shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)) {
							if (xTaskGetTickCount() - hit_saucer_timestamp > delay_hit) {
								the_saucers[i]->remain_hits = the_saucers[i]->remain_hits - 1;
								hit_saucer_timestamp = xTaskGetTickCount();
								if (the_saucers[i]->remain_hits == none) {
									score += POINTS_SAUCER_HIT;
									the_saucers[i]->position.x = -15;
									the_saucers[i]->position.y = -15;
									for (j = 0; j <= 9; j++) {
										the_saucers[i]->shots[j].x = -5;
										the_saucers[i]->shots[j].y = -5;
									}
								}
							}
						}
					}
				}

	// 			Drawing functions
				gdispClear(Black);

	//			Debug print line for angle and thrust
				if(SHOW_DEBUG_LVL_3){
					sprintf(str, "Angle: %d | Thrust: %d | 360: %d", input.angle, input.thrust, (uint16_t)(angle_float_goal));
					gdispDrawString(0, 230, str, font1, White);
				}

	//			Drawing the fired canon shots
				for(incr = 0; incr < input.shots_fired; incr++){
					if(shots[incr].status == spawn){
						gdispFillCircle(shots[incr].position.x, shots[incr].position.y, 3, Yellow);
					}
				}

	// 			Drawing the player's ship and asteroids
				if (life_count != 0) {
					// Simple clock at top of screen
					if (timer_1sec == 1)
						time_passed++;
					sprintf(str2, "%d sec", time_passed);
					gdispDrawString(DISPLAY_CENTER_X - 5, 10, str2, font1, White);

					// Score board
					sprintf(str, "Score: %i", score);
					gdispDrawString(5, 10, str, font1, White);

					// Life count
					if(score >= GET_MORE_LIVES_LEVEL_THREE && no_extra_life == true){
						life_count++;
						no_extra_life = false;
						new_life_timer = xTaskGetTickCount();
					}
					if((xTaskGetTickCount() - new_life_timer < 1000) && no_extra_life == false){
						sprintf(str, "EARNED NEW LIFE!"); // Generate game over message
						gdispFillArea(TEXT_X(str) - 10, 25, 120, 10, Green); // White border
						gdispDrawString(TEXT_X(str), 25, str, font1, Black);
					}
					sprintf(str, "Lives: %d", life_count);
					gdispDrawString(260, 10, str, font1, White);

					sprintf(str2, "Level 3");
					gdispDrawString(5, 230, str2, font1, Green);

					// Player ship
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
					if (asteroid_1.remain_hits == three)
						gdispDrawPoly(asteroid_1.position.x, asteroid_1.position.y,
							shapes_large[asteroid_1.shape], NUM_POINTS_LARGE, White);
					else if (asteroid_1.remain_hits == two)
						gdispDrawPoly(asteroid_1.position.x, asteroid_1.position.y,
							shapes_medium[asteroid_1.shape], NUM_POINTS_MEDIUM, White);
					else if (asteroid_1.remain_hits == one)
						gdispDrawPoly(asteroid_1.position.x, asteroid_1.position.y,
							shapes_small[asteroid_1.shape], NUM_POINTS_SMALL, White);

					// Asteroid 2
					if (asteroid_2.remain_hits == one)
						gdispDrawPoly(asteroid_2.position.x, asteroid_2.position.y,
								shapes_small[asteroid_2.shape], NUM_POINTS_SMALL, White);

					// Asteroid 3
					if (asteroid_3.remain_hits == two)
						gdispDrawPoly(asteroid_3.position.x, asteroid_3.position.y,
							shapes_medium[asteroid_3.shape], NUM_POINTS_MEDIUM, White);
					else if (asteroid_3.remain_hits == one)
						gdispDrawPoly(asteroid_3.position.x, asteroid_3.position.y,
							shapes_small[asteroid_3.shape], NUM_POINTS_SMALL, White);

					// Asteroid 4
					if (asteroid_4.remain_hits == one)
						gdispDrawPoly(asteroid_4.position.x, asteroid_4.position.y,
							shapes_small[asteroid_4.shape], NUM_POINTS_SMALL, White);

					// Asteroid 5
					if (asteroid_5.remain_hits == three)
						gdispDrawPoly(asteroid_5.position.x, asteroid_5.position.y,
							shapes_large[asteroid_5.shape], NUM_POINTS_LARGE, White);
					else if (asteroid_5.remain_hits == two)
						gdispDrawPoly(asteroid_5.position.x, asteroid_5.position.y,
							shapes_medium[asteroid_5.shape], NUM_POINTS_MEDIUM, White);
					else if (asteroid_5.remain_hits == one)
						gdispDrawPoly(asteroid_5.position.x, asteroid_5.position.y,
							shapes_small[asteroid_5.shape], NUM_POINTS_SMALL, White);

					// Asteroid 6
					if (asteroid_6.remain_hits == one)
						gdispDrawPoly(asteroid_6.position.x, asteroid_6.position.y,
							shapes_small[asteroid_6.shape], NUM_POINTS_SMALL, White);

					// Asteroid 7
					if (asteroid_7.remain_hits == two)
						gdispDrawPoly(asteroid_7.position.x, asteroid_7.position.y,
							shapes_medium[asteroid_7.shape], NUM_POINTS_MEDIUM, White);
					else if (asteroid_7.remain_hits == one)
						gdispDrawPoly(asteroid_7.position.x, asteroid_7.position.y,
							shapes_small[asteroid_7.shape], NUM_POINTS_SMALL, White);

					// Asteroid 8
					if (asteroid_8.remain_hits == one)
						gdispDrawPoly(asteroid_8.position.x, asteroid_8.position.y,
							shapes_small[asteroid_8.shape], NUM_POINTS_SMALL, White);

					// Asteroid 9
					if (asteroid_9.remain_hits == two)
						gdispDrawPoly(asteroid_9.position.x, asteroid_9.position.y,
							shapes_medium[asteroid_9.shape], NUM_POINTS_MEDIUM, White);
					else if (asteroid_9.remain_hits == one)
						gdispDrawPoly(asteroid_9.position.x, asteroid_9.position.y,
							shapes_small[asteroid_9.shape], NUM_POINTS_SMALL, White);

					// Asteroid 10
					if (asteroid_10.remain_hits == one)
						gdispDrawPoly(asteroid_10.position.x, asteroid_10.position.y,
							shapes_small[asteroid_10.shape], NUM_POINTS_SMALL, White);

					// Asteroid 11
					if (asteroid_11.remain_hits == two)
						gdispDrawPoly(asteroid_11.position.x, asteroid_11.position.y,
							shapes_medium[asteroid_11.shape], NUM_POINTS_MEDIUM, White);
					else if (asteroid_11.remain_hits == one)
						gdispDrawPoly(asteroid_11.position.x, asteroid_11.position.y,
							shapes_small[asteroid_11.shape], NUM_POINTS_SMALL, White);

					// Asteroid 12
					if (asteroid_12.remain_hits == one)
						gdispDrawPoly(asteroid_12.position.x, asteroid_12.position.y,
								shapes_small[asteroid_12.shape], NUM_POINTS_SMALL, White);

					// Asteroid 13
					if (asteroid_13.remain_hits == two)
						gdispDrawPoly(asteroid_13.position.x, asteroid_13.position.y,
							shapes_medium[asteroid_13.shape], NUM_POINTS_MEDIUM, White);
					else if (asteroid_13.remain_hits == one)
						gdispDrawPoly(asteroid_13.position.x, asteroid_13.position.y,
							shapes_small[asteroid_13.shape], NUM_POINTS_SMALL, White);

					// Asteroid 14
					if (asteroid_14.remain_hits == one)
						gdispDrawPoly(asteroid_14.position.x, asteroid_14.position.y,
							shapes_small[asteroid_14.shape], NUM_POINTS_SMALL, White);

					// Asteroid 15
					if (asteroid_15.remain_hits == two)
						gdispDrawPoly(asteroid_15.position.x, asteroid_15.position.y,
							shapes_medium[asteroid_15.shape], NUM_POINTS_MEDIUM, White);
					else if (asteroid_15.remain_hits == one)
						gdispDrawPoly(asteroid_15.position.x, asteroid_15.position.y,
							shapes_small[asteroid_15.shape], NUM_POINTS_SMALL, White);

					// Asteroid 16
					if (asteroid_16.remain_hits == one)
						gdispDrawPoly(asteroid_16.position.x, asteroid_16.position.y,
							shapes_small[asteroid_16.shape], NUM_POINTS_SMALL, White);

					// Saucer 1
					gdispDrawPoly(saucer_1.position.x, saucer_1.position.y,
							saucer_shape, NUM_POINTS_SAUCER, Lime);

					for (i = 0; i <= 9; i++) {
						if (saucer_1.shot_fired[i] == true) {
							if ((player.position.y - saucer_1.position.y != 0)
									&& (player.position.x - saucer_1.position.x != 0))
								gdispFillCircle((int )saucer_1.shots[i].x, (int )saucer_1.shots[i].y, 3, Red);
						}
					}

					if ((time_passed >= SAUCER_1_SPAWN_TIME) && (saucer_1.remain_hits != none)) {
						sprintf(str, "Lives: %d", saucer_1.remain_hits);
						gdispDrawString(DISPLAY_CENTER_X - 70, 10, str, font1, Lime);
					}

					// Saucer 2
					gdispDrawPoly(saucer_2.position.x, saucer_2.position.y,
							saucer_shape, NUM_POINTS_SAUCER, Cyan);

					for (i = 0; i <= 9; i++) {
						if (saucer_2.shot_fired[i] == true) {
							if ((player.position.y - saucer_2.position.y != 0)
									&& (player.position.x - saucer_2.position.x != 0))
								gdispFillCircle((int )saucer_2.shots[i].x, (int )saucer_2.shots[i].y, 3, Red);
						}
					}

					if ((time_passed >= SAUCER_2_SPAWN_TIME) && (saucer_2.remain_hits != none)) {
						sprintf(str2, "Lives: %d", saucer_2.remain_hits);
						gdispDrawString(DISPLAY_CENTER_X + 42, 10, str2, font1, Cyan);
					}


				}

				// GAME OVER
				else if (life_count == 0) {
					gdispFillArea(70, DISPLAY_CENTER_Y - 2, 180, 15, White); // White border
					sprintf(str, "GAME OVER. Press D to continue."); // Generate game over message
					gdispDrawString(TEXT_X(str), DISPLAY_CENTER_Y, str, font1, Black);
					if (buttonCount(BUT_D)) { // Move into highscores menu when user presses D
						xQueueSend(HighScoresQueue, &score, 0);
						goto gamestart3;
					}
				}

				// TRANSITION TO GAME WON STATE when user presses D, basically generate asteroids indefinitely until player dies
				if (score >= BEAT_GAME_SCORE_THRESHOLD && infinite_respawn == false) {
					gdispFillArea(35, DISPLAY_CENTER_Y - 2, 245, 15, White); // White border
					sprintf(str, "YOU WIN. Press D TO BECOME A LEGEND."); // Generate game over message
					gdispDrawString(TEXT_X(str), DISPLAY_CENTER_Y, str, font1, Black);
					if (buttonCount(BUT_D)) {
						infinite_respawn = true;
						// xQueueSend(StateQueue, &next_state_signal_mainmenu, 100);
					}
				}
				memcpy(&joy_direct_old, &joy_direct, sizeof(struct coord));
			}
			else if(state_pause == true){
				gdispClear(Black);
				sprintf(user_help, "> GAME PAUSED. E to resume. <");
				gdispFillArea(75, DISPLAY_CENTER_Y + 20, 175, 10, Yellow);
				gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);

				sprintf(user_help, "> D to quit. <");
				gdispFillArea(75, DISPLAY_CENTER_Y + 30, 175, 10, Yellow);
				gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 30, user_help[0], font1, Black);

				if(buttonCount(BUT_D)){
					xQueueSend(HighScoresQueue, &score, 0);
					goto gamestart3;
				}
			}
			gdispDrawString(270, 230, fps_print[0], font1, White);
		} // Block until screen is ready
	} // While-loop
} // Task
