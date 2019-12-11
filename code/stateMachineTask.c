/*
 * stateMachineTask.c
 *
 *  Created on: Dec 2, 2019
 *      Author: lab_espl_stud04
 */
#include "includes.h"
#include "stateMachineTask.h"

extern QueueHandle_t StateQueue;
extern TaskHandle_t drawTaskStartMenuHandle;
extern TaskHandle_t drawTaskSingleHandle;
extern TaskHandle_t drawTaskPauseHandle;
extern TaskHandle_t drawTaskCheatsHandle;
extern TaskHandle_t drawTaskHighScoreHandle;
extern TaskHandle_t drawTaskHighScoreInterfaceHandle;

void stateMachineTask(void * params) {
	unsigned char current_state = MAIN_MENU_STATE; // Default state
	unsigned char state_changed = 1; // Only re-evaluate state if it has changed
	while (1) {
		if (state_changed)
			goto initial_state;

		// Handle state machine input
		if (xQueueReceive(StateQueue, &current_state, portMAX_DELAY) == pdTRUE) {
			state_changed = 1;
		}
		initial_state:
		// Handle current state
		if (state_changed) {
			switch (current_state) {
			case MAIN_MENU_STATE:
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskPauseHandle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskResume(drawTaskStartMenuHandle);
				state_changed = 0;
				break;
			case SINGLE_PLAYER_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskPauseHandle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskResume(drawTaskSingleHandle);
				state_changed = 0;
				break;
			case PAUSE_MENU_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskResume(drawTaskPauseHandle);
				state_changed = 0;
				break;
			case CHEATS_MENU_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskPauseHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskResume(drawTaskCheatsHandle);
				state_changed = 0;
				break;
			case HIGHSCORE_DISPLAY_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskPauseHandle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskResume(drawTaskHighScoreHandle);
				state_changed = 0;
				break;
			case HIGHSCORE_INTERFACE_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskPauseHandle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskResume(drawTaskHighScoreInterfaceHandle);
				state_changed = 0;
				break;
			default:
				break;
			}
		}
	}
}
