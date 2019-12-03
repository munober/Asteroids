/*
 * drawTaskPause.c
 *
 *  Created on: Dec 2, 2019
 *      Author: lab_espl_stud04
 */
#include "includes.h"
#include "drawTaskPause.h"

extern QueueHandle_t ButtonQueue;
extern QueueHandle_t StateQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

extern QueueHandle_t JoystickAngleQueue;
extern QueueHandle_t JoystickPulseQueue;

void drawTaskPause(void * params) {
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	const unsigned char next_state_signal_single = SINGLE_PLAYER_STATE;
	double angle = 0;
	unsigned char pulse = 1;
	unsigned char joystick_angle_change = 1;
	unsigned char joystick_pulse_change = 1;
	char str[1][70] = {{0}};
	char resume[1][20] = {"Resume"};
	char quit[1][20] = {"Quit"};

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			if (xQueueReceive(JoystickAngleQueue, &angle, portMAX_DELAY) == pdTRUE)
				sprintf(str, "Angle: %f | Pulse: %c", angle, pulse);
			if (xQueueReceive(JoystickPulseQueue, &pulse, portMAX_DELAY) == pdTRUE)
				sprintf(str, "Angle: %f | Pulse: %c", angle, pulse);

			if (buttonCount(BUT_A)){
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			}
			if (buttonCount(BUT_C)){
				xQueueSend(StateQueue, &next_state_signal_single, 100);
			}

			gdispClear(Black);

			for (unsigned char i = 0; i < 1; i++){
				gdispDrawString(120, 90, resume[i],	font1, Yellow);
				gdispDrawString(120, 150, quit[i],	font1, White);
				gdispDrawString(TEXT_X(str[i]), 210, str[i], font1, White);
			}
		}
	}
}
