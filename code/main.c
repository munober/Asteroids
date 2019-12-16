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

#define STATE_QUEUE_LENGTH 		 1
#define BUTTON_QUEUE_LENGTH		20
#define JOYSTICK_QUEUE_LENGTH	30
#define STACK_SIZE				200

font_t font1;

void frameSwapTask(void * params);
void stateMachineTask(void * params);

void drawTaskStartMenu(void * params);
void drawTaskSingle (void * params);
void timer(void * params);
void drawTaskPause (void * params);
void drawTaskCheats(void * params);
void drawTaskHighScore (void * params);
void drawTaskHighScoreInterface (void * params);

void checkJoystickTask (void * params);

//QueueHandle_t ButtonQueue;
QueueHandle_t StateQueue;
QueueHandle_t JoystickQueue;
QueueHandle_t JoystickAngle360Queue;
QueueHandle_t PlayerNavigationQueue;
QueueHandle_t LifeCountQueue;
QueueHandle_t HighScoresQueue;
QueueHandle_t ESPL_RxQueue; // DONT DELETE THIS LINE
SemaphoreHandle_t ESPL_DisplayReady;
SemaphoreHandle_t DrawReady; // After swapping buffer calll drawing
SemaphoreHandle_t timerSignal;

TaskHandle_t frameSwapHandle;
TaskHandle_t stateMachineTaskHandle;
TaskHandle_t drawTaskStartMenuHandle;
TaskHandle_t drawTaskSingleHandle;
TaskHandle_t drawTaskPauseHandle;
TaskHandle_t drawTaskCheatsHandle;
TaskHandle_t checkJoystickTaskHandle;
TaskHandle_t drawTaskHighScoreHandle;
TaskHandle_t drawTaskHighScoreInterfaceHandle;
TaskHandle_t timerHandle;

int main(void){

	// Initialize Board functions and graphics
	ESPL_SystemInit();
	font1 = gdispOpenFont("DejaVuSans24*");
	// General
	StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));
	JoystickQueue = xQueueCreate(JOYSTICK_QUEUE_LENGTH, sizeof(struct joystick_angle_pulse));
	JoystickAngle360Queue = xQueueCreate(JOYSTICK_QUEUE_LENGTH, sizeof(float));
	PlayerNavigationQueue = xQueueCreate(JOYSTICK_QUEUE_LENGTH, sizeof(struct coord));
	LifeCountQueue = xQueueCreate(10, sizeof(unsigned int));
	HighScoresQueue = xQueueCreate(10, sizeof(struct highscore));

	ESPL_DisplayReady = xSemaphoreCreateBinary();
	DrawReady = xSemaphoreCreateBinary();

	timerSignal = xSemaphoreCreateBinary();

	// Initializes Tasks with their respective priority
	// Core tasks
	xTaskCreate(frameSwapTask, "frameSwapper", 1000, NULL, 5, &frameSwapHandle);
	xTaskCreate(stateMachineTask, "stateMachineTask", 1000, NULL, 3, &stateMachineTaskHandle);

	xTaskCreate(drawTaskStartMenu, "drawTaskStartMenu", 1000, NULL, 2, &drawTaskStartMenuHandle);
	xTaskCreate(drawTaskSingle, "drawTaskSingle", 1000, NULL, 2, &drawTaskSingleHandle);
	xTaskCreate(timer, "Timer", 1000, NULL, 2, &timerHandle);
	xTaskCreate(drawTaskPause, "drawTaskPause", 1000, NULL, 2, &drawTaskPauseHandle);
	xTaskCreate(drawTaskCheats, "drawTaskCheats", 1000, NULL, 2, &drawTaskCheatsHandle);
	xTaskCreate(drawTaskHighScore, "drawTaskHighScore", 1000, NULL, 2, &drawTaskHighScoreHandle);
	xTaskCreate(drawTaskHighScoreInterface, "drawTaskHighScoreInterface", 1000, NULL, 2, &drawTaskHighScoreInterfaceHandle);

	xTaskCreate(checkJoystickTask, "checkJoystickTask", 1000, NULL, 4, &checkJoystickTaskHandle);

    vTaskSuspend(drawTaskStartMenuHandle);
    vTaskSuspend(drawTaskSingleHandle);
    vTaskSuspend(drawTaskPauseHandle);
    vTaskSuspend(drawTaskCheatsHandle);
    vTaskSuspend(drawTaskHighScoreHandle);
    vTaskSuspend(drawTaskHighScoreInterfaceHandle);

	vTaskStartScheduler();
}

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

