/*
 * drawTaskMultiplayer.c
 *
 *  Created on: Dec 18, 2019
 *      Author: Teodor Fratiloiu
 */
#include "includes.h"
#include "drawTaskMultiplayer.h"
#include "uartTask.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern QueueHandle_t StateQueue;
extern QueueHandle_t JoystickQueue;
extern QueueHandle_t LifeCountQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;
extern QueueHandle_t ESPL_RxQueue;
extern QueueHandle_t HighScoresQueue;
extern QueueHandle_t LocalMasterQueue;

void drawTaskMultiplayer (void * params){
	const unsigned char next_state_signal_highscoresinterface = HIGHSCORE_INTERFACE_STATE;
	boolean executed = false;
	start:
	if(executed == true){
		xQueueSend(StateQueue, &next_state_signal_highscoresinterface, 100);
	}
	executed = true;
	boolean is_master = false;
	boolean remote_is_master = false;
	int score[2];
	score[0] = 0;
	score[1] = 0;
	char user_help[1][70];
	struct joystick_angle_pulse joystick_internal;
	boolean show_debug = false;
	boolean first_check = false;
	TickType_t double_toggle_delay = 2000;
	TickType_t check_time = xTaskGetTickCount();

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
	int remote_bullet_dir_x = HEADING_ANGLE_NULL;
	int remote_bullet_dir_y = HEADING_ANGLE_NULL;
	int remote_bullet_dir_total = HEADING_ANGLE_NULL;

	player_local.state = fine;
	int incr;
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
	TickType_t game_start = xTaskGetTickCount();
	TickType_t sync_period = 200;

	char uart_input = 0;
	uint8_t to_send_x = ((int) player_local.position.x) / 4 + 1;
	uint8_t to_send_y = 100 + ((int) player_local.position.y) / 3;
	uint8_t to_send_sync = sync_byte_1;

	boolean uart_connected = false;
	boolean state_pause_local = false;
	boolean state_pause_remote = false;
	boolean state_quit_remote = false;
	boolean no_sync = true;
	boolean ready_to_start = false;
	int last_sent = 0;
	int last_received = 0;

	int lives[2];
	lives[0] = STARTING_LIVES_MULTIPLAYER; // local
	lives[1] = STARTING_LIVES_MULTIPLAYER; // remote

//	Bullets
	int number_local_shots = 0;
	struct shot_multiplayer local_shots[5];
	for(int i = 0; i < 4; i++){
		local_shots[i].status = hide;
		local_shots[i].x = local_x_old;
		local_shots[i].y = local_y_old;
		local_shots[i].heading = HEADING_ANGLE_NULL;
	}
	int number_remote_shots = 0;
	struct shot_multiplayer remote_shots[5];
	for(int i = 0; i < 4; i++){
		remote_shots[i].status = hide;
		remote_shots[i].x = local_x_old;
		remote_shots[i].y = local_y_old;
		remote_shots[i].heading = HEADING_ANGLE_NULL;
	}

	struct point form_orig[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	struct point form[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	const point saucer_shape[] = { { -10, 3 }, { -6, 6 }, { 6, 6 }, { 10, 3 }, { -10, 3 },
			{ -6, 0 }, { 6, 0 }, { 10, 3 }, { 6, 0 }, { 4, -5 }, { -4, -5 },
			{ -6, 0 } };

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block drawing until screen is ready
			xQueueReceive(JoystickQueue, &joystick_internal, 0);
			xQueueReceive(ESPL_RxQueue, &uart_input, 0);
			xQueueReceive(LocalMasterQueue, &is_master, 0);
			if(is_master == true){
				remote_is_master = false;
			}
			else if(is_master == false){
				remote_is_master = true;
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
				if(last_received){
					if(uart_input >= 1 && uart_input <= 80){
						remote_y = (uart_input - 1) * 3;
						remote_bullet_dir_y = HEADING_ANGLE_N;
					}
					else if(uart_input >= 81 && uart_input <= 160){
						remote_y = (uart_input - 81) * 3;
						remote_bullet_dir_y = HEADING_ANGLE_NULL;
					}
					else if(uart_input >= 161 && uart_input <= 240){
						remote_y = (uart_input - 161) * 3;
						remote_bullet_dir_y = HEADING_ANGLE_S;
					}
				}
				else if(!last_received){
					if(uart_input >= 1 && uart_input <= 80){
						remote_x = (uart_input - 1) * 4;
						remote_bullet_dir_x = HEADING_ANGLE_W;
					}
					else if(uart_input >= 81 && uart_input <= 160){
						remote_x = (uart_input - 81) * 4;
						remote_bullet_dir_x = HEADING_ANGLE_NULL;
					}
					else if(uart_input >= 161 && uart_input <= 240){
						remote_x = (uart_input - 161) * 4;
						remote_bullet_dir_x = HEADING_ANGLE_E;
					}
				}
				if(remote_bullet_dir_x == HEADING_ANGLE_NULL){
					if(remote_bullet_dir_y == HEADING_ANGLE_N){
						remote_bullet_dir_total = HEADING_ANGLE_N;
					}
					else if(remote_bullet_dir_y == HEADING_ANGLE_NULL){
						remote_bullet_dir_total = HEADING_ANGLE_NULL;
					}
					else if(remote_bullet_dir_y == HEADING_ANGLE_S){
						remote_bullet_dir_total = HEADING_ANGLE_S;
					}
				}
				else if(remote_bullet_dir_x == HEADING_ANGLE_W){
					if(remote_bullet_dir_y == HEADING_ANGLE_N){
						remote_bullet_dir_total = HEADING_ANGLE_NW;
					}
					else if(remote_bullet_dir_y == HEADING_ANGLE_NULL){
						remote_bullet_dir_total = HEADING_ANGLE_W;
					}
					else if(remote_bullet_dir_y == HEADING_ANGLE_S){
						remote_bullet_dir_total = HEADING_ANGLE_SW;
					}
				}
				else if(remote_bullet_dir_x == HEADING_ANGLE_E){
					if(remote_bullet_dir_y == HEADING_ANGLE_N){
						remote_bullet_dir_total = HEADING_ANGLE_NE;
					}
					else if(remote_bullet_dir_y == HEADING_ANGLE_NULL){
						remote_bullet_dir_total = HEADING_ANGLE_E;
					}
					else if(remote_bullet_dir_y == HEADING_ANGLE_S){
						remote_bullet_dir_total = HEADING_ANGLE_SE;
					}
				}
				last_received = !last_received;

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
					player_local.position.y += 10;
				}
				else if(player_local.position.x <= 0){
					player_local.position.x = DISPLAY_SIZE_X;
					player_local.position.y += 10;
				}
				if(player_local.position.y >= DISPLAY_SIZE_Y){
					player_local.position.y = 0;
					player_local.position.x += 10;
				}
				else if(player_local.position.y <= 0){
					player_local.position.y = DISPLAY_SIZE_Y;
					player_local.position.x += 10;
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

//				Bullet direction
				if(buttonCountWithLiftup(BUT_B)){
					if(number_local_shots != 5){
						local_shots[number_local_shots].x = local_x_old;
						local_shots[number_local_shots].y = local_y_old;
						local_shots[number_local_shots].heading = heading_direction;
						local_shots[number_local_shots].status = spawn;
						number_local_shots++;
					}
				}
//				Detecting hits of remote player by local bullet
				for(int i = 0; i < 4; i++){
					if((abs(local_x_old - local_shots[i].x) < HIT_LIMIT_PLAYER_SHOT_MULTI)
							&& (abs(local_y_old - local_shots[i].y) < HIT_LIMIT_PLAYER_SHOT_MULTI)){
//						lives[1]--;
						number_local_shots--;
					}
				}
//				Detecting hits of local player by remote bullet
				for(int i = 0; i < 4; i++){
					if((abs(remote_x - remote_shots[i].x) < HIT_LIMIT_PLAYER_SHOT_MULTI)
							&& (abs(remote_y - remote_shots[i].y) < HIT_LIMIT_PLAYER_SHOT_MULTI)){
//						lives[0]--;
						number_remote_shots--;
					}
				}

			}

//			Drawing functions
			gdispClear(Black);

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
					xQueueSend(HighScoresQueue, &score, 0);
					goto start;
				}
			}

//			Drawing 2 player ships
			if(uart_connected == true){
				gdispFillConvexPoly(local_x_old, local_y_old, form, (sizeof(form)/sizeof(form[0])), White);
				gdispFillConvexPoly(remote_x, remote_y, saucer_shape, (sizeof(saucer_shape)/sizeof(saucer_shape[0])), Yellow);
	//			Drawing bullets
	//			Local
				for(incr = 0; incr < number_local_shots; incr++){
					if(local_shots[incr].status == spawn){
						gdispFillCircle(local_shots[incr].x, local_shots[incr].y, 3, Green);
					}
				}
	//			Remote
				for(incr = 0; incr < number_remote_shots; incr++){
					if(remote_shots[incr].status == spawn){
						gdispFillCircle(remote_shots[incr].x, remote_shots[incr].y, 3, Red);
					}
				}
			}
			to_send_x = local_x / 4 + 1;
			to_send_y = local_y / 3 + 1;
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
				if(last_sent){
					UART_SendData(to_send_y);
				}
				else if(!last_sent){
					UART_SendData(to_send_x);
				}
				last_sent = !last_sent;
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
					sprintf(user_help, "> GAME PAUSED. D to quit. <");
					gdispFillArea(75, DISPLAY_CENTER_Y + 20, 160, 10, Yellow);
					gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);
				}
				else if(state_quit_remote == true){
					sprintf(user_help, "Other player quit. D to exit.");
					gdispFillArea(75, DISPLAY_CENTER_Y + 20, 165, 10, Yellow);
					gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);
				}
				if(buttonCount(BUT_D)){
					UART_SendData(quit_byte);
					xQueueSend(HighScoresQueue, &score, 0);
					goto start;
				}
			}
			if(state_quit_remote == true){
				sprintf(user_help, "Other player quit. D to exit.");
				gdispFillArea(75, DISPLAY_CENTER_Y + 20, 165, 10, Yellow);
				gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);
				if(buttonCount(BUT_D)){
					UART_SendData(quit_byte);
					xQueueSend(HighScoresQueue, &score, 0);
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
			sprintf(user_help, "%d", score[0]);
			gdispDrawString(60, 5, user_help, font1, White);
			sprintf(user_help, "%d", score[1]);
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
		} // Block screen until ready to draw
	} // while(1) loop
} // Actual task code
