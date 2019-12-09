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

#define TEXT_SELECT_1_X				90
#define TEXT_SELECT_2_X				95
#define TEXT_SELECT_3_X				100
#define TEXT_SELECT_4_X				105
#define TEXT_SELECT_5_X				110
#define TEXT_SELECT_6_X				115

void drawTaskHighScoreInterface(void * params) {
	// Possible next states
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;

	struct joystick_angle_pulse joystick_internal;
	unsigned int menu_select = NAME_INPUT;
	unsigned int edit_mode = 0;
	unsigned int letter_sel = 0;

	struct score score_internal;

	char user_help[1][70] = {"HIGH SCORES. Navigate with joystick, select with E."};
	char done[1][70] = {"Done"};
	char exit_help[1][70] = {"Press E to save your edit."};
	char select_dash[1][1] = {"-"};
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
			if(menu_select == NAME_INPUT){
				if(buttonCount(BUT_E)){
					edit_mode = !edit_mode;
				}
				if(edit_mode){
					if(xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE){
						if(joystick_internal.pulse.x == JOYSTICK_PULSE_RIGHT){
							if(letter_sel < 5){
								letter_sel++;
							}
							else if(letter_sel == 5){
								letter_sel = 0;
							}
						}
						if(joystick_internal.pulse.x == JOYSTICK_PULSE_LEFT){
							if(letter_sel > 0){
								letter_sel--;
							}
							else if(letter_sel == 0){
								letter_sel = 5;
							}
						}
						if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
							// scroll down thru letters
						}
						if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
							// scroll up thru letters
						}
					}
				}
			}

			sprintf(score_print, "Your score: %i. You got position 8 on the list.", &score_internal.score);
			sprintf(name_print, "Edit your name here: %c", &score_internal.name);

			gdispClear(Black);

			switch (menu_select) {
			case NAME_INPUT:
				for(int i = 0; i < 1; i++){
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
					gdispDrawString(TEXT_X(score_print[i]), 80, score_print[i], font1, White);
					gdispDrawString(TEXT_X(name_print[i]), 100, name_print[i], font1, Yellow);
					gdispDrawString(TEXT_X(done[i]), 130, done[i], font1, White);
				}
				break;
			case DONE:
				for(int i = 0; i < 1; i++){
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
					gdispDrawString(TEXT_X(score_print[i]), 80, score_print[i], font1, White);
					gdispDrawString(TEXT_X(name_print[i]), 100, name_print[i], font1, White);
					gdispDrawString(TEXT_X(done[i]), 130, done[i],	font1, Yellow);
				}
				if(buttonCount(BUT_E)){
					xQueueSend(StateQueue, &next_state_signal_menu, 0);
					xQueueSend(HighScoresQueue, &score_internal, 0);
				}
				break;
			}

			if(edit_mode){
				for(int i = 0; i < 1; i++){
					gdispDrawString(TEXT_X(exit_help[i]), 120, exit_help[i], font1, White);
				}
				switch(letter_sel){
				case 0:
					for(int i = 0; i < 1; i++){
						gdispDrawString(TEXT_SELECT_1_X, 111, select_dash[i],font1, White);
					}
					break;
				case 1:
					for(int i = 0; i < 1; i++){
						gdispDrawString(TEXT_SELECT_2_X, 111, select_dash[i],font1, White);
					}
					break;
				case 2:
					for(int i = 0; i < 1; i++){
						gdispDrawString(TEXT_SELECT_3_X, 111, select_dash[i],font1, White);
					}
					break;	
				case 3:					
					for(int i = 0; i < 1; i++){
						gdispDrawString(TEXT_SELECT_4_X, 111, select_dash[i],font1, White);
					}
					break;
				case 4:	
					for(int i = 0; i < 1; i++){
						gdispDrawString(TEXT_SELECT_5_X, 111, select_dash[i],font1, White);
					}
					break;
				case 5:
					for(int i = 0; i < 1; i++){
						gdispDrawString(TEXT_SELECT_6_X, 111, select_dash[i],font1, White);
					}
					break;
				}
			}

		}
	}
}
