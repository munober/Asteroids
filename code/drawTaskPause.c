/*
 * drawTaskPause.c
 *
 *  Created on: Dec 2, 2019
 *      Author: lab_espl_stud04
 */
#include "includes.h"
#include "drawTaskPause.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern QueueHandle_t StateQueue;
extern QueueHandle_t JoystickQueue;
extern QueueHandle_t LifeCountQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

#define RESUME_SELECT		1
#define QUIT_SELECT			2

void drawTaskPause(void * params) {
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	const unsigned char next_state_signal_single = SINGLE_PLAYER_STATE;
	char status_debug[1][70] = {{0}};
	char user_help[1][70] = {"PAUSE MENU. Navigate with joystick, select with E."};
	char resume[1][70] = {"Resume"};
	char quit[1][70] = {"Quit"};
	unsigned char menu_select = RESUME_SELECT;
	struct joystick_angle_pulse joystick_internal;

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE){
				sprintf(status_debug, "x:%5d, y:%5d | DEG:%5d | P_x: %5d | P_y: %5d",
						joystick_internal.axis.x, joystick_internal.axis.y, joystick_internal.angle,
						joystick_internal.pulse.x, joystick_internal.pulse.y);
				if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
					menu_select = QUIT_SELECT;
				}
				else if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
					menu_select = RESUME_SELECT;
				}
			}

			gdispClear(Black);

			for (unsigned char i = 0; i < 1; i++){
//				gdispDrawString(TEXT_X(status_debug[i]), 0, status_debug[i], font1, White);
				gdispDrawString(TEXT_X(user_help[i]), 60, user_help[i],font1, White);
			}

			if(menu_select == RESUME_SELECT){
				for (unsigned char i = 0; i < 1; i++){
					gdispDrawString(120, 90, resume, font1, Yellow);
					gdispDrawString(120, 150, quit,	font1, White);
				}
				if (buttonCount(BUT_E)){
					xQueueSend(StateQueue, &next_state_signal_single, 100);
				}
			}
			else if(menu_select == QUIT_SELECT){
				for (unsigned char i = 0; i < 1; i++){
					gdispDrawString(120, 90, resume, font1, White);
					gdispDrawString(120, 150, quit,	font1, Yellow);
				}
				if (buttonCount(BUT_E)){
					xQueueSend(StateQueue, &next_state_signal_menu, 100);
				}
			}

		}
	}
}
