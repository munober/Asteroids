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
	char uart_input;
	char uart_text[1][70] = {"Nothing received over UART so far."};
	struct joystick_angle_pulse joystick_internal;
	uint8_t uart_package[4];
	boolean show_debug = false;
	boolean first_check = false;
	TickType_t delay = 2000;
	TickType_t check_time = xTaskGetTickCount();

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
	float angle_float = 0;
	float angle_x = 0;
	float angle_y = 0;
	unsigned int moved = 0;
	char heading_direction;
	struct coord_player inertia_speed;
	TickType_t inertia_start;
	TickType_t inertia_period = 100;
	char shot_fired_byte = 255;

	struct point form_orig[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };
	struct point form[] = { { -6, 6 }, { 0, -12 }, { 6, 6 } };

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block drawing until screen is ready
			xQueueReceive(JoystickQueue, &joystick_internal, 0);
//			Receving over UART
			if(USART_GetITStatus(USART1, USART_IT_RXNE)){
				uart_input = USART1->DR;
				sprintf(uart_text, uart_input);
			}
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
			if((joy_direct.x > 136) || (joy_direct.x < 120) || (joy_direct.y > 136) || (joy_direct.y < 120)){
				moved = 1;
				inertia_start = xTaskGetTickCount();
			}
			else{
				moved = 0;
			}

			if(moved){
				if(player_local.position.x <= DISPLAY_SIZE_X && player_local.position.y <= DISPLAY_SIZE_Y){
					player_local.position.x += (joy_direct.x - 128) / 32;
					player_local.position.y += (joy_direct.y - 128) / 64;
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
						else{
							heading_direction = HEADING_ANGLE_NULL;
						}	
					} 
					player_local.position_old.x = player_local.position.x;
					player_local.position_old.y = player_local.position.y;
				}
			}

			if(xTaskGetTickCount() == inertia_start){
				inertia_speed.x = INERTIA_SPEED_INITIAL_X + ((abs(joy_direct_old.x - joy_direct.x)) / 16);
				inertia_speed.y = INERTIA_SPEED_INITIAL_Y + ((abs(joy_direct_old.y - joy_direct.y)) / 32);
			}
			if(inertia_speed.x > INERTIA_MIN_SPEED_X){
				if((xTaskGetTickCount() - inertia_start) % INERTIA_TIME_INCREMENT == 0){
					inertia_speed.x -= INERTIA_DECELERATE_X;
				}
			}
			if(inertia_speed.y > INERTIA_MIN_SPEED_Y){
				if((xTaskGetTickCount() - inertia_start) % INERTIA_TIME_INCREMENT == 0){
					inertia_speed.y -= INERTIA_DECELERATE_Y;
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

//			Local Player Ship rotation
			angle_x = (float)((int16_t)joy_direct.x-128);
			angle_y = (float)((int16_t)joy_direct.y-128);
			angle_float = 0;
			if (abs(joy_direct.x - 128) > 5 || abs(joy_direct.y - 128) > 5){
				if((angle_x != 0) && (angle_y != 0)){
					if(angle_y != 128){
						angle_float = (CONVERT_TO_DEG * atan2f(angle_y, angle_x)) + 90;
					}
					else{
						angle_float = 0;
					}
				}
				else{
					angle_float = 0;
				}
			}

			memcpy(&form, &form_orig, 3 * sizeof(struct point));
			for(incr = 0; incr < 3; incr++){
				form[incr].x = form_orig[incr].x * cos(angle_float * CONVERT_TO_RAD)
									- form_orig[incr].y * sin(angle_float * CONVERT_TO_RAD);
				form[incr].y = form_orig[incr].x * sin(angle_float * CONVERT_TO_RAD)
									+ form_orig[incr].y * cos(angle_float * CONVERT_TO_RAD);
			}

//			Drawing functions
			gdispClear(Black);
			sprintf(user_help, "Angle: %i", moved);
			gdispDrawString(TEXT_X(user_help[0]), 20, user_help[0],font1, White);
			if(show_debug == true){
				gdispDrawString(TEXT_X(uart_text[0]), 10, uart_text[0],font1, White);
			}
			gdispFillConvexPoly(player_local.position.x, player_local.position.y, form, (sizeof(form)/sizeof(form[0])), White);

			UART_SendData(player_local.position.x / 2);
			UART_SendData(player_local.position.y);
			UART_SendData(angle_float / 2);
			if(buttonCountWithLiftup(BUT_B)){
				UART_SendData(shot_fired_byte);
			}

			memcpy(&joy_direct_old, &joy_direct, sizeof(struct coord));
// 			Quitting multiplayer screen
			if(buttonCount(BUT_D)){
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			}

		} // Block screen until ready to draw
	} // while(1) loop
} // Actual task code
