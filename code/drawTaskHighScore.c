/*
 * drawTaskHighScore.c
 *
 *  Created on: Dec 9, 2019
 *      Author: lab_espl_stud04
 */

#include "includes.h"
#include "drawTaskHighScore.h"

#define COORD_X_PRINTLINE			25
#define	LINE_DISTANCE				15

extern QueueHandle_t StateQueue;
extern QueueHandle_t HighScoresQueue;
extern QueueHandle_t LeaderboardQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;


void drawTaskHighScore(void * params) {
	// Possible next states
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	struct joystick_angle_pulse joystick_internal;
	int i, j; // Iterators, used in a lot of places
	struct highscore new_highscore;
	struct highscore highscores[10];
	for(i = 0; i < 10; i++){
		highscores[i].score = 0;
	}
	char user_help[1][70] = {"HIGH SCORES. Press D to exit."};
	char print[10][30];
	int line_x;
	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			if (buttonCount(BUT_D))
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			xQueueReceive(HighScoresQueue, &new_highscore, 0);
			for(i = 0; i < 10; i++){
				if(new_highscore.score > highscores[i].score){
					for(int j = i; j < 9; j++){
						memcpy(&highscores[i+1], &highscores[i], sizeof(struct highscore));
					}
					memcpy(&highscores[i], &new_highscore, sizeof(struct highscore));
				}
			}
			for(i = 0; i < 10; i++){
				sprintf(print[i][30], "1. %c | %i", highscores[i].tag, highscores[i].score);
			}

			gdispClear(Black);
			line_x = COORD_X_PRINTLINE;
			for(i = 0; i < 10; i++){
				gdispDrawString(TEXT_X(user_help), 10, user_help,font1, White);
				if(highscores[i].score != 0){
					gdispDrawString(TEXT_X(print[i]), line_x, print[i],font1, White);
				}
				line_x += LINE_DISTANCE;
			}
		}
	}
}
