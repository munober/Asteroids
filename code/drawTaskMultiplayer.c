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

#define RESUME_SELECT		1
#define QUIT_SELECT			2

void drawTaskMultiplayer (void * params){
	const unsigned char next_state_signal_pause = PAUSE_MENU_STATE;
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;

	char user_help[1][70];
	struct joystick_angle_pulse joystick_internal;
	uint8_t uart_package[4];
	boolean show_debug = false;
	boolean first_check = false;
	TickType_t delay = 2000;
	TickType_t check_time = xTaskGetTickCount();

//	Movement
	struct coord joy_direct;
	struct coord joy_direct_old;
	struct players_ship player_local;
	struct players_ship player_remote;
	player_local.position.x = DISPLAY_CENTER_X;
	player_local.position.y = DISPLAY_CENTER_Y;
	player_local.position_old.x = player_local.position.x;
	player_local.position_old.y = player_local.position.y;
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
	TickType_t uart_check_start = xTaskGetTickCount();
	TickType_t uart_alive_period = 1000;
	TickType_t uart_init_delay = 5000;
	static const uint8_t alive_byte_start = 0xAA, alive_byte_uart_connected = 0x55;
	unsigned int can_start_game = 0;
	unsigned int uart_connected = 0;

	struct point form_orig[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	struct point form[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block drawing until screen is ready
			xQueueReceive(JoystickQueue, &joystick_internal, 0);
//			Receving over UART
			xQueueReceive(ESPL_RxQueue, &uart_input, 0);
			if(uart_input == alive_byte_uart_connected){
				uart_connected = 1;
			}
			else{
				uart_connected = 0;
			}
			uart_input = 0;
// 			Sending over UART
			UART_SendData(alive_byte_uart_connected);

//			Joystick input
			joy_direct.x = (int16_t)(ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
			joy_direct.y = (int16_t)(255 - (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4));
// 			Toggle to show debug content and UART Input
			if(first_check == false){
				if(buttonCount(BUT_C)){
					check_time = xTaskGetTickCount();
					first_check = true;
				}
			}
			if(first_check == true){
				if(xTaskGetTickCount() - check_time <= delay){
					if(buttonCount(BUT_C)){
						first_check = false;
						show_debug = !show_debug;
					}
				}
			}

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

//			Drawing functions
			gdispClear(Black);

			if(show_debug == true){
				if(uart_connected == 1){
					sprintf(user_help, "> UART connected. <");
					gdispDrawString(TEXT_X(user_help[0]), 230, user_help[0],font1, Green);
				}
				else if(uart_connected == 0){
					sprintf(user_help, "> UART disconnected. <");
					gdispDrawString(TEXT_X(user_help[0]), 230, user_help[0],font1, Red);
				}
			}
			gdispFillConvexPoly(player_local.position.x, player_local.position.y, form, (sizeof(form)/sizeof(form[0])), White);

			memcpy(&joy_direct_old, &joy_direct, sizeof(struct coord));
// 			Quitting multiplayer screen
			if(buttonCount(BUT_D)){
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			}

		} // Block screen until ready to draw
	} // while(1) loop
} // Actual task code
