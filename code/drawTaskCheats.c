/*
 * drawTaskCheats.c
 *
 *  Created on: Dec 5, 2019
 *      Author: Teodor Fratiloiu
 */

#include "includes.h"
#include "drawTaskCheats.h"

extern QueueHandle_t StateQueue;
extern QueueHandle_t JoystickQueue;
extern QueueHandle_t LifeCountQueue1;
extern QueueHandle_t LifeCountQueue2;
extern QueueHandle_t LifeCountQueue3;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

#define CLASSIC_3_LIVES				1
#define INFINITE_LIVES				2
#define GRANULAR_SELECT				3
#define BACK						4

void drawTaskCheats(void * params) {
	// Possible next states
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;

	unsigned int menu_select = CLASSIC_3_LIVES;

	struct joystick_angle_pulse joystick_internal;

	char single [1][20] = {"Classic | 3 lives"};
	char multi [1][20] = {"9000 lives"};
	char settings [1][20] = {"Granaular: "};
	char cheats [1][20] = {"Back"};
	char user_help[1][70] = {"Navigate with joystick, select with E."};
	char lives_display[20];
	char dash [1][5] = {">"};
	char dash_reverse [1][5] = {"<"};

	unsigned int starting_lives = 3;
	unsigned int starting_lives_old = starting_lives;

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			if (xQueueReceive(JoystickQueue, &joystick_internal, 0) == pdTRUE){
				if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
					switch(menu_select){
					case CLASSIC_3_LIVES:
						menu_select = INFINITE_LIVES;
						break;
					case INFINITE_LIVES:
						menu_select = GRANULAR_SELECT;
						break;
					case GRANULAR_SELECT:
						menu_select = BACK;
						break;
					}
				}
				else if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
					switch(menu_select){
					case BACK:
						menu_select = GRANULAR_SELECT;
						break;
					case GRANULAR_SELECT:
						menu_select = INFINITE_LIVES;
						break;
					case INFINITE_LIVES:
						menu_select = CLASSIC_3_LIVES;
						break;
					}
				}
				joystick_internal.pulse.y = JOYSTICK_PULSE_NULL;
			}

			sprintf(lives_display, " %i", starting_lives);
			gdispClear(Black);

			switch (menu_select) {
			case CLASSIC_3_LIVES:
				gdispDrawString(120, 30, single[0],	font1, Yellow);
				gdispDrawString(120, 90, multi[0],	font1, White);
				gdispDrawString(120, 150, settings[0],	font1, White);
				gdispDrawString(120, 210, cheats[0],	font1, White);
				gdispDrawString(TEXT_X(user_help[0]), 10, user_help[0],font1, White);
				gdispDrawString(190, 150, lives_display[0],	font1, White);
				gdispDrawString(110, 30, dash[0], font1, Yellow);
				gdispDrawString(225, 30, dash_reverse[0], font1, Yellow);
				if(buttonCount(BUT_E)){
					starting_lives = 3;
				}
				break;
			case INFINITE_LIVES:
				gdispDrawString(120, 30, single[0],	font1, White);
				gdispDrawString(120, 90, multi[0],	font1, Yellow);
				gdispDrawString(120, 150, settings[0],	font1, White);
				gdispDrawString(120, 210, cheats[0],	font1, White);
				gdispDrawString(TEXT_X(user_help[0]), 10, user_help[0],font1, White);
				gdispDrawString(190, 150, lives_display[0],	font1, White);
				gdispDrawString(110, 90, dash[0], font1, Yellow);
				gdispDrawString(225, 90, dash_reverse[0], font1, Yellow);
				if(buttonCount(BUT_E)){
					starting_lives = 9000;
				}
				break;
			case GRANULAR_SELECT:
				gdispDrawString(120, 30, single[0],	font1, White);
				gdispDrawString(120, 90, multi[0],	font1, White);
				gdispDrawString(120, 150, settings[0],	font1, Yellow);
				gdispDrawString(120, 210, cheats[0],	font1, White);
				gdispDrawString(TEXT_X(user_help[0]), 10, user_help[0],font1, White);
				gdispDrawString(190, 150, lives_display[0],	font1, White);
				gdispDrawString(110, 150, dash[0], font1, Yellow);
				gdispDrawString(225, 150, dash_reverse[0], font1, Yellow);

				xQueueReceive(JoystickQueue, &joystick_internal, 0);
				if(joystick_internal.pulse.x == JOYSTICK_PULSE_RIGHT){
					if(starting_lives < 9000){
						starting_lives++;
					}
				}
				else if(joystick_internal.pulse.x == JOYSTICK_PULSE_LEFT){
					if(starting_lives > 0){
						starting_lives--;
					}
				}
				joystick_internal.pulse.x = JOYSTICK_PULSE_NULL;


				if(buttonCount(BUT_A)){
					starting_lives+=10;
				}
				else if(buttonCount(BUT_B)){
					starting_lives-=10;
				}

				break;
			case BACK:
				gdispDrawString(120, 30, single[0],	font1, White);
				gdispDrawString(120, 90, multi[0],	font1, White);
				gdispDrawString(120, 150, settings[0],	font1, White);
				gdispDrawString(120, 210, cheats[0],	font1, Yellow);
				gdispDrawString(TEXT_X(user_help[0]), 10, user_help[0],font1, White);
				gdispDrawString(190, 150, lives_display[0],	font1, White);
				gdispDrawString(110, 210, dash[0], font1, Yellow);
				gdispDrawString(225, 210, dash_reverse[0], font1, Yellow);
				if(buttonCount(BUT_E)){
					menu_select = CLASSIC_3_LIVES;
					xQueueSend(StateQueue, &next_state_signal_menu, 100);
				}
				break;
			}
			if(starting_lives != starting_lives_old){
				xQueueSend(LifeCountQueue1, &starting_lives, 100);
				xQueueSend(LifeCountQueue2, &starting_lives, 100);
				xQueueSend(LifeCountQueue3, &starting_lives, 100);
				starting_lives_old = starting_lives;
			}

		}
	}
}
