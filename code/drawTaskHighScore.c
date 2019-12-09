/*
 * drawTaskHighScore.c
 *
 *  Created on: Dec 9, 2019
 *      Author: lab_espl_stud04
 */

#include "includes.h"
#include "drawTaskHighScore.h"

extern QueueHandle_t StateQueue;
extern QueueHandle_t HighScoresQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;


void drawTaskHighScore(void * params) {
	// Possible next states
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;

	struct joystick_angle_pulse joystick_internal;
	struct highscore highscores;
	char user_help[1][70] = {"HIGH SCORES. Press D to exit."};
	char player_1[1][70] = { 0 };
	char player_2[1][70] = { 0 };
	char player_3[1][70] = { 0 };
	char player_4[1][70] = { 0 };
	char player_5[1][70] = { 0 };
	char player_6[1][70] = { 0 };
	char player_7[1][70] = { 0 };
	char player_8[1][70] = { 0 };
	char player_9[1][70] = { 0 };
	char player_10[1][70] = { 0 };

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			if (buttonCount(BUT_D))
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			xQueueReceive(HighScoresQueue, &highscores, 0);
			sprintf(player_1, "1. %c | %i", highscores.player1_tag, highscores.player1_score);
			sprintf(player_2, "2. %c | %i", highscores.player2_tag, highscores.player2_score);
			sprintf(player_3, "3. %c | %i", highscores.player3_tag, highscores.player3_score);
			sprintf(player_4, "4. %c | %i", highscores.player4_tag, highscores.player4_score);
			sprintf(player_5, "5. %c | %i", highscores.player5_tag, highscores.player5_score);
			sprintf(player_6, "6. %c | %i", highscores.player6_tag, highscores.player6_score);
			sprintf(player_7, "7. %c | %i", highscores.player7_tag, highscores.player7_score);
			sprintf(player_8, "8. %c | %i", highscores.player8_tag, highscores.player8_score);
			sprintf(player_9, "9. %c | %i", highscores.player9_tag, highscores.player9_score);
			sprintf(player_10, "10. %c | %i", highscores.player10_tag, highscores.player10_score);

			gdispClear(Black);

			for(int i = 0; i < 1; i++){
				gdispDrawString(TEXT_X(user_help[i]), 10, user_help[i],font1, White);
				gdispDrawString(TEXT_X(player_1[i]), 25, player_1[i],font1, White);
				gdispDrawString(TEXT_X(player_2[i]), 40, player_2[i],font1, White);
				gdispDrawString(TEXT_X(player_3[i]), 55, player_3[i],font1, White);
				gdispDrawString(TEXT_X(player_4[i]), 70, player_4[i],font1, White);
				gdispDrawString(TEXT_X(player_5[i]), 85, player_5[i],font1, White);
				gdispDrawString(TEXT_X(player_6[i]), 100, player_6[i],font1, White);
				gdispDrawString(TEXT_X(player_7[i]), 115, player_7[i],font1, White);
				gdispDrawString(TEXT_X(player_8[i]), 130, player_8[i],font1, White);
				gdispDrawString(TEXT_X(player_9[i]), 145, player_9[i],font1, White);
				gdispDrawString(TEXT_X(player_10[i]), 160, player_10[i],font1, White);
			}
		}
	}
}
