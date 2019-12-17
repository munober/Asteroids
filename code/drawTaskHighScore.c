/*
 * drawTaskHighScore.c
 *
 *  Created on: Dec 9, 2019
 *      Author: lab_espl_stud04
 */

#include "includes.h"
#include "drawTaskHighScore.h"

#define COORD_Y_PRINT				50
#define	LINE_DISTANCE				10
//#define COORD_Y_PRINTLINE(LINE)	 		DISPLAY_SIZE_Y / 2 - (gdispGetFontMetric(font1, fontHeight) * -(LINE + 0.5)) + 85

extern QueueHandle_t StateQueue;
extern QueueHandle_t HighScoresQueue;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;


void drawTaskHighScore(void * params) {
	// Possible next states
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	struct joystick_angle_pulse joystick_internal;
	int i, j; // Iterators, used in a lot of places
	struct highscore new_highscore = { {0} };
	struct highscore highscore_list[5] = { {0} };
	for(i = 0; i < 5; i++){
		highscore_list[i].score = 0;
	}
	char user_help[2][70] = {"HIGH SCORES. Press D to exit.", "Gamer Tag:   |   Score: "};
	char print[6][30];
	int line_y;
	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			if (buttonCount(BUT_D))
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			xQueueReceive(HighScoresQueue, &new_highscore, 0);

			for(i = 4; i >= 0; i--){
				if(new_highscore.score > highscore_list[i].score){
//					for(j = i - 1; j < 5; j++){
//						memcpy(&highscore_list[i+1], &highscore_list[i], sizeof(struct highscore));
//					}
					memcpy(&highscore_list[i], &new_highscore, sizeof(struct highscore));
					i = - 1;
				}
			}

			sprintf(print[0], "1. %s | %i ", highscore_list[0].tag, highscore_list[0].score);
			sprintf(print[1], "2. %s | %i ", highscore_list[1].tag, highscore_list[1].score);
			sprintf(print[2], "3. %s | %i ", highscore_list[2].tag, highscore_list[2].score);
			sprintf(print[3], "4. %s | %i ", highscore_list[3].tag, highscore_list[3].score);
			sprintf(print[4], "5. %s | %i ", highscore_list[4].tag, highscore_list[4].score);
			sprintf(print[5], "x. Baby Yoda | 0 ");

			gdispClear(Black);

			gdispDrawString(TEXT_X(user_help[0]), 10, user_help[0],font1, White);
			gdispDrawString(TEXT_X(user_help[1]), 30, user_help[1],font1, White);
			for(i = 0; i < 5; i++){
				if(highscore_list[i].score != 0){
					gdispDrawString(TEXT_X(print[i]), (50 + 15 * i), print[i],font1, White);
				}
				else{
					gdispDrawString(TEXT_X(print[5]), (50 + 15 * i), print[5],font1, White);
				}
			}

		}
	}
}
