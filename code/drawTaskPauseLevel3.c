/*
 * drawTaskPauseLevel3.c
 *
 *  Created on: Jan 8, 2020
 *      Author: lab_espl_stud04
 */

#include "includes.h"
#include "drawTaskPauseLevel2.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern QueueHandle_t StateQueue;
extern QueueHandle_t JoystickQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

#define RESUME_SELECT		1
#define QUIT_SELECT			2

void drawTaskPauseLevel3(void * params) {
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	const unsigned char next_state_signal_single = SINGLE_PLAYER_LEVEL_3;
	char status_debug[1][70] = {{0}};
	char user_help[1][70] = {"PAUSE MENU. Navigate with joystick, select with E."};
	char resume[1][70] = {"Resume"};
	char quit[1][70] = {"Quit"};
	char dash [1][5] = {">"};
	char dash_reverse [1][5] = {"<"};
	unsigned char menu_select = RESUME_SELECT;
	boolean show_debug = false;
	boolean first_check = false;
	TickType_t delay = 2000;
	TickType_t check_time = xTaskGetTickCount();
	struct joystick_angle_pulse joystick_internal;

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE){
				sprintf(status_debug, "You found an Easter Egg!");
				if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
					menu_select = QUIT_SELECT;
				}
				else if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
					menu_select = RESUME_SELECT;
				}
			}

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

			gdispClear(Black);
			if(show_debug == true){
				gdispDrawString(TEXT_X(status_debug[0]), 0, status_debug[0], font1, White);
			}
			gdispDrawString(TEXT_X(user_help[0]), 60, user_help[0],font1, White);

			if(menu_select == RESUME_SELECT){
				gdispDrawString(120, 90, resume, font1, Yellow);
				gdispDrawString(120, 150, quit,	font1, White);
				gdispDrawString(110, 90, dash[0], font1, Yellow);
				gdispDrawString(195, 90, dash_reverse[0], font1, Yellow);
				if (buttonCount(BUT_E)){
					xQueueSend(StateQueue, &next_state_signal_single, 100);
				}
			}
			else if(menu_select == QUIT_SELECT){
				gdispDrawString(120, 90, resume, font1, White);
				gdispDrawString(120, 150, quit,	font1, Yellow);
				gdispDrawString(110, 150, dash[0], font1, Yellow);
				gdispDrawString(195, 150, dash_reverse[0], font1, Yellow);
				if (buttonCount(BUT_E)){
					menu_select = RESUME_SELECT;
					xQueueSend(StateQueue, &next_state_signal_menu, 100);
				}
			}

		}
	}
}
