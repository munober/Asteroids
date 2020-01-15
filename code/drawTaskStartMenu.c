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
	unsigned int game_mode_local = 0; // 0 is single
	unsigned int game_mode_remote = 0; // 0 is single

	struct joystick_angle_pulse joystick_internal;

	char version [1][30] = { 0 }; // Use this variable to print a custom message on the start menu.
	sprintf(version, "Build number -- %i", BUILD_NUMBER);
	char single [1][20] = {"Start game"};
	char multi [1][20] = {"Mode: Singleplayer"};
	char multi_second [1][20] = {"Mode: Multiplayer"};
	char settings [1][20] = {"About"};
	char cheats [1][20] = {"Cheats"};
	char highscores [1][20] = {"High Scores"};
	char dash [1][5] = {">"};
	char dash_reverse [1][5] = {"<"};
	char user_help[1][70];
	char user_help_content[1][70] = {"Navigate with joystick, select with E."};
	char user_help_two[1][70];
	sprintf(user_help[0], " %s", user_help_content);
	char uart_input = 0;
	boolean uart_connected = false;
	boolean is_master = false;
	boolean remote_is_master = false;
	boolean send_master = true;
	const char sync_byte_1 = 253; // when sent: tells remote to be slave
	const char sync_byte_2 = 254; // alive signal, undecided

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			xQueueReceive(ESPL_RxQueue, &uart_input, 0);
			if(uart_input != 0){
				uart_connected = true;
			}
			else if(uart_input == 0){
				uart_connected = false;
				is_master = false;
				remote_is_master = false;
				game_mode_local = 0;
				game_mode_remote = 0;
			}
			if(uart_input == sync_byte_1){
				game_mode_remote = 1;
			}
			else if(uart_input == sync_byte_2){
				game_mode_remote = 0;
			}

			if(game_mode_local == 1){
				UART_SendData(sync_byte_1);
			}
			else if(game_mode_local == 0){
				UART_SendData(sync_byte_2);
			}
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
			gdispDrawString(TEXT_X(version[0]), 230, version[0],font1, White);
			gdispDrawString(TEXT_X(user_help[0]), 10, user_help[0],font1, White);

			if(is_master == true){
				if(remote_is_master == true){
					sprintf(user_help_two, "> Is master, other is master. <");
				}
				else if(remote_is_master == false){
					sprintf(user_help_two, "> Is master, other is slave. <");
				}
			}
			else if(is_master == false){
				if(remote_is_master == true){
					sprintf(user_help_two, "> Is slave, other is master. <");
				}
				else if(remote_is_master == false){
					sprintf(user_help_two, "> Is slave, other is slave. <");
				}
			}
			gdispDrawString(TEXT_X(user_help_two[0]), 220, user_help_two[0], font1, Green);

			switch (menu_select) {
			case SINGLEPLAYER_SELECT:
				gdispDrawString(120, 40, single[0],	font1, Yellow);
				if(game_mode_local == 1 || game_mode_remote == 1){
					gdispDrawString(120, 80, multi_second[0],	font1, White);
				}
				else if(game_mode_local == 0 || game_mode_remote == 0){
					gdispDrawString(120, 80, multi[0],	font1, White);
				}
				gdispDrawString(120, 120, settings[0],	font1, White);
				gdispDrawString(120, 160, cheats[0],	font1, White);
				gdispDrawString(120, 200, highscores[0],	font1, White);
				gdispDrawString(110, 40, dash[0], font1, Yellow);
				gdispDrawString(195, 40, dash_reverse[0], font1, Yellow);
				if (buttonCount(BUT_E)){
					if(game_mode_local == game_mode_remote){
						xQueueSend(StateQueue, &next_state_signal_single, 100);
					}
					if(game_mode_local != game_mode_remote){
						xQueueSend(StateQueue, &next_state_signal_multiplayer, 100);
					}
				}
				break;
			case MULTIPLAYER_SELECT:
					gdispDrawString(120, 40, single[0],	font1, White);
					if(game_mode_local == 1 || game_mode_remote == 1){
						gdispDrawString(120, 80, multi_second[0],	font1, Yellow);
					}
					else if(game_mode_local == 0 || game_mode_remote == 0){
						gdispDrawString(120, 80, multi[0],	font1, Yellow);
					}
					gdispDrawString(120, 120, settings[0],	font1, White);
					gdispDrawString(120, 160, cheats[0],	font1, White);
					gdispDrawString(120, 200, highscores[0],	font1, White);
					gdispDrawString(110, 80, dash[0], font1, Yellow);
//					gdispDrawString(195, 80, dash_reverse[0], font1, Yellow);
					if(buttonCount(BUT_E)){
						if(game_mode_remote == 0){
							game_mode_local = !game_mode_local;
						}
					}
				break;
			case SETTINGS_SELECT:
				gdispDrawString(120, 40, single[0],	font1, White);
				if(game_mode_local == 1 || game_mode_remote == 1){
					gdispDrawString(120, 80, multi_second[0],	font1, White);
				}
				else if(game_mode_local == 0 || game_mode_remote == 0){
					gdispDrawString(120, 80, multi[0],	font1, White);
				}
				gdispDrawString(120, 120, settings[0],	font1, Yellow);
				gdispDrawString(120, 160, cheats[0],	font1, White);
				gdispDrawString(120, 200, highscores[0],	font1, White);
				gdispDrawString(110, 120, dash[0], font1, Yellow);
				gdispDrawString(195, 120, dash_reverse[0], font1, Yellow);
				break;
			case CHEATS_SELECT:
				gdispDrawString(120, 40, single[0],	font1, White);
				if(game_mode_local == 1 || game_mode_remote == 1){
					gdispDrawString(120, 80, multi_second[0],	font1, White);
				}
				else if(game_mode_local == 0 || game_mode_remote == 0){
					gdispDrawString(120, 80, multi[0],	font1, White);
				}
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
				if(game_mode_local == 1 || game_mode_remote == 1){
					gdispDrawString(120, 80, multi_second[0],	font1, White);
				}
				else if(game_mode_local == 0 || game_mode_remote == 0){
					gdispDrawString(120, 80, multi[0],	font1, White);
				}
				gdispDrawString(120, 120, settings[0],	font1, White);
				gdispDrawString(120, 160, cheats[0],	font1, White);
				gdispDrawString(120, 200, highscores[0],	font1, Yellow);
				gdispDrawString(110, 200, dash[0], font1, Yellow);
				gdispDrawString(195, 200, dash_reverse[0], font1, Yellow);
				if (buttonCount(BUT_E))
					xQueueSend(StateQueue, &next_state_signal_highscores, 100);
				break;
			}
			if(game_mode_local == 1 && game_mode_remote == 0){
				is_master = true;
				remote_is_master = false;
			}
			else if(game_mode_local == 0 && game_mode_remote == 0){
				is_master = false;
				remote_is_master = false;
			}
			else if(game_mode_local == 0 && game_mode_remote == 1){
				is_master = false;
				remote_is_master = true;
			}
			else if(game_mode_local == 1 && game_mode_remote == 1){
				is_master = false;
				remote_is_master = false;
				game_mode_local = 0;
			}
			uart_input = 0;
		}
	}
}
