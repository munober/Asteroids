/*
 * stateMachineTask.c
 *
 *  Created on: Dec 2, 2019
 *      Author: Teodor Fratiloiu
 *      		Frederik Zumegen
 */
#include "includes.h"
#include "stateMachineTask.h"

extern QueueHandle_t StateQueue;
extern TaskHandle_t drawTaskStartMenuHandle;
extern TaskHandle_t drawTaskSingleHandle;
extern TaskHandle_t drawTaskCheatsHandle;
extern TaskHandle_t drawTaskHighScoreHandle;
extern TaskHandle_t drawTaskHighScoreInterfaceHandle;
extern TaskHandle_t drawTaskMultiplayerHandle;
extern TaskHandle_t drawTaskSingleLevel2Handle;
extern TaskHandle_t drawTaskSingleLevel3Handle;

void stateMachineTask(void * params) {
	unsigned char current_state = STARTING_STATE; // Default state
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
			    vTaskSuspend(drawTaskSingleLevel2Handle);
			    vTaskSuspend(drawTaskSingleLevel3Handle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskSuspend(drawTaskMultiplayerHandle);
			    vTaskResume(drawTaskStartMenuHandle);
				state_changed = 0;
				break;
			case SINGLE_PLAYER_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleLevel2Handle);
			    vTaskSuspend(drawTaskSingleLevel3Handle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskSuspend(drawTaskMultiplayerHandle);
			    vTaskResume(drawTaskSingleHandle);
				state_changed = 0;
				break;
			case CHEATS_MENU_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskSingleLevel2Handle);
			    vTaskSuspend(drawTaskSingleLevel3Handle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskSuspend(drawTaskMultiplayerHandle);
			    vTaskResume(drawTaskCheatsHandle);
				state_changed = 0;
				break;
			case HIGHSCORE_DISPLAY_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskSingleLevel2Handle);
			    vTaskSuspend(drawTaskSingleLevel3Handle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskSuspend(drawTaskMultiplayerHandle);
			    vTaskResume(drawTaskHighScoreHandle);
				state_changed = 0;
				break;
			case HIGHSCORE_INTERFACE_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskSingleLevel2Handle);
			    vTaskSuspend(drawTaskSingleLevel3Handle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskMultiplayerHandle);
			    vTaskResume(drawTaskHighScoreInterfaceHandle);
				state_changed = 0;
				break;
			case MULTIPLAYER_STATE:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskSingleLevel2Handle);
			    vTaskSuspend(drawTaskSingleLevel3Handle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskResume(drawTaskMultiplayerHandle);
				state_changed = 0;
				break;
			case SINGLE_PLAYER_LEVEL_2:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskSingleLevel3Handle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskSuspend(drawTaskMultiplayerHandle);
			    vTaskResume(drawTaskSingleLevel2Handle);
				state_changed = 0;
				break;
			case SINGLE_PLAYER_LEVEL_3:
			    vTaskSuspend(drawTaskStartMenuHandle);
			    vTaskSuspend(drawTaskSingleHandle);
			    vTaskSuspend(drawTaskSingleLevel2Handle);
			    vTaskSuspend(drawTaskCheatsHandle);
			    vTaskSuspend(drawTaskHighScoreHandle);
			    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
			    vTaskSuspend(drawTaskMultiplayerHandle);
			    vTaskResume(drawTaskSingleLevel3Handle);
				state_changed = 0;
				break;
			default:
				break;
			}
		}
	}
}
