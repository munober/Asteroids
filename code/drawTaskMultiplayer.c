/*
 * drawTaskMultiplayer.c
 *
 *  Created on: Dec 18, 2019
 *      Author: lab_espl_stud04
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
extern QueueHandle_t RemoteQueuePlayer;
extern QueueHandle_t RemoteQueueSync;
extern QueueHandle_t HighScoresQueue;

#define RESUME_SELECT		1
#define QUIT_SELECT			2

void drawTaskMultiplayer (void * params){
	const unsigned char next_state_signal_pause = PAUSE_MENU_STATE;
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	const unsigned char next_state_signal_highscoresinterface = HIGHSCORE_INTERFACE_STATE;
	int exeCount = 0;
	start:
	if(exeCount != 0){
		xQueueSend(StateQueue, &next_state_signal_highscoresinterface, 100);
	}
	exeCount = 1;
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
	int pos_x_old = 0;
	int pos_y_old = 0;
	int difference_x = 0;
	int difference_y = 0;

	int remote_x = DISPLAY_CENTER_X;
	int remote_y = DISPLAY_CENTER_Y;
	int remote_diff_x = 0;
	int remote_diff_y = 0;

	player_local.state = fine;
	int incr;
	float angle_float_goal = 0;
	float angle_float_current = 0;
	float angle_x = 0;
	float angle_y = 0;
	unsigned int moved = 0;
	char heading_direction;
	struct coord_player inertia_speed;
	struct coord_player inertia_speed_final;
	TickType_t inertia_start;

//	UART
/*
 * The way UART is implemented:
 * We have all numbers btw. 000 ... 255
 * 000 is reserved for lost connection.
 * For x coordinate of player: value btw. 001 ... 091
 * For shot bullet: value btw. 092 ... 099
 * For y coordinate of player: value btw. 100 ... 180
 * For Pause: 181
 * For Quitting the game session: 182
 */
	char uart_input = 0;
	uint8_t to_send_x = ((int) player_local.position.x) / 4 + 1;
	uint8_t to_send_y = 100 + ((int) player_local.position.y) / 3;
	char pause_byte = 181;
	char quit_byte = 182;

	boolean uart_connected = false;
	boolean state_pause_local = false;
	boolean state_pause_remote = false;
	boolean state_quit_remote = false;
	int last_sent = 0;
	int send_bullet = 0;

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
//			if(uart_input >= 91 || uart_input <= 99){
//				if(number_remote_shots != 5){
//					remote_shots[number_remote_shots].x = remote_x;
//					remote_shots[number_remote_shots].y = remote_y;
//					switch(uart_input){
//					case 91:
//						remote_shots[number_remote_shots].heading = HEADING_ANGLE_NULL;
//						break;
//					case 92:
//						remote_shots[number_remote_shots].heading = HEADING_ANGLE_E;
//						break;
//					case 93:
//						remote_shots[number_remote_shots].heading = HEADING_ANGLE_NE;
//						break;
//					case 94:
//						remote_shots[number_remote_shots].heading = HEADING_ANGLE_N;
//						break;
//					case 95:
//						remote_shots[number_remote_shots].heading = HEADING_ANGLE_NW;
//						break;
//					case 96:
//						remote_shots[number_remote_shots].heading = HEADING_ANGLE_W;
//						break;
//					case 97:
//						remote_shots[number_remote_shots].heading = HEADING_ANGLE_SW;
//						break;
//					case 98:
//						remote_shots[number_remote_shots].heading = HEADING_ANGLE_S;
//						break;
//					case 99:
//						remote_shots[number_remote_shots].heading = HEADING_ANGLE_SE;
//						break;
//					}
//					remote_shots[number_remote_shots].status = spawn;
//					number_remote_shots++;
//				}
//			}
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
//			Only runs if UART is connected, game not paused and remote hasn't quit
			if(uart_connected == true && state_pause_local == false && state_pause_remote == false && state_quit_remote == false){
				if(uart_input < 100){
					remote_x = (uart_input - 1) * 4;
				}
				else if(uart_input >= 100){
					remote_y = (uart_input - 100) * 3;
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
				}
				else if(player_local.position.x <= 0){
					player_local.position.x = DISPLAY_SIZE_X;
				}
				if(player_local.position.y >= DISPLAY_SIZE_Y){
					player_local.position.y = 0;
				}
				else if(player_local.position.y <= 0){
					player_local.position.y = DISPLAY_SIZE_Y;
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
						lives[1]--;
						number_local_shots--;
					}
				}
//				Detecting hits of local player by remote bullet
				for(int i = 0; i < 4; i++){
					if((abs(remote_x - remote_shots[i].x) < HIT_LIMIT_PLAYER_SHOT_MULTI)
							&& (abs(remote_y - remote_shots[i].y) < HIT_LIMIT_PLAYER_SHOT_MULTI)){
						lives[0]--;
						number_remote_shots--;
					}
				}

			}

//			Drawing functions
			gdispClear(Black);

			if(uart_connected == true){
				sprintf(user_help, "> UART connected. <");
				gdispDrawString(TEXT_X(user_help[0]), 230, user_help[0], font1, Green);
			}
			else if(uart_connected == false && state_quit_remote == false){
				sprintf(user_help, "> UART disconnected. <");
				gdispFillArea(80, DISPLAY_CENTER_Y + 20, 160, 10, Red);
				gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);
			}

//			Drawing 2 player ships
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

			uint8_t to_send_x = local_x / 4 + 1;
			uint8_t to_send_y = 100 + local_y / 3;
			if(show_debug == true){
				// Using lowpoly local version for debugging
				sprintf(user_help, "Local: %d, %d | Remote: %d, %d", local_x_lowpoly, local_y_lowpoly, remote_x, remote_y);
				gdispDrawString(TEXT_X(user_help[0]), 220, user_help[0], font1, White);
			}
			if(state_pause_local == false){
				if(last_sent){
					UART_SendData(to_send_y);
				}
				else if(!last_sent){
					UART_SendData(to_send_x);
				}
				last_sent = !last_sent;
			}
			else if(state_pause_local == true){
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
					gdispFillArea(70, DISPLAY_CENTER_Y + 20, 165, 10, Yellow);
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
				gdispFillArea(70, DISPLAY_CENTER_Y + 20, 165, 10, Yellow);
				gdispDrawString(TEXT_X(user_help[0]), DISPLAY_CENTER_Y + 20, user_help[0], font1, Black);
				if(buttonCount(BUT_D)){
					UART_SendData(quit_byte);
					xQueueSend(HighScoresQueue, &score, 0);
					goto start;
				}
			}
//			Printing number of lives for both players
			sprintf(user_help, "Lives: %d|", lives[0]);
			gdispDrawString(260, 10, user_help, font1, White);
			sprintf(user_help, "          %d", lives[1]);
			gdispDrawString(260, 10, user_help, font1, Yellow);

//			Printing scores for both players
			sprintf(user_help, "Score: %d|", score[0]);
			gdispDrawString(20, 10, user_help, font1, White);
			sprintf(user_help, "          %d", score[1]);
			gdispDrawString(20, 10, user_help, font1, Yellow);

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
