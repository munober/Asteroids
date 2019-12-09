/*
 * drawTaskHighScoreInterface.c
 *
 *  Created on: Dec 9, 2019
 *      Author: lab_espl_stud04
 */

#include "includes.h"
#include "drawTaskHighScoreInterface.h"

extern QueueHandle_t StateQueue;
extern QueueHandle_t HighScoresQueue;
extern QueueHandle_t JoystickQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

#define NAME_INPUT					1
#define DONE						2

void drawTaskHighScoreInterface(void * params) {
	// Possible next states
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;

	struct joystick_angle_pulse joystick_internal;
	unsigned int menu_select = NAME_INPUT;

	struct score score_internal;

	char user_help[1][70] = {"HIGH SCORES. Enter your name below."};
	char done[1][70] = {"Done"};
	char name_print[1][70] = { 0 };
	char score_print[1][70] = { 0 };

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE){
				if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
					menu_select = DONE;
				}
				else if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
					menu_select = NAME_INPUT;
				}
				joystick_internal.pulse.y = JOYSTICK_PULSE_NULL;
			}
//			if(menu_select == NAME_INPUT){
//				if(xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE)){
//					if(joystick_internal.pulse.x == JOYSTICK_PULSE_RIGHT){
//						if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN)
//						else if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN)
//					}
//					if(joystick_internal.pulse.x == JOYSTICK_PULSE_LEFT){
//						if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN)
//						else if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN)
//					}
//				}
//			}

			sprintf(score_print, "Your score: %i. You got position 8 on the list.", &score_internal.score);
			sprintf(name_print, "Enter your name here: %c", &score_internal.name);

			gdispClear(Black);

			switch (menu_select) {
			case NAME_INPUT:
				for(int i = 0; i < 1; i++){
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
					gdispDrawString(TEXT_X(score_print[i]), 80, score_print[i], font1, White);
					gdispDrawString(TEXT_X(name_print[i]), 100, name_print[i], font1, Yellow);
					gdispDrawString(TEXT_X(done[i]), 120, done[i], font1, White);
				}
				break;
			case DONE:
				for(int i = 0; i < 1; i++){
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
					gdispDrawString(TEXT_X(score_print[i]), 80, score_print[i], font1, White);
					gdispDrawString(TEXT_X(name_print[i]), 100, name_print[i], font1, White);
					gdispDrawString(TEXT_X(done[i]), 120, done[i],	font1, Yellow);
				}
				if(buttonCount(BUT_E)){
					xQueueSend(StateQueue, &next_state_signal_menu, 0);
					xQueueSend(HighScoresQueue, &score_internal, 0);
				}
				break;
			}
		}
	}
}
