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

#define RESUME_SELECT		1
#define QUIT_SELECT			2

void drawTaskMultiplayer (void * params){
	const unsigned char next_state_signal_pause = PAUSE_MENU_STATE;
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;

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
	char uart_input = 0;
//	int uart_input_number;
	uint8_t to_send_x = ((int) player_local.position.x) / 4 + 1;
	uint8_t to_send_y = 100 + ((int) player_local.position.y) / 3;

	boolean uart_connected = false;
	int last_sent = 0;

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
			else{
				uart_connected = false;
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

			if(uart_connected == true){
				if(uart_input < 100){
					remote_x = (uart_input - 1) * 4;
				}
				else if(uart_input >= 100){
					remote_y = (uart_input - 100) * 3;
				}
//			Decoding received UART message
//				uart_input_number = uart_input;
//				received_buffer[3] = uart_input_number % 5;
//				temp = uart_input_number / 5;
//				received_buffer[2] = temp % 5;
//				temp = temp / 5;
//				received_buffer[1] = temp % 5;
//				temp = temp / 5;
//				received_buffer[0] = temp % 5;

//				switch(received_buffer[3]){
//				case 0:
//					remote_diff_x = -2;
//					break;
//				case 1:
//					remote_diff_x = -1;
//					break;
//				case 2:
//					remote_diff_x = 0;
//					break;
//				case 3:
//					remote_diff_x = 1;
//					break;
//				case 4:
//					remote_diff_x = 2;
//					break;
//				}
//				switch(received_buffer[2]){
//				case 0:
//					remote_diff_y = -2;
//					break;
//				case 1:
//					remote_diff_y = -1;
//					break;
//				case 2:
//					remote_diff_y = 0;
//					break;
//				case 3:
//					remote_diff_y = 1;
//					break;
//				case 4:
//					remote_diff_y = 2;
//					break;
//				}
//				remote_x += remote_diff_x;
//				remote_y += remote_diff_y;

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

//			Make remote player show up at the other side of the screen when reaching screen border
//				if(remote_x >= DISPLAY_SIZE_X){
//					remote_x = 0;
//				}
//				else if(remote_x <= 0){
//					remote_x = DISPLAY_SIZE_X;
//				}
//				if(remote_y >= DISPLAY_SIZE_Y){
//					remote_y = 0;
//				}
//				else if(remote_y <= 0){
//					remote_y = DISPLAY_SIZE_Y;
//				}


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

//			Dumbed down local player movement
				local_x = (int) player_local.position.x;
				local_y = (int) player_local.position.y;

	//			Some math for remote board
//				difference_x = (int) (player_local.position.x - pos_x_old);
//				difference_y = (int) (player_local.position.y - pos_y_old);
//				pos_x_old = player_local.position.x;
//				pos_y_old = player_local.position.y;

//				to_send = to_send_buffer[3] + 5 * to_send_buffer[2] + 25 * to_send_buffer[1] + 125 * to_send_buffer[0];


//			Encoding UART package
//				switch(difference_x){
//				case -2:
//					to_send_buffer[3] = 0;
//					break;
//				case -1:
//					to_send_buffer[3] = 1;
//					break;
//				case 0:
//					to_send_buffer[3] = 2;
//					break;
//				case 1:
//					to_send_buffer[3] = 3;
//					break;
//				case 2:
//					to_send_buffer[3] = 4;
//					break;
//				}
//				switch(difference_y){
//				case -2:
//					to_send_buffer[2] = 0;
//					break;
//				case -1:
//					to_send_buffer[2] = 1;
//					break;
//				case 0:
//					to_send_buffer[2] = 2;
//					break;
//				case 1:
//					to_send_buffer[2] = 3;
//					break;
//				case 2:
//					to_send_buffer[2] = 4;
//					break;
//				}
			}

//			Drawing functions
			gdispClear(Black);

			if(show_debug == true){
				if(uart_connected == true){
					sprintf(user_help, "> UART connected. <");
					gdispDrawString(TEXT_X(user_help[0]), 230, user_help[0], font1, Green);
				}
				else if(uart_connected == false){
					sprintf(user_help, "> UART disconnected. <");
					gdispDrawString(TEXT_X(user_help[0]), 230, user_help[0], font1, Red);
				}
			}

//			Drawing 2 player ships
			gdispFillConvexPoly(local_x, local_y, form, (sizeof(form)/sizeof(form[0])), White);
			gdispFillConvexPoly(remote_x, remote_y, saucer_shape, (sizeof(saucer_shape)/sizeof(saucer_shape[0])), Yellow);

			uint8_t to_send_x = local_x / 4 + 1;
			uint8_t to_send_y = 100 + local_y / 3;
			sprintf(user_help, "To send: %d, %d", to_send_x, to_send_y);
			gdispDrawString(TEXT_X(user_help[0]), 220, user_help[0], font1, White);
			if(last_sent){
				UART_SendData(to_send_y);
			}
			else if(!last_sent){
				UART_SendData(to_send_x);
			}
			last_sent = !last_sent;

// 			Quitting multiplayer screen
			if(buttonCount(BUT_D)){
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			}
			uart_input = 0;
		} // Block screen until ready to draw
	} // while(1) loop
} // Actual task code
