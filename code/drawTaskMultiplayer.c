/*
 * drawTaskMultiplayer.c
 *
 *  Created on: Dec 18, 2019
 *      Author: Teodor Fratiloiu
 *      		Frederik Zumegen
 */
#include "includes.h"
#include "drawTaskMultiplayer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "drawTaskHighScore.h"

extern QueueHandle_t StateQueue;
extern QueueHandle_t JoystickQueue;
extern QueueHandle_t StartingScoreQueue;
extern QueueHandle_t LifeCountQueue1;
extern QueueHandle_t FPSQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;
extern QueueHandle_t ESPL_RxQueue;
extern QueueHandle_t HighScoresQueueMP;
extern QueueHandle_t LocalMasterQueue;

#define NUM_POINTS_SMALL (sizeof(type_1)/sizeof(type_1[0]))
#define NUM_POINTS_MEDIUM (sizeof(type_4)/sizeof(type_4[0]))
#define NUM_POINTS_LARGE (sizeof(type_7)/sizeof(type_7[0]))

#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

void drawTaskMultiplayer (void * params){
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	boolean executed = false;
	start:
	if(executed == true){
		xQueueSend(StateQueue, &next_state_signal_menu, 100);
	}
	executed = true;
	boolean is_master = false;
	boolean remote_is_master = false;
	struct highscore score;
	score.score = 0;
	score.score_remote = 0;
	int16_t score_from_main_menu = 0;
	xQueueReceive(StartingScoreQueue, &score_from_main_menu, 0);
	score.score = score_from_main_menu;
	score.score_remote = score_from_main_menu;
	char user_help[1][70];
	struct joystick_angle_pulse joystick_internal;
	boolean show_debug = false;
	boolean first_check = false;
	TickType_t double_toggle_delay = 2000;
	TickType_t check_time = xTaskGetTickCount();
	char str2[1][30] = { {0} };
	boolean fired_bullet_this_frame = false;

//	Movement
	struct coord joy_direct;
	struct coord joy_direct_old;
	struct players_ship player_local;
	player_local.position.x = DISPLAY_CENTER_X;
	player_local.position.y = DISPLAY_CENTER_Y;
	player_local.position_old.x = player_local.position.x;
	player_local.position_old.y = player_local.position.y;
	int local_x = (int) player_local.position.x;
	int local_y = (int) player_local.position.y;
//	Will draw the position of local ship from previous frame
	int local_x_old = local_x;
	int local_y_old = local_y;
//	Will use low poly version of local ship for collision detections
	int local_x_lowpoly = local_x_old / 4;
	local_x_lowpoly = local_x_lowpoly * 4;
	int local_y_lowpoly = local_y_old / 3;
	local_y_lowpoly = local_y_lowpoly * 3;

	int remote_x = DISPLAY_CENTER_X;
	int remote_y = DISPLAY_CENTER_Y;

	int remote_bullet_dir_x = JOYSTICK_ANGLE_NULL;
	int remote_bullet_dir_y = JOYSTICK_ANGLE_NULL;
	int remote_bullet_dir_total = JOYSTICK_ANGLE_NULL;
	int remote_bullet_dir_total_old = JOYSTICK_ANGLE_NULL;
	player_status remote_player_state = fine;
	player_local.state = fine;
	int incr;
	int incr2;
	int i = 0;
	float angle_float_goal = 0;
	float angle_float_current = 0;
	float angle_x = 0;
	float angle_y = 0;
	unsigned int moved = 0;
	char heading_direction = HEADING_ANGLE_NULL;
	struct coord_flt inertia_speed;
	struct coord_flt inertia_speed_final;
	TickType_t inertia_start;

//	UART
/*
 * The way UART is implemented:
 * We send one byte per frame, namely a number between 000 ... 255.
 * 000 is reserved for lost connection.
 * For x coordinate of player and x direction of shooting: values between 001 ... 240
 * For y coordinate of player and y direction of shooting: values between 001 ... 240
 * For Pause: 251
 * For one side quitting the game session: 252
 * Syncing bytes (253 and 254) are explained below.
 */

	const char pause_byte = 251;
	const char quit_byte = 252;
	const char sync_byte_1 = 253; // used in main menu for master/slave sync
	const char sync_byte_2 = 254; // used in main menu for master/slave sync
	const char sync_byte_3 = 255; // ack to remote to tell is slave

	char uart_input = 0;
	uint8_t to_send_x = ((int) player_local.position.x) / 4 + 1;
	uint8_t to_send_y = 100 + ((int) player_local.position.y) / 3;

	boolean uart_connected = false;
	boolean state_pause_local = false;
	boolean state_pause_remote = false;
	boolean state_quit_remote = false;
	boolean ready_to_start = false;
	int last_received = 0;

	int lives[2];
	lives[0] = STARTING_LIVES_MULTIPLAYER; // local
	lives[1] = STARTING_LIVES_MULTIPLAYER; // remote
	unsigned int life_readin = STARTING_LIVES_MULTIPLAYER;	// Will be filled with queue readin if received

//	Bullets
	int number_local_shots = 0;
	struct shot local_shots[50];
	void initialize_single_shot(int i){
		local_shots[i].position.x = 0;
		local_shots[i].position.y = 0;
		local_shots[i].angle =  JOYSTICK_ANGLE_NULL;
		local_shots[i].status = hide;
	}

	int number_remote_shots = 0;
	struct shot remote_shots[50];
	void initialize_remote_shot(int i){
		remote_shots[i].position.x = 0;
		remote_shots[i].position.y = 0;
		remote_shots[i].angle =  JOYSTICK_ANGLE_NULL;
		remote_shots[i].status = hide;
	}

	struct point form_orig[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	struct point form[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	const point saucer_shape[] = { { -10, 3 }, { -6, 6 }, { 6, 6 }, { 10, 3 }, { -10, 3 },
			{ -6, 0 }, { 6, 0 }, { 10, 3 }, { 6, 0 }, { 4, -5 }, { -4, -5 },
			{ -6, 0 } };

	// ASTEROIDS
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

	/* This the only random value generated.
		 * This is used to only need to send 1 variable via UART.
		 */
		srand(xTaskGetTickCount());
		int16_t super_random = rand() % 241;
		boolean infinite_respawn = true;
		TickType_t hit_timestamp;
		TickType_t hit_timestamp_2;
		boolean one_asteroid_hit_small = false;
		boolean one_asteroid_hit_medium = false;
		boolean one_asteroid_hit_large = false;
		const TickType_t shot_delay = 1000;
		int time_passed = 0; // Simple clock
		unsigned char timer_1sec = 0;
		TickType_t lastTime_1 = xTaskGetTickCount();
		const TickType_t one_second = 1000 / portTICK_PERIOD_MS;
		boolean life_count_lock = false;
		boolean life_count_lock_2 = false;
		const TickType_t delay_hit = 1000;

		// All asteroid shapes in one array: From this array we later choose which shape to draw
		const point* shapes_small[3] = {type_1, type_2, type_3};
		const point* shapes_medium[3] = {type_4, type_5, type_6};
		const point* shapes_large[3] = {type_7, type_8, type_9};
		const point** shapes_all[3] = {shapes_small, shapes_medium, shapes_large};

		//	Number of asteroids to be destroyed
	int asteroids_to_destroy_small = TO_DESTROY_MULTI_SMALL;
	int asteroids_to_destroy_medium = TO_DESTROY_MULTI_MEDIUM;
	int asteroids_to_destroy_large = TO_DESTROY_MULTI_LARGE;

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
	void setSpawnPosition(sides spawn_side, int i, uint8_t neun_und_sechzig) {
		switch (spawn_side) {
		case left: 	all_asteroids[i]->position.x = all_asteroids[i]->spawn_position.x;
					all_asteroids[i]->position.y = ((int)(all_asteroids[i]->position.y + neun_und_sechzig) % 200) + 20; break;
		case right:	all_asteroids[i]->position.x = all_asteroids[i]->spawn_position.x;
					all_asteroids[i]->position.y = ((int)(all_asteroids[i]->position.y + neun_und_sechzig) % 200) + 20; break;
		case up:	all_asteroids[i]->position.x = ((int)(all_asteroids[i]->position.x + neun_und_sechzig) % 280) + 20;
					all_asteroids[i]->position.y = all_asteroids[i]->spawn_position.y; break;
		case down:	all_asteroids[i]->position.x = ((int)(all_asteroids[i]->position.x + neun_und_sechzig) % 280) + 20;
					all_asteroids[i]->position.y = all_asteroids[i]->spawn_position.y; break;
		}
	}
	boolean executed_once = false;
	uint16_t fps_num;
	char fps_print[1][50];

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block drawing until screen is ready
			if(xQueueReceive(LifeCountQueue1, &life_readin, 0) ==pdTRUE){
				// adding this as extra lives
				lives[0] += life_readin;
				lives[1] += life_readin;
			}
			xQueueReceive(FPSQueue, &fps_num, 0);
			sprintf(fps_print, "FPS: %i", fps_num);

			xQueueReceive(JoystickQueue, &joystick_internal, 0);
			xQueueReceive(ESPL_RxQueue, &uart_input, 0);
			xQueueReceive(LocalMasterQueue, &is_master, 0);
			if(executed_once == true){
				if(is_master == true){
					remote_is_master = false;
				}
				else if(is_master == false){
					remote_is_master = true;
				}
				if(is_master == true){
					last_received = 1;
				}
			}


			if(uart_input != 0){
				uart_connected = true;
			}
			else if(uart_input == 0){
				uart_connected = false;
			}

			if(uart_input == pause_byte){
				state_pause_remote = true;
			}
			else if(uart_input != pause_byte){
				state_pause_remote = false;
			}

			if(uart_input == quit_byte){
				state_quit_remote = true;
			}
			if(uart_connected == true){
				if(uart_input != sync_byte_1 && uart_input != sync_byte_2){
					ready_to_start = true;
				}
			}

// 			Toggle to show debug content and UART Input
			if(first_check == false){
				if(buttonCount(BUT_C)){
					check_time = xTaskGetTickCount();
					first_check = true;
				}
			}
			if(first_check == true){
				if(xTaskGetTickCount() - check_time <= double_toggle_delay){
					if(buttonCount(BUT_C)){
						first_check = false;
						show_debug = !show_debug;
					}
				}
			}

//			Only runs if UART is connected and other player is also on this screen, game not paused and remote hasn't quit
			if(uart_connected == true && state_pause_local == false && state_pause_remote == false &&
					state_quit_remote == false && ready_to_start == true){
				if(!last_received){
					if(uart_input >= 1 && uart_input <= 80){
						remote_y = (uart_input - 1) * 3;
						remote_bullet_dir_y = JOYSTICK_ANGLE_N;
					}
					else if(uart_input >= 81 && uart_input <= 160){
						remote_y = (uart_input - 81) * 3;
						remote_bullet_dir_y =  JOYSTICK_ANGLE_NULL;
					}
					else if(uart_input >= 161 && uart_input <= 240){
						remote_y = (uart_input - 161) * 3;
						remote_bullet_dir_y =  JOYSTICK_ANGLE_S;
					}
				}
				else if(last_received){
					if(uart_input >= 1 && uart_input <= 80){
						remote_x = (uart_input - 1) * 4;
						remote_bullet_dir_x =  JOYSTICK_ANGLE_W;
					}
					else if(uart_input >= 81 && uart_input <= 160){
						remote_x = (uart_input - 81) * 4;
						remote_bullet_dir_x = JOYSTICK_ANGLE_NULL;
					}
					else if(uart_input >= 161 && uart_input <= 240){
						remote_x = (uart_input - 161) * 4;
						remote_bullet_dir_x = JOYSTICK_ANGLE_E;
					}
				}
				if(remote_bullet_dir_x == JOYSTICK_ANGLE_NULL){
					if(remote_bullet_dir_y == JOYSTICK_ANGLE_N){
						remote_bullet_dir_total = JOYSTICK_ANGLE_N;
					}
					else if(remote_bullet_dir_y == JOYSTICK_ANGLE_NULL){
						remote_bullet_dir_total = JOYSTICK_ANGLE_NULL;
					}
					else if(remote_bullet_dir_y == JOYSTICK_ANGLE_S){
						remote_bullet_dir_total = JOYSTICK_ANGLE_S;
					}
				}
				else if(remote_bullet_dir_x == JOYSTICK_ANGLE_W){
					if(remote_bullet_dir_y == JOYSTICK_ANGLE_N){
						remote_bullet_dir_total = JOYSTICK_ANGLE_NW;
					}
					else if(remote_bullet_dir_y == JOYSTICK_ANGLE_NULL){
						remote_bullet_dir_total = JOYSTICK_ANGLE_W;
					}
					else if(remote_bullet_dir_y == JOYSTICK_ANGLE_S){
						remote_bullet_dir_total = JOYSTICK_ANGLE_SW;
					}
				}
				else if(remote_bullet_dir_x == JOYSTICK_ANGLE_E){
					if(remote_bullet_dir_y == JOYSTICK_ANGLE_N){
						remote_bullet_dir_total = JOYSTICK_ANGLE_NE;
					}
					else if(remote_bullet_dir_y == JOYSTICK_ANGLE_NULL){
						remote_bullet_dir_total = JOYSTICK_ANGLE_E;
					}
					else if(remote_bullet_dir_y == JOYSTICK_ANGLE_S){
						remote_bullet_dir_total = JOYSTICK_ANGLE_SE;
					}
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
					if(player_local.position.x <= DISPLAY_SIZE_X && player_local.position.y <= DISPLAY_SIZE_Y){
						if(abs(joy_direct_old.x - joy_direct.x) / SPEED_SCALING_FACTOR <= PLAYER_SPEED_MAX_X){
							player_local.speed_goal.x = (joy_direct.x - 128) / SPEED_SCALING_FACTOR;
						}
						if(abs(joy_direct_old.y - joy_direct.y) / SPEED_SCALING_FACTOR <= PLAYER_SPEED_MAX_Y){
							player_local.speed_goal.y = (joy_direct.y - 128) / SPEED_SCALING_FACTOR;
						}
						if(abs(player_local.speed_current.x - player_local.speed_goal.x) >= 0.1){
							if(player_local.speed_current.x > player_local.speed_goal.x){
								player_local.speed_current.x -= PLAYER_STANDARD_ACCELERATION_X;
							}
							if(player_local.speed_current.x < player_local.speed_goal.x){
								player_local.speed_current.x += PLAYER_STANDARD_ACCELERATION_X;
							}
						}
						if(abs(player_local.speed_current.y - player_local.speed_goal.y) >= 0.1){
							if(player_local.speed_current.y > player_local.speed_goal.y){
								player_local.speed_current.y -= PLAYER_STANDARD_ACCELERATION_Y;
							}
							if(player_local.speed_current.y < player_local.speed_goal.y){
								player_local.speed_current.y += PLAYER_STANDARD_ACCELERATION_Y;
							}
						}
						player_local.position.x += player_local.speed_current.x;
						player_local.position.y += player_local.speed_current.y;
						if((player_local.position_old.x - player_local.position.x) > 0){
							if((player_local.position_old.y - player_local.position.y) > 0){
								heading_direction = HEADING_ANGLE_NW;
							}
							else if((player_local.position_old.y - player_local.position.y) < 0){
								heading_direction = HEADING_ANGLE_NE;
							}
							else{
								heading_direction = HEADING_ANGLE_N;
							}
						}
						else if((player_local.position_old.x - player_local.position.x) < 0){
							if((player_local.position_old.y - player_local.position.y) > 0){
								heading_direction = HEADING_ANGLE_SW;
							}
							else if((player_local.position_old.y - player_local.position.y) < 0){
								heading_direction = HEADING_ANGLE_SE;
							}
							else{
								heading_direction = HEADING_ANGLE_S;
							}
						}
						else{
							if((player_local.position_old.y - player_local.position.y) > 0){
								heading_direction = HEADING_ANGLE_W;
							}
							else if((player_local.position_old.y - player_local.position.y) < 0){
								heading_direction = HEADING_ANGLE_E;
							}
						}
						player_local.position_old.x = player_local.position.x;
						player_local.position_old.y = player_local.position.y;
					}
					inertia_speed.x = abs(player_local.speed_current.x);
					inertia_speed.y = abs(player_local.speed_current.y);

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
							player_local.position.x -= inertia_speed.x;
							break;
						case HEADING_ANGLE_S:
							player_local.position.x += inertia_speed.x;
							break;
						case HEADING_ANGLE_E:
							player_local.position.y += inertia_speed.y;
							break;
						case HEADING_ANGLE_W:
							player_local.position.y -= inertia_speed.y;
							break;
						case HEADING_ANGLE_NE:
							player_local.position.y += inertia_speed.y;
							player_local.position.x -= inertia_speed.x;
							break;
						case HEADING_ANGLE_NW:
							player_local.position.y -= inertia_speed.y;
							player_local.position.x -= inertia_speed.x;
							break;
						case HEADING_ANGLE_SE:
							player_local.position.y += inertia_speed.y;
							player_local.position.x += inertia_speed.x;
							break;
						case HEADING_ANGLE_SW:
							player_local.position.y -= inertia_speed.y;
							player_local.position.x += inertia_speed.x;
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
				if(player_local.position.x >= DISPLAY_SIZE_X){
					player_local.position.x = 0;
					player_local.position.y = rand() % 241;
				}
				else if(player_local.position.x <= 0){
					player_local.position.x = DISPLAY_SIZE_X;
					player_local.position.y = rand() % 241;
				}
				if(player_local.position.y >= DISPLAY_SIZE_Y){
					player_local.position.y = 0;
					player_local.position.x = rand() % 321;
				}
				else if(player_local.position.y <= 0){
					player_local.position.y = DISPLAY_SIZE_Y;
					player_local.position.x = rand() % 321;
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

//				Dumbed down local player movement
				local_x = (int) player_local.position.x;
				local_y = (int) player_local.position.y;

	//			Handling cannon shot firing

//				Local Player bullets
	//			Spawning new cannon shots on player input
				if(buttonCountWithLiftup(BUT_B) && joystick_internal.angle != JOYSTICK_ANGLE_NULL){
					local_shots[number_local_shots].status = spawn;
					local_shots[number_local_shots].position.x = local_x_lowpoly;
					local_shots[number_local_shots].position.y = local_y_lowpoly;
					local_shots[number_local_shots].angle = joystick_internal.angle;
					number_local_shots++;
					fired_bullet_this_frame = true;
				}

	//			Making fired shots disappear when reaching the screen edge
				for(incr = 0; incr < number_local_shots; incr++){
					if(local_shots[incr].position.x >= DISPLAY_SIZE_X){
						number_local_shots--;
						for(incr2 = incr; incr2 < number_local_shots; incr2++){
							memcpy(&local_shots[incr2], &local_shots[incr2 + 1], sizeof(struct shot));
						}
						lastTime_1 = xTaskGetTickCount();
						initialize_single_shot(number_local_shots + 1);
					}
					else if(local_shots[incr].position.x <= 0){
						number_local_shots--;
						for(incr2 = incr; incr2 < number_local_shots; incr2++){
							memcpy(&local_shots[incr2], &local_shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_single_shot(number_local_shots + 1);
					}

					if(local_shots[incr].position.y >= DISPLAY_SIZE_Y){
						number_local_shots--;
						for(incr2 = incr; incr2 < number_local_shots; incr2++){
							memcpy(&local_shots[incr2], &local_shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_single_shot(number_local_shots + 1);
					}
					else if(local_shots[incr].position.y <= 0){
						number_local_shots--;
						for(incr2 = incr; incr2 < number_local_shots; incr2++){
							memcpy(&local_shots[incr2], &local_shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_single_shot(number_local_shots + 1);
					}
				}
	//			Handling movement of fired shots
				for(incr = 0; incr < number_local_shots; incr++){
					switch(local_shots[incr].angle){
					case JOYSTICK_ANGLE_N:
						local_shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_S:
						local_shots[incr].position.y += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_E:
						local_shots[incr].position.x += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_W:
						local_shots[incr].position.x -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_NW:
						local_shots[incr].position.x -= LASER_BLASTER_SPEED;
						local_shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_NE:
						local_shots[incr].position.x += LASER_BLASTER_SPEED;
						local_shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_SW:
						local_shots[incr].position.x -= LASER_BLASTER_SPEED;
						local_shots[incr].position.y += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_SE:
						local_shots[incr].position.x += LASER_BLASTER_SPEED;
						local_shots[incr].position.y += LASER_BLASTER_SPEED;
						break;
					default:
						local_shots[incr].position.x = 0;
						local_shots[incr].position.y = 0;
						local_shots[incr].status = hide;
						break;
					}
				}

//				Remote Player bullets
	//			Spawning new cannon shots on player input
				if(remote_bullet_dir_total != JOYSTICK_ANGLE_NULL){
					remote_shots[number_remote_shots].status = spawn;
					remote_shots[number_remote_shots].position.x = remote_x;
					remote_shots[number_remote_shots].position.y = remote_y;
					remote_shots[number_remote_shots].angle = remote_bullet_dir_total;
					number_remote_shots++;
				}

	//			Making fired shots disappear when reaching the screen edge
				for(incr = 0; incr < number_remote_shots; incr++){
					if(remote_shots[incr].position.x >= DISPLAY_SIZE_X){
						number_remote_shots--;
						for(incr2 = incr; incr2 < number_remote_shots; incr2++){
							memcpy(&remote_shots[incr2], &remote_shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_remote_shot(number_remote_shots + 1);
					}
					else if(remote_shots[incr].position.x <= 0){
						number_remote_shots--;
						for(incr2 = incr; incr2 < number_remote_shots; incr2++){
							memcpy(&remote_shots[incr2], &remote_shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_remote_shot(number_remote_shots + 1);
					}

					if(remote_shots[incr].position.y >= DISPLAY_SIZE_Y){
						number_remote_shots--;
						for(incr2 = incr; incr2 < number_remote_shots; incr2++){
							memcpy(&remote_shots[incr2], &remote_shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_remote_shot(number_remote_shots + 1);
					}
					else if(remote_shots[incr].position.y <= 0){
						number_remote_shots--;
						for(incr2 = incr; incr2 < number_remote_shots; incr2++){
							memcpy(&remote_shots[incr2], &remote_shots[incr2 + 1], sizeof(struct shot));
						}
						initialize_remote_shot(number_remote_shots + 1);
					}
				}
	//			Handling movement of remote fired shots
				for(incr = 0; incr < number_remote_shots; incr++){
					switch(remote_shots[incr].angle){
					case JOYSTICK_ANGLE_N:
						remote_shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_S:
						remote_shots[incr].position.y += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_E:
						remote_shots[incr].position.x += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_W:
						remote_shots[incr].position.x -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_NW:
						remote_shots[incr].position.x -= LASER_BLASTER_SPEED;
						remote_shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_NE:
						remote_shots[incr].position.x += LASER_BLASTER_SPEED;
						remote_shots[incr].position.y -= LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_SW:
						remote_shots[incr].position.x -= LASER_BLASTER_SPEED;
						remote_shots[incr].position.y += LASER_BLASTER_SPEED;
						break;
					case JOYSTICK_ANGLE_SE:
						remote_shots[incr].position.x += LASER_BLASTER_SPEED;
						remote_shots[incr].position.y += LASER_BLASTER_SPEED;
						break;
					// case JOYSTICK_ANGLE_NULL:
					// 	remote_shots[incr].position.y -= LASER_BLASTER_SPEED;
					// 	break;
					default:
						remote_shots[incr].position.x = 0;
						remote_shots[incr].position.y = 0;
						remote_shots[incr].status = hide;
						break;
					}
				}

////				Detecting hits of remote player by local bullet
//				for(int i = 0; i < 4; i++){
//					if((abs(local_x_old - local_shots[i].x) < HIT_LIMIT_PLAYER_SHOT_MULTI)
//							&& (abs(local_y_old - local_shots[i].y) < HIT_LIMIT_PLAYER_SHOT_MULTI)){
////						lives[1]--;
//						number_local_shots--;
//					}
//				}
////				Detecting hits of local player by remote bullet
//				for(int i = 0; i < 4; i++){
//					if((abs(remote_x - remote_shots[i].x) < HIT_LIMIT_PLAYER_SHOT_MULTI)
//							&& (abs(remote_y - remote_shots[i].y) < HIT_LIMIT_PLAYER_SHOT_MULTI)){
////						lives[0]--;
//						number_remote_shots--;
//					}
//				}

//				/* Check if player was hit by saucer fire
//				 * Threshold zone is a square around the players ship center with 6px side length
//				 */
//				for (i = 0; i <= 1; i++) {
//					for (j = 0; j <= 9; j++) {
//						if ((abs(the_saucers[i]->shots[j].x - player.position.x) <= HIT_LIMIT_SHOT_SMALL)
//								&& (abs(the_saucers[i]->shots[j].y - player.position.y) <= HIT_LIMIT_SHOT_SMALL)) {
//							player.state = hit;
//							hit_timestamp = xTaskGetTickCount();
//						}
//					}
//				}

			// ASTEROIDS
				//Re-spawning asteroids
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


					// Timer logic
					if ((xTaskGetTickCount() - lastTime_1) >= one_second) {
						timer_1sec = 1;
						lastTime_1 = xTaskGetTickCount();
					}
					else
								timer_1sec = 0;
					if (timer_1sec == 1)
						time_passed++;
					// This creates the seed for all the following rand-Functions
					srand((uint32_t)time_passed);

					/*
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
									setSpawnPosition(all_asteroids[i]->spawn_side, i, 69);
									all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
									} break;
						case NE: if ((all_asteroids[i]->position.x >= 370)
										|| (all_asteroids[i]->position.y <= -50)) {
									setSpawnPosition(all_asteroids[i]->spawn_side, i, 69);
									all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
									} break;
						case E: if (all_asteroids[i]->position.x >= 370) {
									setSpawnPosition(all_asteroids[i]->spawn_side, i, 69);
									all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
									} break;
						case SE: if ((all_asteroids[i]->position.x >= 370)
										|| (all_asteroids[i]->position.y >= 290)) {
									setSpawnPosition(all_asteroids[i]->spawn_side, i, 69);
									all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
									} break;
						case S: if (all_asteroids[i]->position.y >= 290) {
									setSpawnPosition(all_asteroids[i]->spawn_side, i, 69);
									all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
									} break;
						case SW: if ((all_asteroids[i]->position.x <= -50)
										|| (all_asteroids[i]->position.y >= 290)) {
									setSpawnPosition(all_asteroids[i]->spawn_side, i, 69);
									all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
									} break;
						case W: if (all_asteroids[i]->position.x <= -50) {
									setSpawnPosition(all_asteroids[i]->spawn_side, i, 69);
									all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
									} break;
						case NW:if ((all_asteroids[i]->position.x <= -50)
										|| (all_asteroids[i]->position.y <= -50)) {
									setSpawnPosition(all_asteroids[i]->spawn_side, i, 69);
									all_asteroids[i]->orientation = all_asteroids[i]->initial_orientation;
									} break;
						}
					}

					/* LOCAL PLAYER
					 * Check if the player's ship was hit by asteroid */
					for (i = 0; i <= 15; i++) {
						if(all_asteroids[i]->remain_hits == one){
							if ((abs(all_asteroids[i]->position.x - local_x_lowpoly) <= HIT_LIMIT_SMALL)
							&& (abs(all_asteroids[i]->position.y - local_y_lowpoly) <= HIT_LIMIT_SMALL)) {
								player_local.state = hit;
								hit_timestamp = xTaskGetTickCount();
							}
						}
						if(all_asteroids[i]->remain_hits == two){
							if ((abs(all_asteroids[i]->position.x - local_x_lowpoly) <= HIT_LIMIT_MEDIUM)
							&& (abs(all_asteroids[i]->position.y - local_y_lowpoly) <= HIT_LIMIT_MEDIUM)) {
								player_local.state = hit;
								hit_timestamp = xTaskGetTickCount();
							}
						}
						if(all_asteroids[i]->remain_hits == three){
							if ((abs(all_asteroids[i]->position.x - local_x_lowpoly) <= HIT_LIMIT_LARGE)
							&& (abs(all_asteroids[i]->position.y - local_y_lowpoly) <= HIT_LIMIT_LARGE)) {
								player_local.state = hit;
								hit_timestamp = xTaskGetTickCount();
							}
						}
					}

					/* REMOTE PLAYER
					 * Check if the player's ship was hit by asteroid */
					for (i = 0; i <= 15; i++) {
						if(all_asteroids[i]->remain_hits == one){
							if ((abs(all_asteroids[i]->position.x - remote_x) <= HIT_LIMIT_SMALL)
							&& (abs(all_asteroids[i]->position.y - remote_y) <= HIT_LIMIT_SMALL)) {
								remote_player_state = hit;
								hit_timestamp_2 = xTaskGetTickCount();
							}
						}
						if(all_asteroids[i]->remain_hits == two){
							if ((abs(all_asteroids[i]->position.x - remote_x) <= HIT_LIMIT_MEDIUM)
							&& (abs(all_asteroids[i]->position.y - remote_y) <= HIT_LIMIT_MEDIUM)) {
								remote_player_state = hit;
								hit_timestamp_2 = xTaskGetTickCount();
							}
						}
						if(all_asteroids[i]->remain_hits == three){
							if ((abs(all_asteroids[i]->position.x - remote_x) <= HIT_LIMIT_LARGE)
							&& (abs(all_asteroids[i]->position.y - remote_y) <= HIT_LIMIT_LARGE)) {
								remote_player_state = hit;
								hit_timestamp_2 = xTaskGetTickCount();
							}
						}
					}

					/* LOCAL PLAYER
					 * Check if asteroids were hit by shot cannon blaster laser thigs
					* Threshold zone is a square around the asteroid center.
					* Square side length: small: 6px; medium: 8px; large: 12px
					*/
					// asteroid_2, _4, _6, _8, _10, _12, _14 and asteroid_16 can only be small
					for(incr = 0; incr < number_local_shots; incr++){
						for (i = 1; i <= 15; i+=2){
							if ((abs(all_asteroids[i]->position.x - local_shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
								&& (abs(all_asteroids[i]->position.y - local_shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)
								&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)
								&& (all_asteroids[i]->remain_hits == one)) {
									all_asteroids[i]->position.x = -50;
									all_asteroids[i]->position.y = -50;
									all_asteroids[i]->position_locked = true;
									all_asteroids[i]->remain_hits = none;
									score.score += POINTS_ASTEROID_SMALL;
									asteroids_to_destroy_small--;
									one_asteroid_hit_small = true;
							}
						}
					}

					// asteroid_3, asteroid_7, asteroid_9, asteroid_11, asteroid_13 and asteroid_15 can be medium or small
					for(incr = 0; incr < number_local_shots; incr++) {
						for (i = 2; i <= 15; i+=2) {
							if (i != 4) {
								if ((abs(all_asteroids[i]->position.x - local_shots[incr].position.x) <= HIT_LIMIT_SHOT_MEDIUM)
										&& (abs(all_asteroids[i]->position.y - local_shots[incr].position.y) <= HIT_LIMIT_SHOT_MEDIUM)
										&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)
										&& (all_asteroids[i]->remain_hits == two)) {
									asteroids_to_destroy_medium--;
									score.score += POINTS_ASTEROID_MEDIUM;
									one_asteroid_hit_medium = true;
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
							else if ((abs(all_asteroids[i]->position.x - local_shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
									&& (abs(all_asteroids[i]->position.y - local_shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)
									&& (all_asteroids[i]->remain_hits == one)
									&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)) {
								all_asteroids[i]->position.x = -50;
								all_asteroids[i]->position.y = -50;
								all_asteroids[i]->position_locked = true;
								all_asteroids[i]->remain_hits = none;
								score.score += POINTS_ASTEROID_SMALL;
								asteroids_to_destroy_small--;
								one_asteroid_hit_small = true;
							}
						}
					}
				}

				// asteroid_1 and asteroid_5 can be all sizes: S, M and L
				for(incr = 0; incr < number_local_shots; incr++) {
					for (i = 0; i <= 7; i+=4) {
						if ((abs(all_asteroids[i]->position.x - local_shots[incr].position.x) <= HIT_LIMIT_SHOT_LARGE)
								&& (abs(all_asteroids[i]->position.y - local_shots[incr].position.y) <= HIT_LIMIT_SHOT_LARGE)
								&& (all_asteroids[i]->remain_hits == three)) {
							asteroids_to_destroy_large--;
							score.score += POINTS_ASTEROID_LARGE;
							one_asteroid_hit_large = true;
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
						else if ((abs(all_asteroids[i]->position.x - local_shots[incr].position.x) <= HIT_LIMIT_SHOT_MEDIUM)
									&& (abs(all_asteroids[i]->position.y - local_shots[incr].position.y) <= HIT_LIMIT_SHOT_MEDIUM)
									&& (all_asteroids[i]->remain_hits == two)
									&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)) {
								asteroids_to_destroy_medium--;
								score.score += POINTS_ASTEROID_MEDIUM;
								one_asteroid_hit_medium = true;
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
						else if ((abs(all_asteroids[i]->position.x - local_shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
									&& (abs(all_asteroids[i]->position.y - local_shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)
									&& (all_asteroids[i]->remain_hits == one)
									&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)) {
								all_asteroids[i]->position.x = -50;
								all_asteroids[i]->position.y = -50;
								all_asteroids[i]->position_locked = true;
								all_asteroids[i]->remain_hits = none;
								score.score += POINTS_ASTEROID_SMALL;
								asteroids_to_destroy_small--;
								one_asteroid_hit_small = true;
						}
					}
				}

				/* REMOTE PLAYER
				* Check if asteroids were hit by shot cannon blaster laser thigs
				* Threshold zone is a square around the asteroid center.
				* Square side length: small: 6px; medium: 8px; large: 12px
				*/
				// asteroid_2, _4, _6, _8, _10, _12, _14 and asteroid_16 can only be small
				for(incr = 0; incr < number_remote_shots; incr++){
					for (i = 1; i <= 15; i+=2){
						if ((abs(all_asteroids[i]->position.x - remote_shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
							&& (abs(all_asteroids[i]->position.y - remote_shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)
							&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)
							&& (all_asteroids[i]->remain_hits == one)) {
								all_asteroids[i]->position.x = -50;
								all_asteroids[i]->position.y = -50;
								all_asteroids[i]->position_locked = true;
								all_asteroids[i]->remain_hits = none;
								score.score_remote += POINTS_ASTEROID_SMALL;
								asteroids_to_destroy_small--;
								one_asteroid_hit_small = true;
						}
					}
				}

				// asteroid_3, asteroid_7, asteroid_9, asteroid_11, asteroid_13 and asteroid_15 can be medium or small
				for(incr = 0; incr < number_remote_shots; incr++) {
					for (i = 2; i <= 15; i+=2) {
						if (i != 4) {
							if ((abs(all_asteroids[i]->position.x - remote_shots[incr].position.x) <= HIT_LIMIT_SHOT_MEDIUM)
									&& (abs(all_asteroids[i]->position.y - remote_shots[incr].position.y) <= HIT_LIMIT_SHOT_MEDIUM)
									&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)
									&& (all_asteroids[i]->remain_hits == two)) {
								asteroids_to_destroy_medium--;
								score.score_remote += POINTS_ASTEROID_MEDIUM;
								one_asteroid_hit_medium = true;
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
						else if ((abs(all_asteroids[i]->position.x - remote_shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
								&& (abs(all_asteroids[i]->position.y - remote_shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)
								&& (all_asteroids[i]->remain_hits == one)
								&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)) {
							all_asteroids[i]->position.x = -50;
							all_asteroids[i]->position.y = -50;
							all_asteroids[i]->position_locked = true;
							all_asteroids[i]->remain_hits = none;
							score.score_remote += POINTS_ASTEROID_SMALL;
							asteroids_to_destroy_small--;
							one_asteroid_hit_small = true;
						}
					}
				}
			}

			// asteroid_1 and asteroid_5 can be all sizes: S, M and L
			for(incr = 0; incr < number_remote_shots; incr++) {
				for (i = 0; i <= 7; i+=4) {
					if ((abs(all_asteroids[i]->position.x - remote_shots[incr].position.x) <= HIT_LIMIT_SHOT_LARGE)
							&& (abs(all_asteroids[i]->position.y - remote_shots[incr].position.y) <= HIT_LIMIT_SHOT_LARGE)
							&& (all_asteroids[i]->remain_hits == three)) {
						asteroids_to_destroy_large--;
						score.score_remote += POINTS_ASTEROID_LARGE;
						one_asteroid_hit_large = true;
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
					else if ((abs(all_asteroids[i]->position.x - remote_shots[incr].position.x) <= HIT_LIMIT_SHOT_MEDIUM)
								&& (abs(all_asteroids[i]->position.y - remote_shots[incr].position.y) <= HIT_LIMIT_SHOT_MEDIUM)
								&& (all_asteroids[i]->remain_hits == two)
								&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)) {
							asteroids_to_destroy_medium--;
							score.score_remote += POINTS_ASTEROID_MEDIUM;
							one_asteroid_hit_medium = true;
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
					else if ((abs(all_asteroids[i]->position.x - remote_shots[incr].position.x) <= HIT_LIMIT_SHOT_SMALL)
								&& (abs(all_asteroids[i]->position.y - remote_shots[incr].position.y) <= HIT_LIMIT_SHOT_SMALL)
												&& (all_asteroids[i]->remain_hits == one)
								&& ((xTaskGetTickCount() - all_asteroids[i]->hit_timestamp) > shot_delay)) {
							all_asteroids[i]->position.x = -50;
							all_asteroids[i]->position.y = -50;
							all_asteroids[i]->position_locked = true;
							all_asteroids[i]->remain_hits = none;
							score.score_remote += POINTS_ASTEROID_SMALL;
							asteroids_to_destroy_small--;
							one_asteroid_hit_small = true;
					}
				}
			}

			}//Only runs if UART is connected and other player is also on this screen

//			Drawing functions
			gdispClear(Black);

			if(score.score < LEVEL_TWO_SCORE_THRESHOLD && score.score_remote < LEVEL_TWO_SCORE_THRESHOLD){
				sprintf(str2, "Level 1");
			}

			else if(score.score >= LEVEL_TWO_SCORE_THRESHOLD && score.score < LEVEL_THREE_SCORE_THRESHOLD && score.score_remote >= LEVEL_TWO_SCORE_THRESHOLD && score.score_remote < LEVEL_THREE_SCORE_THRESHOLD){
				sprintf(str2, "Level 2");
			}
			else if(score.score >= LEVEL_THREE_SCORE_THRESHOLD && score.score < BEAT_GAME_SCORE_THRESHOLD && score.score_remote >= LEVEL_THREE_SCORE_THRESHOLD && score.score_remote < BEAT_GAME_SCORE_THRESHOLD){
				sprintf(str2, "Level 3");
			}

			gdispDrawString(5, 230, str2[0], font1, Green);
			if(SHOW_DEBUG_MULTI){
				if(is_master == true){
					if(remote_is_master == true){
						sprintf(user_help, "> Is master, other is master. <");
					}
					else
						sprintf(user_help, "> Is master, other is slave. <");
				}
				else if(is_master == false){
					if(remote_is_master == true){
						sprintf(user_help, "> Is slave, other is master. <");
					}
					else
						sprintf(user_help, "> Is slave, other is slave. <");
				}
				gdispDrawString(TEXT_X(user_help[0]), 220, user_help[0], font1, Green);
			}

			if(uart_connected == true){
				sprintf(user_help, "> Connected. <");
				gdispDrawString(TEXT_X(user_help[0]), 230, user_help[0], font1, Green);
				if(ready_to_start == false){
					sprintf(user_help, "> Waiting for the other player. <");
					UART_SendData(sync_byte_3);
					gdispFillArea(60, DISPLAY_CENTER_Y + 20, 200, 10, Orange);
					gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);
				}
			}
			else if(uart_connected == false && state_quit_remote == false){
				sprintf(user_help, "> UART disconnected. <");
				gdispFillArea(80, DISPLAY_CENTER_Y + 20, 160, 10, Orange);
				gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);

				sprintf(user_help, "Reconnect or press D to quit.");
				gdispFillArea(80, DISPLAY_CENTER_Y + 30, 160, 10, Orange);
				gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 30, user_help[0], font1, Black);

				if(buttonCount(BUT_D)){
					xQueueSend(HighScoresQueueMP, &score, 0);
					goto start;
				}
			}

//			Drawing 2 player ships
			if(uart_connected == true){
				sprintf(str2, "%d sec", time_passed);
				gdispDrawString(DISPLAY_CENTER_X - 5, 10, str2[0], font1, White);

				// Local Player
				if (player_local.state == fine) {
					gdispFillConvexPoly(local_x_old, local_y_old, form, (sizeof(form)/sizeof(form[0])), White);

					// Local players bullets
					for(incr = 0; incr < number_local_shots; incr++){
						if(local_shots[incr].status == spawn){
							gdispFillCircle(local_shots[incr].position.x, local_shots[incr].position.y, 3, Green);
						}
					}
				}
				else if (player_local.state == hit) {
					if (life_count_lock == false) {
						lives[0]--;
						life_count_lock = true;
					}
					if (xTaskGetTickCount() - hit_timestamp > delay_hit) {
						player_local.state = fine; // Reset the players ship if not yet game over
						life_count_lock = false; // Unlock the life counter
						player_local.position.x = DISPLAY_CENTER_X; // Reset player coordinates
						player_local.position.y = DISPLAY_CENTER_Y; // Reset player coordinates
						moved = 0; // Stop inertia until joystick input
					}
				}

				// Remote player
				if (remote_player_state == fine) {
				gdispFillConvexPoly(remote_x, remote_y, saucer_shape, (sizeof(saucer_shape)/sizeof(saucer_shape[0])), Yellow);

					// Remote players bullets
					for(incr = 0; incr < number_remote_shots; incr++){
						if(remote_shots[incr].status == spawn){
							gdispFillCircle(remote_shots[incr].position.x, remote_shots[incr].position.y, 3, Red);
						}
					}
				}
				else if (remote_player_state == hit) {
					if (life_count_lock_2 == false) {
						lives[1]--;
						life_count_lock_2 = true;
					}
					if (xTaskGetTickCount() - hit_timestamp_2 > delay_hit) {
						remote_player_state = fine; // Reset the players ship if not yet game over
						life_count_lock_2 = false; // Unlock the life counter
						remote_x = DISPLAY_CENTER_X; // Reset player coordinates
						remote_y = DISPLAY_CENTER_Y; // Reset player coordinates
					}
				}
			}

			to_send_x = local_x / 4 + 1;
			to_send_y = local_y / 3 + 1;

			if(fired_bullet_this_frame == false){
				to_send_x += 80;
				to_send_y += 80;
			}
			else if(fired_bullet_this_frame == true){
				switch((int) joystick_internal.angle){
				case JOYSTICK_ANGLE_NULL:
					to_send_x += 80;
					to_send_y += 80;
					break;
				case JOYSTICK_ANGLE_E:
					to_send_x += 160;
					to_send_y += 80;
					break;
				case JOYSTICK_ANGLE_NE:
					to_send_x += 160;
					break;
				case JOYSTICK_ANGLE_SW:
					to_send_y += 160;
					break;
				case JOYSTICK_ANGLE_S:
					to_send_x += 80;
					to_send_y += 160;
					break;
				case JOYSTICK_ANGLE_SE:
					to_send_x += 160;
					to_send_y += 160;
					break;
				case JOYSTICK_ANGLE_N:
					to_send_x += 80;
					break;
				case JOYSTICK_ANGLE_W:
					to_send_y += 80;
					break;
				case JOYSTICK_ANGLE_NW:
					break;
				}
				fired_bullet_this_frame = false;
			}

			// ASTEROIDS
			if(uart_connected == true){
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
			}

			if(show_debug == true){
				// Using lowpoly position for local version for debugging for consistency with remote
				sprintf(user_help, "Position> Local: %d, %d | Remote: %d, %d", local_x_lowpoly, local_y_lowpoly, remote_x, remote_y);
				gdispDrawString(TEXT_X(user_help[0]), 200, user_help[0], font1, White);
				// Local and remote bullet heading
				sprintf(user_help, "Bullet heading> Local: %d | Remote: %d", ((int) joystick_internal.angle), remote_bullet_dir_total);
				gdispDrawString(TEXT_X(user_help[0]), 210, user_help[0], font1, White);
			}
			if(state_pause_local == false && ready_to_start == true){
				if(!last_received){
					UART_SendData(to_send_y);
				}
				else if(last_received){
					UART_SendData(to_send_x);
				}
				last_received = !last_received;
			}
			else if(state_pause_local == true && ready_to_start == true){
				UART_SendData(pause_byte);
			}

//			Player enters pause menu
			if(buttonCount(BUT_E)){
				state_pause_local = !state_pause_local;
			}

// 			Quitting multiplayer screen
			if(state_pause_local == true || state_pause_remote == true){
				if(state_quit_remote == false){
					sprintf(user_help, "> GAME PAUSED. E to resume. <");
					gdispFillArea(75, DISPLAY_CENTER_Y + 20, 175, 10, Yellow);
					gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);

					sprintf(user_help, "> D to quit. <");
					gdispFillArea(75, DISPLAY_CENTER_Y + 30, 175, 10, Yellow);
					gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 30, user_help[0], font1, Black);
				}
				else if(state_quit_remote == true){
					sprintf(user_help, "Other player quit. D to exit.");
					gdispFillArea(75, DISPLAY_CENTER_Y + 20, 165, 10, Yellow);
					gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);
				}
				if(buttonCount(BUT_D)){
					UART_SendData(quit_byte);
					xQueueSend(HighScoresQueueMP, &score, 0);
					goto start;
				}
			}
			if(state_quit_remote == true){
				sprintf(user_help, "Other player quit. D to exit.");
				gdispFillArea(75, DISPLAY_CENTER_Y + 20, 165, 10, Yellow);
				gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);
				if(buttonCount(BUT_D)){
					UART_SendData(quit_byte);
					xQueueSend(HighScoresQueueMP, &score, 0);
					goto start;
				}
			}
//			Printing number of lives for both players
			sprintf(user_help, "Lives: ");
			gdispDrawString(240, 5, user_help, font1, White);
			sprintf(user_help, "%d", lives[0]);
			gdispDrawString(285, 5, user_help, font1, White);
			sprintf(user_help, "%d", lives[1]);
			gdispDrawString(285, 15, user_help, font1, Yellow);

//			Printing scores for both players
			sprintf(user_help, "Score: ");
			gdispDrawString(15, 5, user_help, font1, White);
			sprintf(user_help, "%d", score.score);
			gdispDrawString(60, 5, user_help, font1, White);
			sprintf(user_help, "%d", score.score_remote);
			gdispDrawString(60, 15, user_help, font1, Yellow);

//			Resetting received uart byte to guarantee quick detection of connection loss
			uart_input = 0;
//			Updating some position variables
			local_x_old = local_x; // Position of x from previous frame
			local_y_old = local_y; // Position of y from previous frame
			local_x_lowpoly = local_x_old / 4; // Low-res x, used for collisions and score
			local_x_lowpoly = local_x_lowpoly * 4;
			local_y_lowpoly = local_y_old / 3; // Low-res y, used for collisions and score
			local_y_lowpoly = local_y_lowpoly * 3;

			remote_bullet_dir_total_old = remote_bullet_dir_total;
			gdispDrawString(270, 230, fps_print[0], font1, White);

		} // Block screen until ready to draw
	} // while(1) loop
} // Actual task code
