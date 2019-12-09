/*
 * drawTaskStartMenu.c
 *
 *  Created on: Dec 2, 2019
 *      Author: lab_espl_stud04
 */
#include "includes.h"
#include "drawTaskStartMenu.h"

extern QueueHandle_t StateQueue;
extern QueueHandle_t JoystickQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

#define SINGLEPLAYER_SELECT			1
#define MULTIPLAYER_SELECT			2
#define SETTINGS_SELECT				3
#define CHEATS_SELECT				4
#define HIGHSCORES_SELECT			5

void drawTaskStartMenu(void * params) {
	const unsigned char next_state_signal_single = SINGLE_PLAYER_STATE;
	const unsigned char next_state_signal_cheats = CHEATS_MENU_STATE;
	const unsigned char next_state_signal_highscores = HIGHSCORE_DISPLAY_STATE;
	unsigned int menu_select = SINGLEPLAYER_SELECT;

	struct joystick_angle_pulse joystick_internal;

	char single [1][20] = {"Singleplayer"};
	char multi [1][20] = {"Multiplayer"};
	char settings [1][20] = {"Settings"};
	char cheats [1][20] = {"Cheats"};
	char highscores [1][20] = {"High Scores"};
	char user_help[1][70] = {"START MENU. Navigate with joystick, select with E."};

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE){
				if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
					switch(menu_select){
					case SINGLEPLAYER_SELECT:
						menu_select = MULTIPLAYER_SELECT;
						break;
					case MULTIPLAYER_SELECT:
						menu_select = SETTINGS_SELECT;
						break;
					case SETTINGS_SELECT:
						menu_select = CHEATS_SELECT;
						break;
					case CHEATS_SELECT:
						menu_select = HIGHSCORES_SELECT;
						break;
					}
				}
				else if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
					switch(menu_select){
					case MULTIPLAYER_SELECT:
						menu_select = SINGLEPLAYER_SELECT;
						break;
					case SETTINGS_SELECT:
						menu_select = MULTIPLAYER_SELECT;
						break;
					case CHEATS_SELECT:
						menu_select = SETTINGS_SELECT;
						break;
					case HIGHSCORES_SELECT:
						menu_select = CHEATS_SELECT;
						break;
					}
				}
				joystick_internal.pulse.y = JOYSTICK_PULSE_NULL;
			}

			gdispClear(Black);

			switch (menu_select) {
			case SINGLEPLAYER_SELECT:
				for (unsigned char i = 0; i < 1; i++){
					gdispDrawString(120, 40, single[i],	font1, Yellow);
					gdispDrawString(120, 80, multi[i],	font1, White);
					gdispDrawString(120, 120, settings[i],	font1, White);
					gdispDrawString(120, 160, cheats[i],	font1, White);
					gdispDrawString(120, 200, highscores[i],	font1, White);
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
				}
				if (buttonCount(BUT_E))
					xQueueSend(StateQueue, &next_state_signal_single, 100);
				break;
			case MULTIPLAYER_SELECT:
				for (unsigned char i = 0; i < 1; i++){
					gdispDrawString(120, 40, single[i],	font1, White);
					gdispDrawString(120, 80, multi[i],	font1, Yellow);
					gdispDrawString(120, 120, settings[i],	font1, White);
					gdispDrawString(120, 160, cheats[i],	font1, White);
					gdispDrawString(120, 200, highscores[i],	font1, White);
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
				}
				break;
			case SETTINGS_SELECT:
				for (unsigned char i = 0; i < 1; i++){
					gdispDrawString(120, 40, single[i],	font1, White);
					gdispDrawString(120, 80, multi[i],	font1, White);
					gdispDrawString(120, 120, settings[i],	font1, Yellow);
					gdispDrawString(120, 160, cheats[i],	font1, White);
					gdispDrawString(120, 200, highscores[i],	font1, White);
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
				}
				break;
			case CHEATS_SELECT:
				for (unsigned char i = 0; i < 1; i++){
					gdispDrawString(120, 40, single[i],	font1, White);
					gdispDrawString(120, 80, multi[i],	font1, White);
					gdispDrawString(120, 120, settings[i],	font1, White);
					gdispDrawString(120, 160, cheats[i],	font1, Yellow);
					gdispDrawString(120, 200, highscores[i],	font1, White);
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
				}
				if (buttonCount(BUT_E))
					xQueueSend(StateQueue, &next_state_signal_cheats, 100);
				break;
			case HIGHSCORES_SELECT:
				for (unsigned char i = 0; i < 1; i++){
					gdispDrawString(120, 40, single[i],	font1, White);
					gdispDrawString(120, 80, multi[i],	font1, White);
					gdispDrawString(120, 120, settings[i],	font1, White);
					gdispDrawString(120, 160, cheats[i],	font1, White);
					gdispDrawString(120, 200, highscores[i],	font1, Yellow);
					gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
				}
				if (buttonCount(BUT_E))
					xQueueSend(StateQueue, &next_state_signal_highscores, 100);
				break;
			}
		}
	}
}
