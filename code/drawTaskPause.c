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
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

void drawTaskPause(void * params) {
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	const unsigned char next_state_signal_single = SINGLE_PLAYER_STATE;
	char str[1][70] = {{0}};
	char resume[1][70] = {"Resume"};
	char quit[1][70] = {"Quit"};
	struct joystick_angle_pulse joystick_internal;

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE){
				sprintf(str, "x:%5d, y:%5d | DEG:%5d | P_x: %5d | P_y: %5d",
						joystick_internal.axis.x, joystick_internal.axis.y, joystick_internal.angle,
						joystick_internal.pulse.x, joystick_internal.pulse.y);
			}

			if (buttonCount(BUT_A)){
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			}
			if (buttonCount(BUT_C)){
				xQueueSend(StateQueue, &next_state_signal_single, 100);
			}

			gdispClear(Black);

			for (unsigned char i = 0; i < 1; i++){
				gdispDrawString(TEXT_X(str[i]), 0, str[i], font1, White);
			}

			for (unsigned char i = 0; i < 1; i++){
				gdispDrawString(120, 90, resume, font1, Yellow);
				gdispDrawString(120, 150, quit,	font1, White);
				gdispDrawString(TEXT_X(str[i]), 0, str[i], font1, White);
			}
		}
	}
}
