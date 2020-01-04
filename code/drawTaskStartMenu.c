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
//  Possible next states
	const unsigned char next_state_signal_single = SINGLE_PLAYER_STATE;
	const unsigned char next_state_signal_cheats = CHEATS_MENU_STATE;
	const unsigned char next_state_signal_highscores = HIGHSCORE_DISPLAY_STATE;
	const unsigned char next_state_signal_multiplayer = MULTIPLAYER_STATE;
	unsigned int menu_select = SINGLEPLAYER_SELECT;

	struct joystick_angle_pulse joystick_internal;

	char version [1][30] = { 0 }; // Use this variable to print a custom message on the start menu.
	sprintf(version, "Build number -- %i", BUILD_NUMBER);
	char single [1][20] = {"Singleplayer"};
	char multi [1][20] = {"Multiplayer"};
	char settings [1][20] = {"About"};
	char cheats [1][20] = {"Cheats"};
	char highscores [1][20] = {"High Scores"};
	char dash [1][5] = {">"};
	char dash_reverse [1][5] = {"<"};
	char user_help[1][70];
	char user_help_content[1][70] = {"Navigate with joystick, select with E."};
	sprintf(user_help[0], " %s", user_help_content);

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
					case HIGHSCORES_SELECT:
						menu_select = SINGLEPLAYER_SELECT;
						break;
					}
				}
				else if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
					switch(menu_select){
					case SINGLEPLAYER_SELECT:
						menu_select = HIGHSCORES_SELECT;
						break;
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
			gdispDrawString(TEXT_X(version[0]), 220, version[0],font1, White);
			gdispDrawString(TEXT_X(user_help[0]), 10, user_help[0],font1, White);

			switch (menu_select) {
			case SINGLEPLAYER_SELECT:
				gdispDrawString(120, 40, single[0],	font1, Yellow);
				gdispDrawString(120, 80, multi[0],	font1, White);
				gdispDrawString(120, 120, settings[0],	font1, White);
				gdispDrawString(120, 160, cheats[0],	font1, White);
				gdispDrawString(120, 200, highscores[0],	font1, White);
				gdispDrawString(110, 40, dash[0], font1, Yellow);
				gdispDrawString(195, 40, dash_reverse[0], font1, Yellow);
				if (buttonCount(BUT_E))
					xQueueSend(StateQueue, &next_state_signal_single, 100);
				break;
			case MULTIPLAYER_SELECT:
					gdispDrawString(120, 40, single[0],	font1, White);
					gdispDrawString(120, 80, multi[0],	font1, Yellow);
					gdispDrawString(120, 120, settings[0],	font1, White);
					gdispDrawString(120, 160, cheats[0],	font1, White);
					gdispDrawString(120, 200, highscores[0],	font1, White);
					gdispDrawString(110, 80, dash[0], font1, Yellow);
					gdispDrawString(195, 80, dash_reverse[0], font1, Yellow);
				if(buttonCount(BUT_E))
					xQueueSend(StateQueue, &next_state_signal_multiplayer, 100);
				break;
			case SETTINGS_SELECT:
				gdispDrawString(120, 40, single[0],	font1, White);
				gdispDrawString(120, 80, multi[0],	font1, White);
				gdispDrawString(120, 120, settings[0],	font1, Yellow);
				gdispDrawString(120, 160, cheats[0],	font1, White);
				gdispDrawString(120, 200, highscores[0],	font1, White);
				gdispDrawString(110, 120, dash[0], font1, Yellow);
				gdispDrawString(195, 120, dash_reverse[0], font1, Yellow);
				break;
			case CHEATS_SELECT:
				gdispDrawString(120, 40, single[0],	font1, White);
				gdispDrawString(120, 80, multi[0],	font1, White);
				gdispDrawString(120, 120, settings[0],	font1, White);
				gdispDrawString(120, 160, cheats[0],	font1, Yellow);
				gdispDrawString(120, 200, highscores[0],	font1, White);
				gdispDrawString(110, 160, dash[0], font1, Yellow);
				gdispDrawString(195, 160, dash_reverse[0], font1, Yellow);
				if (buttonCount(BUT_E))
					xQueueSend(StateQueue, &next_state_signal_cheats, 100);
				break;
			case HIGHSCORES_SELECT:
				gdispDrawString(120, 40, single[0],	font1, White);
				gdispDrawString(120, 80, multi[0],	font1, White);
				gdispDrawString(120, 120, settings[0],	font1, White);
				gdispDrawString(120, 160, cheats[0],	font1, White);
				gdispDrawString(120, 200, highscores[0],	font1, Yellow);
				gdispDrawString(110, 200, dash[0], font1, Yellow);
				gdispDrawString(195, 200, dash_reverse[0], font1, Yellow);
				if (buttonCount(BUT_E))
					xQueueSend(StateQueue, &next_state_signal_highscores, 100);
				break;
			}
		}
	}
}
