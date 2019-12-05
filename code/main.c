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
#include "stateMachineTask.h"
#include "checkJoystickTask.h"

#define STATE_QUEUE_LENGTH 		 1
#define BUTTON_QUEUE_LENGTH		20
#define JOYSTICK_QUEUE_LENGTH	30
#define STACK_SIZE 200

font_t font1;

void frameSwapTask(void * params);
void stateMachineTask(void * params);
void checkButtons(void * params);
void drawTaskStartMenu(void * params);
void drawTaskSingle (void * params);
void drawTaskPause (void * params);
void checkJoystickTask (void * params);

QueueHandle_t ButtonQueue;
QueueHandle_t StateQueue;
QueueHandle_t JoystickQueue;
QueueHandle_t JoystickAngle360Queue;
QueueHandle_t ESPL_RxQueue; // DONT DELETE THIS LINE
SemaphoreHandle_t ESPL_DisplayReady;
SemaphoreHandle_t DrawReady; // After swapping buffer calll drawing

TaskHandle_t frameSwapHandle;
TaskHandle_t stateMachineTaskHandle;
TaskHandle_t drawTaskStartMenuHandle;
TaskHandle_t drawTaskSingleHandle;
TaskHandle_t drawTaskPauseHandle;
TaskHandle_t checkJoystickTaskHandle;

int main(void){

	// Initialize Board functions and graphics
	ESPL_SystemInit();
	font1 = gdispOpenFont("DejaVuSans24*");
	// General
	ButtonQueue = xQueueCreate(BUTTON_QUEUE_LENGTH, sizeof(struct buttons));
	StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));
	JoystickQueue = xQueueCreate(JOYSTICK_QUEUE_LENGTH, sizeof(struct joystick_angle_pulse));
	JoystickAngle360Queue = xQueueCreate(JOYSTICK_QUEUE_LENGTH, sizeof(float));

	ESPL_DisplayReady = xSemaphoreCreateBinary();
	DrawReady = xSemaphoreCreateBinary();

	// Initializes Tasks with their respective priority
	// Core tasks
	xTaskCreate(frameSwapTask, "frameSwapper", 1000, NULL, 5, &frameSwapHandle);
	xTaskCreate(stateMachineTask, "stateMachineTask", 1000, NULL, 3, &stateMachineTaskHandle);
	xTaskCreate(checkButtons, "checkButtons", 1000, NULL, 4, NULL);

	xTaskCreate(drawTaskStartMenu, "drawTaskStartMenu", 1000, NULL, 2, &drawTaskStartMenuHandle);
	xTaskCreate(drawTaskSingle, "drawTaskSingle", 1000, NULL, 2, &drawTaskSingleHandle);
	xTaskCreate(drawTaskPause, "drawTaskPause", 1000, NULL, 2, &drawTaskPauseHandle);

	xTaskCreate(checkJoystickTask, "checkJoystickTask", 1000, NULL, 4, &checkJoystickTaskHandle);

    vTaskSuspend(drawTaskStartMenuHandle);
    vTaskSuspend(drawTaskSingleHandle);
    vTaskSuspend(drawTaskPauseHandle);

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

void checkButtons(void * params) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	struct buttons buttonStatus = { { 0 } };
	const TickType_t PollingRate = 20;

	while (TRUE) {
		// Remember last joystick values
		buttonStatus.joystick.x = (uint8_t)(
				ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		buttonStatus.joystick.y = (uint8_t) 255
				- (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);
		buttonStatus.joystick_direct.x = (uint8_t)(
				ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		buttonStatus.joystick_direct.y = (uint8_t) 255
				- (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);
		buttonStatus.A = 0;
		buttonStatus.B = 0;
		buttonStatus.C = 0;
		buttonStatus.D = 0;
		buttonStatus.E = 0;
		buttonStatus.K = 0;
		xQueueSend(ButtonQueue, &buttonStatus, 0);
		vTaskDelayUntil(&xLastWakeTime, PollingRate);
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

