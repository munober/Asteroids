#include "includes.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "drawTaskStartMenu.h"
#include "drawTaskSingle.h"
#include "drawTaskPause.h"
#include "drawTaskCheats.h"
#include "stateMachineTask.h"
#include "checkJoystickTask.h"
#include "drawTaskHighScore.h"
#include "drawTaskHighScoreInterface.h"
#include "drawTaskMultiplayer.h"
#include "uartTask.h"

#define STATE_QUEUE_LENGTH 		 1
#define BUTTON_QUEUE_LENGTH		20
#define JOYSTICK_QUEUE_LENGTH	30
#define STACK_SIZE				200

font_t font1;

// Function declarations for tasks
void frameSwapTask(void * params);
void stateMachineTask(void * params);
void drawTaskStartMenu(void * params);
void drawTaskSingle (void * params);
void drawTaskMultiplayer (void * params);
void drawTaskPause (void * params);
void drawTaskCheats(void * params);
void drawTaskHighScore (void * params);
void drawTaskHighScoreInterface (void * params);
void checkJoystickTask (void * params);
void uartTask (void * params);
void drawTaskSingleLevel2 (void * params);
void drawTaskSingleLevel3 (void * params);
void drawTaskPauseLevel2 (void * params);
void drawTaskPauseLevel3 (void * params);

//QueueHandle_t ButtonQueue;
QueueHandle_t StateQueue;
QueueHandle_t JoystickQueue;
QueueHandle_t JoystickAngle360Queue;
QueueHandle_t PlayerNavigationQueue;
QueueHandle_t LifeCountQueue1;
QueueHandle_t LifeCountQueue2;
QueueHandle_t LifeCountQueue3;
QueueHandle_t HighScoresQueue;
QueueHandle_t ESPL_RxQueue; // DONT DELETE THIS LINE
SemaphoreHandle_t ESPL_DisplayReady;

SemaphoreHandle_t DrawReady;
TaskHandle_t frameSwapHandle;
TaskHandle_t stateMachineTaskHandle;
TaskHandle_t drawTaskStartMenuHandle;
TaskHandle_t drawTaskSingleHandle;
TaskHandle_t drawTaskMultiplayerHandle;
TaskHandle_t drawTaskPauseHandle;
TaskHandle_t drawTaskCheatsHandle;
TaskHandle_t checkJoystickTaskHandle;
TaskHandle_t drawTaskHighScoreHandle;
TaskHandle_t drawTaskHighScoreInterfaceHandle;
TaskHandle_t uartTaskHandle;
TaskHandle_t drawTaskSingleLevel2Handle;
TaskHandle_t drawTaskSingleLevel3Handle;
TaskHandle_t drawTaskPauseLevel2Handle;
TaskHandle_t drawTaskPauseLevel3Handle;

int main(void){

	// Initialize Board functions and graphics
	ESPL_SystemInit();
	font1 = gdispOpenFont("DejaVuSans24*");

//	Creating ALL Queues
	StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));
	JoystickQueue = xQueueCreate(JOYSTICK_QUEUE_LENGTH, sizeof(struct joystick_angle_pulse));
	JoystickAngle360Queue = xQueueCreate(JOYSTICK_QUEUE_LENGTH, sizeof(float));
	PlayerNavigationQueue = xQueueCreate(JOYSTICK_QUEUE_LENGTH, sizeof(struct coord));
	LifeCountQueue1 = xQueueCreate(10, sizeof(unsigned int));
	LifeCountQueue2 = xQueueCreate(10, sizeof(unsigned int));
	LifeCountQueue3 = xQueueCreate(10, sizeof(unsigned int));
	HighScoresQueue = xQueueCreate(10, sizeof(struct highscore));

	ESPL_DisplayReady = xSemaphoreCreateBinary();
	DrawReady = xSemaphoreCreateBinary();

	// Initializes Tasks with their respective priority
	// Core tasks

//	Creating ALL Tasks
	xTaskCreate(frameSwapTask, "frameSwapper", 1000, NULL, 5, &frameSwapHandle);
	xTaskCreate(stateMachineTask, "stateMachineTask", 1000, NULL, 3, &stateMachineTaskHandle);

	xTaskCreate(drawTaskStartMenu, "drawTaskStartMenu", 1000, NULL, 2, &drawTaskStartMenuHandle);
	xTaskCreate(drawTaskSingle, "drawTaskSingle", 1000, NULL, 2, &drawTaskSingleHandle);
	xTaskCreate(drawTaskSingleLevel2, "drawTaskSingleLevel2", 1000, NULL, 3, &drawTaskSingleLevel2Handle);
	xTaskCreate(drawTaskSingleLevel3, "drawTaskSingleLevel3", 2000, NULL, 3, &drawTaskSingleLevel3Handle);
	xTaskCreate(drawTaskMultiplayer, "drawTaskMultiplayer", 1000, NULL, 2, &drawTaskMultiplayerHandle);
	xTaskCreate(drawTaskPause, "drawTaskPause", 1000, NULL, 2, &drawTaskPauseHandle);
	xTaskCreate(drawTaskCheats, "drawTaskCheats", 1000, NULL, 2, &drawTaskCheatsHandle);
	xTaskCreate(drawTaskHighScore, "drawTaskHighScore", 1000, NULL, 4, &drawTaskHighScoreHandle);
	xTaskCreate(drawTaskHighScoreInterface, "drawTaskHighScoreInterface", 1000, NULL, 2, &drawTaskHighScoreInterfaceHandle);
	xTaskCreate(checkJoystickTask, "checkJoystickTask", 1000, NULL, 4, &checkJoystickTaskHandle);
	xTaskCreate(uartTask, "uartTask", 1000, NULL, 3, &uartTaskHandle);
	xTaskCreate(drawTaskPauseLevel2, "drawTaskPauseLevel2", 1000, NULL, 2, &drawTaskPauseLevel2Handle);
	xTaskCreate(drawTaskPauseLevel3, "drawTaskPauseLevel3", 1000, NULL, 2, &drawTaskPauseLevel3Handle);

//	Suspending ALL tasks that draw to the screen, will be handled by state machine.

    vTaskSuspend(drawTaskStartMenuHandle);
    vTaskSuspend(drawTaskSingleHandle);
    vTaskSuspend(drawTaskSingleLevel2Handle);
    vTaskSuspend(drawTaskSingleLevel3Handle);
    vTaskSuspend(drawTaskMultiplayerHandle);
    vTaskSuspend(drawTaskPauseHandle);
    vTaskSuspend(drawTaskCheatsHandle);
    vTaskSuspend(drawTaskHighScoreHandle);
    vTaskSuspend(drawTaskHighScoreInterfaceHandle);
    vTaskSuspend(uartTaskHandle);
    vTaskSuspend(drawTaskPauseLevel2Handle);
    vTaskSuspend(drawTaskPauseLevel3Handle);

	vTaskStartScheduler();
}

// Keeping the frame swapping task here

void frameSwapTask(void * params) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t frameratePeriod = 20;

	while (1){
		// Draw next frame
		xSemaphoreGive(DrawReady);
		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// Swap buffers
		ESPL_DrawLayer();
		vTaskDelayUntil(&xLastWakeTime, frameratePeriod);
	}
}

void vApplicationIdleHook() {
	while (TRUE) {
	};
}

void vApplicationMallocFailedHook() {
	while (TRUE) {
	};
}
