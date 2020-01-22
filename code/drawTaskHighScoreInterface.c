/*
 * drawTaskHighScoreInterface.c
 *
 *  Created on: Dec 9, 2019
 *      Author: lab_espl_stud04
 */

#include "includes.h"
#include "drawTaskHighScoreInterface.h"
#include "drawTaskHighScore.h"

extern QueueHandle_t StateQueue;
extern QueueHandle_t HighScoresQueue;
extern QueueHandle_t JoystickQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

#define NAME_INPUT					1
#define DONE						2

#define WIDTH						8
#define LINE_WIDTH					6
#define TEXT_SELECT_1_X				210
#define TEXT_SELECT_2_X				TEXT_SELECT_1_X + WIDTH
#define TEXT_SELECT_3_X				TEXT_SELECT_2_X + WIDTH
#define TEXT_SELECT_4_X				TEXT_SELECT_3_X + WIDTH
#define TEXT_SELECT_5_X				TEXT_SELECT_4_X + WIDTH
#define TEXT_SELECT_6_X				TEXT_SELECT_5_X + WIDTH

void drawTaskHighScoreInterface(void * params) {
	// Possible next states
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;

	struct joystick_angle_pulse joystick_internal;
	unsigned int menu_select = NAME_INPUT;
	unsigned int edit_mode = 0;
	unsigned int selected_letter = 0;
	char name[] = "------";

	char user_help[1][70] = {"HIGH SCORES. Navigate with joystick, select with E."};
	char done[1][70] = {"Done"};
	char edit_help[1][70] = {"Press E to type in your name"};
	char exit_help[1][70] = {"Press E to save your name:"};
	char str[30];
	int16_t score_internal = 0;
	struct highscore score_to_send;

	while (1) {
		xQueueReceive(HighScoresQueue, &score_internal, 0);
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE){
				if(!edit_mode){
					if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
						menu_select = DONE;
					}
					else if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
						menu_select = NAME_INPUT;
					}
					joystick_internal.pulse.y = JOYSTICK_PULSE_NULL;
				}
				else{
					if(joystick_internal.pulse.x == JOYSTICK_PULSE_RIGHT){
						if(selected_letter < 5){
							selected_letter++;
						}
						else if (selected_letter == 5){
							selected_letter = 0;
						}
					}
					else if(joystick_internal.pulse.x == JOYSTICK_PULSE_LEFT){
						if (selected_letter > 0){
							selected_letter--;
						}
						else if (selected_letter == 0){
							selected_letter = 5;
						}
					}

					if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
						if(name[selected_letter] == '-'){
							name[selected_letter] = 'A';
						}
						else if(name[selected_letter] < 'Z'){
							name[selected_letter]++;
						}
						else if(name[selected_letter] == 'Z'){
							name[selected_letter] = '-';
						}
					}
					else if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
						if(name[selected_letter] == '-'){
							name[selected_letter] = 'Z';
						}
						else if(name[selected_letter] > 'A'){
							name[selected_letter]--;
						}
						else if(name[selected_letter] =='A'){
							name[selected_letter] = '-';
						}
					}

					joystick_internal.pulse.x = JOYSTICK_PULSE_NULL;
					joystick_internal.pulse.y = JOYSTICK_PULSE_NULL;
				}
			}

			sprintf(str, "High score: %i | Name: %s", score_internal, name);
			gdispClear(Black);

			switch (menu_select) {
			case NAME_INPUT:
				if(buttonCount(BUT_E)){
					edit_mode = !edit_mode;
				}
				if(!edit_mode){
					gdispDrawString(TEXT_X(str), 90, str, font1, White);
					for(int i = 0; i < 1; i++){
						gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
						gdispDrawString(TEXT_X(edit_help[i]), 110, edit_help[i],font1, Yellow);
						gdispDrawString(TEXT_X(done[i]), 130, done[i], font1, White);
					}
				}
				else{
					for(int i = 0; i < 1; i++){
						gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
						gdispDrawString(TEXT_X(exit_help[i]) - 30, 110, exit_help[i],font1, Yellow);
					}
					gdispDrawString(TEXT_SELECT_1_X, 110, name, font1, White);
					switch(selected_letter) {
					case 0:
						gdispDrawLine(TEXT_SELECT_1_X, 120, TEXT_SELECT_2_X, 120, White);
						break;
					case 1:
						gdispDrawLine(TEXT_SELECT_2_X, 120, TEXT_SELECT_3_X, 120, White);
						break;
					case 2:
						gdispDrawLine(TEXT_SELECT_3_X, 120, TEXT_SELECT_4_X, 120, White);
						break;
					case 3:
						gdispDrawLine(TEXT_SELECT_4_X, 120, TEXT_SELECT_5_X, 120, White);
						break;
					case 4:
						gdispDrawLine(TEXT_SELECT_5_X, 120, TEXT_SELECT_6_X, 120, White);
						break;
					case 5:
						gdispDrawLine(TEXT_SELECT_6_X, 120, TEXT_SELECT_6_X + WIDTH, 120, White);
						break;
					}
				}
				break;
			case DONE:
				gdispDrawString(TEXT_X(str), 90, str, font1, White);
				for(int i = 0; i < 1; i++){
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
					gdispDrawString(TEXT_X(edit_help[i]), 110, edit_help[i],font1, White);
					gdispDrawString(TEXT_X(done[i]), 130, done[i], font1, Yellow);
				}
				if(buttonCount(BUT_E)){
					score_to_send.score = score_internal;
					sprintf(score_to_send.tag, "%s", name);
					xQueueSend(HighScoresQueue, &score_to_send, 0);
					xQueueSend(StateQueue, &next_state_signal_menu, 0);
				}
				break;
			}

		}
	}
}
