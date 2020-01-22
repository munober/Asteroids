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
extern QueueHandle_t LocalMasterQueue;
extern QueueHandle_t StartingScoreQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;

#define SINGLEPLAYER_SELECT			1
#define MULTIPLAYER_SELECT			2
#define SETTINGS_SELECT				3
#define CHEATS_SELECT				4
#define HIGHSCORES_SELECT			5

void drawTaskStartMenu(void * params) {
//  Possible next states
	const unsigned char next_state_signal_single_one = SINGLE_PLAYER_STATE;
	const unsigned char next_state_signal_single_two = SINGLE_PLAYER_LEVEL_2;
	const unsigned char next_state_signal_single_three = SINGLE_PLAYER_LEVEL_3;

	const unsigned char next_state_signal_cheats = CHEATS_MENU_STATE;
	const unsigned char next_state_signal_highscores = HIGHSCORE_DISPLAY_STATE;
	const unsigned char next_state_signal_multiplayer = MULTIPLAYER_STATE;
	unsigned int menu_select = SINGLEPLAYER_SELECT;
	unsigned int game_mode_local = 0; // 0 is single
	unsigned int game_mode_remote = 0; // 0 is single
	unsigned int starting_level = 1;
	int16_t starting_score = 0;

	struct joystick_angle_pulse joystick_internal;

	char version [1][30] = { 0 }; // Use this variable to print a custom message on the start menu.
	sprintf(version, "Build number -- %i", BUILD_NUMBER);
	char single [1][20] = {"Start game"};
	char multi [1][20] = { {0} };
	char multi_second [1][20] = { {0} };
	sprintf(multi, "Mode: Singleplayer");
	sprintf(multi_second, "Mode: Multiplayer");
	char settings [1][30] = { {0} };
	sprintf(settings, "Starting score: %i", starting_score);
	char settings_2 [1][20] = { {0} };
	sprintf(settings_2, "Starting level: %d", starting_level);
	char cheats [1][20] = {"Starting lives"};
	char highscores [1][20] = {"High Scores"};
	char dash [1][5] = {">"};
	char dash_reverse [1][5] = {"<"};
	char user_help[1][70];
	char user_help_content[1][70] = {"Navigate with joystick, select with E."};
	char user_help_two[1][70];
	sprintf(user_help[0], "%s", user_help_content);
	char uart_input = 0;
	boolean uart_connected = false;
	boolean is_master = false;
	boolean remote_is_master = false;
	const char sync_byte_1 = 253; // when sent: tells remote to be slave
	const char sync_byte_2 = 254; // alive signal
	int game_starting = 0;

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

			if(game_starting == 0){
				if(game_mode_local == 1){
					UART_SendData(sync_byte_1);
				}
				else if(game_mode_local == 0){
					UART_SendData(sync_byte_2);
				}
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
			if(game_starting == 0){
				if(SHOW_DEBUG_MAINMENU){
					gdispDrawString(TEXT_X(version[0]), 230, version[0],font1, White);
				}
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
								sprintf(multi_second, "Mode: Multiplayer");
								gdispDrawString(120, 40, single[0],	font1, Yellow);
								if(game_mode_local == 1 || game_mode_remote == 1){
									gdispDrawString(120, 80, multi_second[0],	font1, White);
								}
								else if(game_mode_local == 0 || game_mode_remote == 0){
									gdispDrawString(120, 80, multi[0],	font1, White);
								}
								gdispDrawString(120, 120, settings[0],	font1, White);
								gdispDrawString(120, 130, settings_2[0], font1, White);
								gdispDrawString(120, 160, cheats[0],	font1, White);
								gdispDrawString(120, 200, highscores[0],	font1, White);
								gdispDrawString(110, 40, dash[0], font1, Yellow);
								gdispDrawString(195, 40, dash_reverse[0], font1, Yellow);
								if (buttonCount(BUT_E)){
									if(game_mode_local == game_mode_remote){
										xQueueSend(StartingScoreQueue, &starting_score, 0);
										switch(starting_level){
										case 1:
											xQueueSend(StateQueue, &next_state_signal_single_one, 100);
											break;
										case 2:
											xQueueSend(StateQueue, &next_state_signal_single_two, 100);
											break;
										case 3:
											xQueueSend(StateQueue, &next_state_signal_single_three, 100);
											break;
										}

									}
									if(game_mode_local != game_mode_remote){
										xQueueSend(StartingScoreQueue, &starting_score, 0);
										xQueueSend(LocalMasterQueue, &is_master, 0);
										xQueueSend(StateQueue, &next_state_signal_multiplayer, 100);
									}
								}

								break;
							case MULTIPLAYER_SELECT:
								sprintf(multi_second, "Mode: Multiplayer");
									gdispDrawString(120, 40, single[0],	font1, White);
									if(game_mode_local == 1 || game_mode_remote == 1){
										gdispDrawString(120, 80, multi_second[0],	font1, Yellow);
									}
									else if(game_mode_local == 0 || game_mode_remote == 0){
										gdispDrawString(120, 80, multi[0],	font1, Yellow);
									}
									gdispDrawString(120, 120, settings[0],	font1, White);
									gdispDrawString(120, 130, settings_2[0],	font1, White);
									gdispDrawString(120, 160, cheats[0],	font1, White);
									gdispDrawString(120, 200, highscores[0],	font1, White);
									gdispDrawString(110, 80, dash[0], font1, Yellow);
									if(buttonCount(BUT_E)){
										if(game_mode_remote == 0){
											game_mode_local = !game_mode_local;
										}
									}
								break;
							case SETTINGS_SELECT:
								sprintf(multi_second, "Mode: Multiplayer");
								gdispDrawString(120, 40, single[0],	font1, White);
								if(game_mode_local == 1 || game_mode_remote == 1){
									gdispDrawString(120, 80, multi_second[0],	font1, White);
								}
								else if(game_mode_local == 0 || game_mode_remote == 0){
									gdispDrawString(120, 80, multi[0],	font1, White);
								}
								if(buttonCount(BUT_E)){
									if(starting_score < 20000)
										starting_score += START_MENU_SCORE_INCREMENT;
									else if (starting_score == 20000)
										starting_score = 0;
								}
								xQueueReceive(JoystickQueue, &joystick_internal, 0);
								if(joystick_internal.pulse.x == JOYSTICK_PULSE_RIGHT){
									if(starting_score < 20000)
										starting_score += START_MENU_SCORE_INCREMENT;
									else if (starting_score == 20000)
										starting_score = 0;
								}
								if(joystick_internal.pulse.x == JOYSTICK_PULSE_LEFT){
									if(starting_score > 0)
										starting_score -= START_MENU_SCORE_INCREMENT;
									else if (starting_score == 0)
										starting_score = 20000;
								}
								joystick_internal.pulse.x = JOYSTICK_PULSE_NULL;
								sprintf(settings, "Starting score: %i", starting_score);
								gdispDrawString(120, 120, settings[0],	font1, Yellow);

								if(starting_score < LEVEL_TWO_SCORE_THRESHOLD){
									starting_level = 1;
								}
								else if(starting_score >= LEVEL_TWO_SCORE_THRESHOLD && starting_score < LEVEL_THREE_SCORE_THRESHOLD){
									starting_level = 2;
								}
								else if(starting_score >= LEVEL_THREE_SCORE_THRESHOLD && starting_score < BEAT_GAME_SCORE_THRESHOLD){
									starting_level = 3;
								}

								sprintf(settings_2, "Starting level: %d", starting_level);
								gdispDrawString(120, 130, settings_2[0],	font1, Yellow);

								gdispDrawString(120, 160, cheats[0],	font1, White);
								gdispDrawString(120, 200, highscores[0],	font1, White);
								gdispDrawString(110, 120, dash[0], font1, Yellow);
//								gdispDrawString(195, 120, dash_reverse[0], font1, Yellow);
								break;
							case CHEATS_SELECT:
								sprintf(multi_second, "Mode: Multiplayer");
								gdispDrawString(120, 40, single[0],	font1, White);
								if(game_mode_local == 1 || game_mode_remote == 1){
									gdispDrawString(120, 80, multi_second[0],	font1, White);
								}
								else if(game_mode_local == 0 || game_mode_remote == 0){
									gdispDrawString(120, 80, multi[0],	font1, White);
								}
								gdispDrawString(120, 120, settings[0],	font1, White);
								gdispDrawString(120, 130, settings_2[0],	font1, White);
								gdispDrawString(120, 160, cheats[0],	font1, Yellow);
								gdispDrawString(120, 200, highscores[0],	font1, White);
								gdispDrawString(110, 160, dash[0], font1, Yellow);
								if (buttonCount(BUT_E))
									xQueueSend(StateQueue, &next_state_signal_cheats, 100);
								break;
							case HIGHSCORES_SELECT:
								sprintf(multi_second, "Mode: Multiplayer");
								gdispDrawString(120, 40, single[0],	font1, White);
								if(game_mode_local == 1 || game_mode_remote == 1){
									gdispDrawString(120, 80, multi_second[0],	font1, White);
								}
								else if(game_mode_local == 0 || game_mode_remote == 0){
									gdispDrawString(120, 80, multi[0],	font1, White);
								}
								gdispDrawString(120, 120, settings[0],	font1, White);
								gdispDrawString(120, 130, settings_2[0],	font1, White);
								gdispDrawString(120, 160, cheats[0],	font1, White);
								gdispDrawString(120, 200, highscores[0],	font1, Yellow);
								gdispDrawString(110, 200, dash[0], font1, Yellow);
								gdispDrawString(195, 200, dash_reverse[0], font1, Yellow);
								if (buttonCount(BUT_E))
									xQueueSend(StateQueue, &next_state_signal_highscores, 100);
								break;
							}
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
