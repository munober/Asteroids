/*
 * drawTaskHighScore.c
 *
 *  Created on: Dec 9, 2019
 *      Author: Teodor Fratiloiu
 */

#include "includes.h"
#include "drawTaskHighScore.h"

#define COORD_Y_PRINT				50
#define	LINE_DISTANCE				10
//#define COORD_Y_PRINTLINE(LINE)	 		DISPLAY_SIZE_Y / 2 - (gdispGetFontMetric(font1, fontHeight) * -(LINE + 0.5)) + 85

extern QueueHandle_t StateQueue;
extern QueueHandle_t HighScoresQueue;
extern QueueHandle_t HighScoresQueueMP;
extern font_t font1;
extern SemaphoreHandle_t DrawReady;


void drawTaskHighScore(void * params) {
	// Possible next states
	TickType_t receive_time;
	receive_time = xTaskGetTickCount();
	TickType_t delay;
	delay = 1000;
	const unsigned char next_state_signal_menu = MAIN_MENU_STATE;
	int i; // Iterators, used in a lot of places
	uint16_t position = 0;
	struct highscore new_highscore = { {0} };
	struct highscore new_highscore_mp = { {0} };
	struct highscore empty_highscore = { {0} };
	empty_highscore.score = 0;
	struct highscore highscore_list[5] = { {0} };
	for(i = 0; i < 5; i++){
		highscore_list[i].score = 0;
	}
	struct highscore highscore_list_mp[5] = { {0} };
	for(i = 0; i < 5; i++){
		highscore_list_mp[i].score = 0;
	}
	char user_help[3][70] = {"HIGH SCORES. Press D to exit.", "SINGLEPLAYER", "MULTIPLAYER"};
	char print[6][30];
	char print_mp[6][30];
	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			if (buttonCount(BUT_D))
				xQueueSend(StateQueue, &next_state_signal_menu, 100);
			if(xQueueReceive(HighScoresQueue, &new_highscore, 0) == pdTRUE){
				receive_time = xTaskGetTickCount();
			}
			if(xQueueReceive(HighScoresQueueMP, &new_highscore_mp, 0) == pdTRUE){
				receive_time = xTaskGetTickCount();
			}

			if(new_highscore.score != 0){
				for(position = 0; position < 5; position++){
					if(new_highscore.score > highscore_list[position].score){
						break;
					}
				}
				
				for(i = 3; i >= position; i--){
					memcpy(&highscore_list[i+1], &highscore_list[i], sizeof(struct highscore));
				}
				memcpy(&highscore_list[position], &new_highscore, sizeof(struct highscore));
				memcpy(&new_highscore, &empty_highscore, sizeof(struct highscore));
			}

			if(new_highscore_mp.score != 0){
				for(position = 0; position < 5; position++){
					if(new_highscore_mp.score > highscore_list_mp[position].score){
						break;
					}
				}

				for(i = 3; i >= position; i--){
					memcpy(&highscore_list_mp[i+1], &highscore_list_mp[i], sizeof(struct highscore));
				}
				memcpy(&highscore_list_mp[position], &new_highscore_mp, sizeof(struct highscore));
				memcpy(&new_highscore_mp, &empty_highscore, sizeof(struct highscore));
			}

			sprintf(print[0], "1. %s | %i ", highscore_list[0].tag, highscore_list[0].score);
			sprintf(print[1], "2. %s | %i ", highscore_list[1].tag, highscore_list[1].score);
			sprintf(print[2], "3. %s | %i ", highscore_list[2].tag, highscore_list[2].score);
			sprintf(print[3], "4. %s | %i ", highscore_list[3].tag, highscore_list[3].score);
			sprintf(print[4], "5. %s | %i ", highscore_list[4].tag, highscore_list[4].score);
			sprintf(print[5], "x. Baby Yoda | 0 ");
			sprintf(print_mp[0], "1. %i | %i ", highscore_list_mp[0].score, highscore_list_mp[0].score_remote);
			sprintf(print_mp[1], "2. %i | %i ", highscore_list_mp[1].score, highscore_list_mp[1].score_remote);
			sprintf(print_mp[2], "3. %i | %i ", highscore_list_mp[2].score, highscore_list_mp[2].score_remote);
			sprintf(print_mp[3], "4. %i | %i ", highscore_list_mp[3].score, highscore_list_mp[3].score_remote);
			sprintf(print_mp[4], "5. %i | %i ", highscore_list_mp[4].score, highscore_list_mp[4].score_remote);
			sprintf(print_mp[5], "x. Baby Yoda | 0 ");

			// Display printing
			gdispClear(Black);
			gdispDrawString(TEXT_X(user_help[0]), 10, user_help[0],font1, White);
			gdispDrawString(TEXT_X(user_help[1]), 30, user_help[1],font1, White);
			gdispDrawString(TEXT_X(user_help[2]), 130, user_help[2],font1, Yellow);
			for(i = 0; i < 5; i++){
				if(highscore_list[i].score != 0){
					gdispDrawString(TEXT_X(print[i]), (40 + 10 * i), print[i],font1, White);
				}
			}

			for(i = 0; i < 5; i++){
				if(highscore_list_mp[i].score != 0){
					gdispDrawString(TEXT_X(print_mp[i]), (140 + 10 * i), print_mp[i],font1, Yellow);
				}
			}
		}
	}
}
