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
	char user_help[1][70] = {"MULTIPLAYER COMING SOON. EXIT WITH D."};
	char uart_input[1][70] = { {0} };
	struct joystick_angle_pulse joystick_internal;

	unsigned int life_readin = 3;

	while (1) {
		xQueueReceive(LifeCountQueue, &life_readin, 0);
		xQueueReceive(JoystickQueue, &joystick_internal, 0);
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			if(buttonCount(BUT_D))
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			gdispClear(Black);
			if(xQueueReceive(ESPL_RxQueue, &uart_input, 0) == pdTRUE){
				gdispDrawString(TEXT_X(uart_input[0]), 70, uart_input[0],font1, White);
			}
			gdispDrawString(TEXT_X(user_help[0]), 60, user_help[0],font1, White);

		}
	}
}
