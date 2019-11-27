/**
 *
 * @author: Alex Hoffman alex.hoffman@tum.de (RCS, TUM)
 * 			Jonathan M��ller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 */
#include "includes.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DISPLAY_SIZE_X  		320
#define DISPLAY_SIZE_Y  		240

#define TEXT_X(TEXT)	 		DISPLAY_SIZE_X / 2 - (gdispGetStringWidth(TEXT, font1) / 2)
#define TEXT_Y(LINE)	 		DISPLAY_SIZE_Y / 2 - (gdispGetFontMetric(font1, fontHeight) * -(LINE + 0.5)) + 65

#define SLIDING_TEXT_X(TEXT)	DISPLAY_SIZE_X / 2 - (gdispGetStringWidth(TEXT, font1) / 2) + sliderCounter() - 100
#define SLIDING_TEXT_Y(LINE)	DISPLAY_SIZE_Y / 2 - (gdispGetFontMetric(font1, fontHeight) * -(LINE + 0.5)) - 100

#define PI						 3.14159265
#define RADIAN_CONVERSION		 PI / 180
#define ROTATION_RADIUS_CIRCLE	 40
#define ROTATING_CIRCLE_X		 DISPLAY_SIZE_X / 2 + ROTATION_RADIUS_CIRCLE * cos(circleRotate() * RADIAN_CONVERSION)
#define ROTATING_CIRCLE_Y		 DISPLAY_SIZE_Y / 2 - 5 + ROTATION_RADIUS_CIRCLE * sin(circleRotate() * RADIAN_CONVERSION)

#define ROTATION_RADIUS_SQUARE	 40
#define ROTATING_SQUARE_X		 DISPLAY_SIZE_X / 2	+ ROTATION_RADIUS_SQUARE * cos((circleRotate() + 180) * RADIAN_CONVERSION)
#define ROTATING_SQUARE_Y		 DISPLAY_SIZE_Y / 2 + ROTATION_RADIUS_SQUARE * sin((circleRotate() + 180) * RADIAN_CONVERSION)

#define SCREEN_SLIDE_X			 buttonStatus.joystick.x / 8
#define SCREEN_SLIDE_Y			 buttonStatus.joystick.y / 8

#define SCREEN_SLIDE_X_UART	 	 move_uart_x / 8
#define SCREEN_SLIDE_Y_UART	 	 move_uart_y / 8

#define BUTTON_QUEUE_LENGTH 	 20
#define STATE_QUEUE_LENGTH 		 1
#define FPS_QUEUE_LENGTH 		 20

#define STATE_COUNT 3
#define STATE_ONE   1
#define STATE_TWO   2
#define STATE_THREE 3

#define NEXT_TASK   1
#define PREV_TASK   2
#define NUM_POINTS	(sizeof(triangle)/sizeof(triangle[0]))

#define STACK_SIZE 200
#define RESET_TIMER_EX_3	15	//In seconds

// Start and stop bytes for the UART protocol (READ-ONLY)
static const uint8_t startByte = 0xAA, stopByte = 0x55;

// Coordinates for the triangle we are drawing in exercise 1 (READ-ONLY)
static const point triangle[] = {
		{ -15, 0 },
		{  0 , -20 },
		{  15, 0 },
};

// Load font for ugfx
font_t font1;

// Function prototypes
// General
void frameSwapTask(void * params);
void basicStateMachine(void * params);
void checkButtons(void * params);
void exercise1Display(void * params);
void exercise2Display(void * params);
void exercise3Display(void * params);

// Exercise 3.2.2
void circleBlinkDynamic(void *params);
void circleBlinkStatic(void * params);

// Exercise 3.2.3
void timesButtonAPressed(void * params);
void timesButtonBPressed(void * params);
void resetTimer(void * params);

// Exercise 3.2.4
void increaseVariable(void * params);

// Exercise 3.2.5
void sendPositionUART(void * params);
void receivePositionUART(void * params);

// Exercise 3.3
void ex4task1(void * params);
void ex4task2(void * params);
void ex4task3(void * params);
void ex4task4(void * params);

// General
QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;
SemaphoreHandle_t DrawReady; // After swapping buffer calll drawing

// Exercise 3.2.2
SemaphoreHandle_t BlinkCircle2Hz;
SemaphoreHandle_t BlinkCircle1Hz;

// Exercise 3.2.3
SemaphoreHandle_t ButtonAPressed;
SemaphoreHandle_t ButtonBPressed;
SemaphoreHandle_t ResetTimer;

// Exercise 3.2.4
SemaphoreHandle_t increaseVariable_Semaphore;

// General
QueueHandle_t ButtonQueue;
QueueHandle_t StateQueue;
TaskHandle_t frameSwapHandle;
TaskHandle_t exercise1DisplayHandle;
TaskHandle_t exercise2DisplayHandle;
TaskHandle_t exercise3DisplayHandle;
TaskHandle_t stateMachineHandle;

//Exercise 3.2.2
TaskHandle_t circleBlinkDynamicHandle;
StaticTask_t xTaskBuffer;
StackType_t xStack[ STACK_SIZE ];
QueueHandle_t FPSQueue;

//Exercise 3.2.3
TaskHandle_t timesButtonAPressedHandle;
TaskHandle_t timesButtonBPressedHandle;
TaskHandle_t resetTimerHandle;
QueueHandle_t resetTimerQueue;

//Exercise 3.2.4
TaskHandle_t increaseVariableHandle;

// Exercise 3.2.5
TaskHandle_t sendPositionUARTHandle;
TaskHandle_t receivePositionUARTHandle;
QueueHandle_t UARTjoystickQueue;
locked_buttons_t buttonCounts;
locked_joystick_t joystickUart;
locked_string_t printString;

// Exercise 3.3
TaskHandle_t ex4task1Handle;
TaskHandle_t ex4task2Handle;
TaskHandle_t ex4task3Handle;
TaskHandle_t ex4task4Handle;
QueueHandle_t ex4TaskQueue;
SemaphoreHandle_t	start_task3;


int main(void){

	// Initialize Board functions and graphics
	ESPL_SystemInit();
	font1 = gdispOpenFont("DejaVuSans24*");
	// General
	ButtonQueue = xQueueCreate(BUTTON_QUEUE_LENGTH, sizeof(struct buttons));
	StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));

	ESPL_DisplayReady = xSemaphoreCreateBinary();
	DrawReady = xSemaphoreCreateBinary();

	// UART buttons counting for 3.2.5
	buttonCounts = initButtons();
	joystickUart = initJoystick();
	printString = initString();

	// Exercise 3.2.2
	BlinkCircle2Hz = xSemaphoreCreateBinary();
	BlinkCircle1Hz = xSemaphoreCreateBinary();
	FPSQueue = xQueueCreate(FPS_QUEUE_LENGTH, sizeof(uint8_t));

	// Exercise 3.2.3
	ButtonAPressed = xSemaphoreCreateBinary();
	ButtonBPressed = xSemaphoreCreateBinary();
	ResetTimer = xSemaphoreCreateBinary();
	resetTimerQueue = xQueueCreate(FPS_QUEUE_LENGTH, sizeof(unsigned char));

	// Exercise 3.2.4
	increaseVariable_Semaphore = xSemaphoreCreateBinary();

	// Initializes Tasks with their respective priority
	// Core tasks
	xTaskCreate(frameSwapTask, "frameSwapper", 1000, NULL, 5, &frameSwapHandle);
	xTaskCreate(basicStateMachine, "StateMachine", 1000, NULL, 3, &stateMachineHandle);
	xTaskCreate(checkButtons, "checkButtons", 1000, NULL, 4, NULL);

	// Display tasks for the different exercises
	xTaskCreate(exercise1Display, "exercise1Display", 1000, NULL, 2, &exercise1DisplayHandle);
	xTaskCreate(exercise2Display, "exercise2Display", 1000, NULL, 2, &exercise2DisplayHandle);
	xTaskCreate(exercise3Display, "exercise3Display", 1000, NULL, 2, &exercise3DisplayHandle);

	// Exercise 3.2.2
	xTaskCreate(circleBlinkDynamic, "circleBlinkDynamic", 1000, NULL, 1, &circleBlinkDynamicHandle);
	xTaskCreateStatic(circleBlinkStatic, "circleBlinkStatic", 200, NULL, 2, xStack, &xTaskBuffer);

	// Exercise 3.2.3
	xTaskCreate(timesButtonAPressed, "timesButtonAPressed", 1000, NULL, 1, &timesButtonAPressedHandle);
	xTaskCreate(timesButtonBPressed, "timesButtonBPressed", 1000, NULL, 1, &timesButtonBPressedHandle);
	xTaskCreate(resetTimer, "resetTimer", 1000, NULL, 2, &resetTimerHandle);

	// Exercise 3.2.4
	xTaskCreate(increaseVariable, "increaseVariable", 1000, NULL, 2, &increaseVariableHandle);

	// Exercise 3.2.5
	xTaskCreate(sendPositionUART, "sendPositionUART", 1000, NULL, 4, &sendPositionUARTHandle);
	xTaskCreate(receivePositionUART, "receivePositionUART", 1000, NULL, 4, &receivePositionUARTHandle);
	UARTjoystickQueue = xQueueCreate(BUTTON_QUEUE_LENGTH, sizeof(struct buttons));

	// Exercise 3.3
	xTaskCreate(ex4task1, "ex4task1", 1000, NULL, 1, &ex4task1Handle);
	xTaskCreate(ex4task2, "ex4task2", 1000, NULL, 2, &ex4task2Handle);
	xTaskCreate(ex4task3, "ex4task3", 1000, NULL, 3, &ex4task3Handle);
	xTaskCreate(ex4task4, "ex4task4", 1000, NULL, 4, &ex4task4Handle);
	start_task3 = xSemaphoreCreateBinary();

	// Initial task suspends
	vTaskSuspend(exercise1DisplayHandle);
	vTaskSuspend(exercise2DisplayHandle);
	vTaskSuspend(exercise3DisplayHandle);
    vTaskSuspend(timesButtonAPressedHandle);
    vTaskSuspend(timesButtonBPressedHandle);
    vTaskSuspend(receivePositionUARTHandle);
    vTaskSuspend(sendPositionUARTHandle);
    vTaskSuspend(ex4task1Handle);
    vTaskSuspend(ex4task2Handle);
    vTaskSuspend(ex4task3Handle);
    vTaskSuspend(ex4task4Handle);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}

void frameSwapTask(void * params) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	TickType_t xOldWakeTime;
	xOldWakeTime = 0;
	const TickType_t frameratePeriod = 20;

	while (1){
		// Draw next frame
		xSemaphoreGive(DrawReady);
		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// Swap buffers
		ESPL_DrawLayer();
		xOldWakeTime = xLastWakeTime;
		vTaskDelayUntil(&xLastWakeTime, frameratePeriod);
		TickType_t xDifference;
		xDifference = xLastWakeTime - xOldWakeTime;
		uint8_t xFPS = 1000 / xDifference;
		xQueueSend(FPSQueue, &xFPS, 0);
	}
}

void changeState(volatile unsigned char *state, unsigned char forwards) {
	switch (forwards) {
	case 0:
		if (*state == 0)
			*state = STATE_COUNT;
		else
			(*state)--;
		break;
	case 1:
		if (*state == STATE_COUNT)
			*state = 0;
		else
			(*state)++;
		break;
	default:
		break;
	}
}

void basicStateMachine(void * params) {
	unsigned char current_state = 1; // Default state
	unsigned char state_changed = 1; // Only re-evaluate state if it has changed
	unsigned char input = 0;
	while (1) {
		if (state_changed)
			goto initial_state;

		// Handle state machine input
		if (xQueueReceive(StateQueue, &input, portMAX_DELAY) == pdTRUE) {
			if (input == NEXT_TASK) {
				changeState(&current_state, 1);
				state_changed = 1;
			}
			else if (input == PREV_TASK) {
				changeState(&current_state, 0);
				changeState(&current_state, 0);
				state_changed = 1;
			}
		}
		initial_state:
		// Handle current state
		if (state_changed) {
			switch (current_state) {
			case STATE_ONE:
				vTaskSuspend(timesButtonAPressedHandle);
				vTaskSuspend(timesButtonBPressedHandle);
				vTaskSuspend(receivePositionUARTHandle);
				vTaskSuspend(sendPositionUARTHandle);
			    vTaskSuspend(ex4task1Handle);
			    vTaskSuspend(ex4task2Handle);
			    vTaskSuspend(ex4task3Handle);
			    vTaskSuspend(ex4task4Handle);
				vTaskSuspend(exercise2DisplayHandle);
				vTaskSuspend(exercise3DisplayHandle);
				vTaskResume(exercise1DisplayHandle);
				state_changed = 0;
				break;
			case STATE_TWO:
			    vTaskSuspend(ex4task1Handle);
			    vTaskSuspend(ex4task2Handle);
			    vTaskSuspend(ex4task3Handle);
			    vTaskSuspend(ex4task4Handle);
				vTaskSuspend(exercise1DisplayHandle);
				vTaskSuspend(exercise3DisplayHandle);
				vTaskResume(exercise2DisplayHandle);
				vTaskResume(timesButtonAPressedHandle);
				vTaskResume(timesButtonBPressedHandle);
				vTaskResume(sendPositionUARTHandle);
				vTaskResume(receivePositionUARTHandle);
				state_changed = 0;
				break;
			case STATE_THREE:
				vTaskSuspend(timesButtonAPressedHandle);
				vTaskSuspend(timesButtonBPressedHandle);
				vTaskSuspend(receivePositionUARTHandle);
				vTaskSuspend(sendPositionUARTHandle);
				vTaskSuspend(exercise1DisplayHandle);
				vTaskSuspend(exercise2DisplayHandle);
				vTaskResume(exercise3DisplayHandle);
				vTaskResume(ex4task1Handle);
				vTaskResume(ex4task2Handle);
				vTaskResume(ex4task3Handle);
				vTaskResume(ex4task4Handle);
				state_changed = 0;
				break;
			default:
				break;
			}
		}
	}
}

void exercise1Display(void * params) {
	char str[100]; // buffer for messages to draw to display
	struct buttons buttonStatus; // joystick queue input buffer
	const unsigned char next_state_signal = NEXT_TASK;

	int press_count_A = 0, press_count_B = 0, press_count_C = 0, press_count_D = 0;
	int sliding_text_position = 0, sliding_text_direction = 1;

	int sliderCounter(){
		int m = sliding_text_position;
		if(sliding_text_position == 0){
			sliding_text_position++;
			sliding_text_direction = 0;
			return m;
		}
		if (sliding_text_position < 200){
			if(!sliding_text_direction){
				sliding_text_position++;
				return m;
			}
			else{
				sliding_text_position--;
				return m;
			}
		}
		if (sliding_text_position == 200){
			sliding_text_position--;
			sliding_text_direction = 1;
			return m;
		}
	}

	int circle_angle = 0;
	int circleRotate(){
		int internal_angle = circle_angle;
		if(circle_angle < 360){
			circle_angle++;
			return internal_angle;
		}
		if(circle_angle == 360){
			circle_angle = 0;
			return internal_angle;
		}
	}

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE);

			// State machine input
			if (buttonCount(BUT_E)){
				xQueueSend(StateQueue, &next_state_signal, 100);
			}

            // Clear background
		    gdispClear(White);

			// Generate string with current joystick values
			sprintf(str, "X from left: %5d|Y from top: %5d|VBat: %5d",
					buttonStatus.joystick.x, buttonStatus.joystick.y,
					ADC_GetConversionValue(ESPL_ADC_VBat));
			// Print string of joystick values
			gdispDrawString(SCREEN_SLIDE_X, SCREEN_SLIDE_Y, str, font1, Black);


			// Count number of button presses
			if(buttonStatus.A){
				press_count_A++;
			}
			if(buttonStatus.B){
				press_count_B++;
			}
			if(buttonStatus.C){
				press_count_C++;
			}
			if(buttonStatus.D){
				press_count_D++;
			}
			if(buttonStatus.K){
				press_count_A = 0;
				press_count_B = 0;
				press_count_C = 0;
				press_count_D = 0;
			}
			// Generate string with number of times buttons have been pressed
			sprintf(str, "A: %d|B: %d|C: %d|D: %d", press_count_A,
					press_count_B, press_count_C, press_count_D);

			// Print string of number of presses
			gdispDrawString(SCREEN_SLIDE_X, 11 + SCREEN_SLIDE_Y, str, font1, Black);


			// Draw small teal triangle in the center
			gdispFillConvexPoly(DISPLAY_SIZE_X / 2 + SCREEN_SLIDE_X,
					DISPLAY_SIZE_Y / 2 + SCREEN_SLIDE_Y, triangle, NUM_POINTS, Teal);

			// Draw rotating orange square
			gdispFillArea(ROTATING_SQUARE_X + SCREEN_SLIDE_X, ROTATING_SQUARE_Y + SCREEN_SLIDE_Y, 12, 12, Orange);

			// Draw rotating olive circle
			gdispFillCircle(ROTATING_CIRCLE_X + SCREEN_SLIDE_X, ROTATING_CIRCLE_Y + SCREEN_SLIDE_Y, 12, Olive);


			// Displaying sliding text above figures
			char str_slide[1][70] = { "ESPL	LAB"};
			for (unsigned char j = 0; j < 1; j++)
				gdispDrawString(SLIDING_TEXT_X(str_slide[j]) + SCREEN_SLIDE_X, SLIDING_TEXT_Y(j) + SCREEN_SLIDE_Y, str_slide[j],
												font1, Black);
			// Displaying text below figures
			char str[1][70] = {"ASTEROIDS"};
			for (unsigned char i = 0; i < 1; i++)
							gdispDrawString(TEXT_X(str[i]) + SCREEN_SLIDE_X, TEXT_Y(i) + SCREEN_SLIDE_Y, str[i],
									font1, Black);
		}
	}
}

void exercise2Display(void * params) {
	char str[1][70] = { "EXERCISE 3" };
	char print_string[1][70] = {{0}};
	struct buttons buttonStatus; // joystick queue input buffer
	const unsigned char next_state_signal = NEXT_TASK;
	unsigned int button_A_count = 0;
	unsigned int button_B_count = 0;
	uint8_t fps_num;
	int counter324 = 0;
	int task_switch = 1;
	int change = 0;
    int blink2Hz_toggle = 1;
    int blink1Hz_toggle = 1;
    uint8_t move_uart_x = 0;
    uint8_t move_uart_y = 0;

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE)
				;

			// State machine input
			if (buttonCount(BUT_E)){
				xQueueSend(StateQueue, &next_state_signal, 100);
			}

            // Clear background
		    gdispClear(White);

		    //Exercise 3.2.2
		    if(xSemaphoreTake(BlinkCircle2Hz, 0) == pdTRUE){
		    	blink2Hz_toggle = !blink2Hz_toggle;
		    }
		    if(xSemaphoreTake(BlinkCircle1Hz, 0) == pdTRUE){
		    	blink1Hz_toggle = !blink1Hz_toggle;
		    }

		    if(blink2Hz_toggle == 1){
		    	gdispFillCircle(DISPLAY_SIZE_X / 2 - 20 + SCREEN_SLIDE_X_UART,
		    			DISPLAY_SIZE_Y / 2 + SCREEN_SLIDE_Y_UART - 50, 12, Blue);
		    }
		    if(blink1Hz_toggle == 1){
		    	gdispFillCircle(DISPLAY_SIZE_X / 2 + 20 + SCREEN_SLIDE_X_UART,
		    			DISPLAY_SIZE_Y / 2 + SCREEN_SLIDE_Y_UART - 50, 12, Red);
		    }

		    //Exercise 3.2.3
		    if(xSemaphoreTake(ButtonAPressed, 0) == pdTRUE){
		    	button_A_count++;
		    }
		    if(xSemaphoreTake(ButtonBPressed, 0) == pdTRUE){
		    	button_B_count++;
		    }
		    if(xSemaphoreTake(ResetTimer, 0) == pdTRUE){
		    	button_A_count = 0;
		    	button_B_count = 0;
		    }
			sprintf(print_string, "Times pressed: A: %d|B: %d", button_A_count, button_B_count);
			for (unsigned char i = 0; i < 1; i++){
				gdispDrawString(TEXT_X(print_string[i]) + SCREEN_SLIDE_X_UART,
						TEXT_Y(i) - 5 + SCREEN_SLIDE_Y_UART - 50, print_string[i], font1, Black);
			}

			for (unsigned char i = 0; i < 1; i++)
				gdispDrawString(TEXT_X(str[i]) + SCREEN_SLIDE_X_UART,
						TEXT_Y(i) + 15 + SCREEN_SLIDE_Y_UART - 50, str[i], font1, Black);


			// FPS from 3.2.2.6
			xQueueReceive(FPSQueue, &fps_num, 0);
			sprintf(print_string, "FPS: %d", fps_num);
			for (unsigned char i = 0; i < 1; i++)
				gdispDrawString(TEXT_X(print_string[i]) + SCREEN_SLIDE_X_UART,
						TEXT_Y(i) + 5 + SCREEN_SLIDE_Y_UART - 50, print_string[i], font1, Black);

			// Exercise 3.2.4
		    if(xSemaphoreTake(increaseVariable_Semaphore, 0) == pdTRUE){
		    	counter324++;
		    }
			if (buttonCount(BUT_C)){
				task_switch = !task_switch;
				change = 1;
			}
			if(change){
				if(!task_switch){
					vTaskSuspend(increaseVariableHandle);
					change = 0;
				}
				else if(task_switch){
					vTaskResume(increaseVariableHandle);
					change = 0;
				}
			}
			sprintf(print_string, "Second Counter: %d", counter324);
			for (unsigned char i = 0; i < 1; i++)
				gdispDrawString(TEXT_X(print_string[i]) + SCREEN_SLIDE_X_UART,
						TEXT_Y(i) - 15 + SCREEN_SLIDE_Y_UART - 50, print_string[i], font1, Black);

			// Exercise 3.2.5
			if(xSemaphoreTake(buttonCounts.lock, portMAX_DELAY) == pdTRUE){
				sprintf(print_string, "From connected board: A: %d|B: %d|C: %d|D: %d",
						buttonCounts.buttons.A,
						buttonCounts.buttons.B,
						buttonCounts.buttons.C,
						buttonCounts.buttons.D);
				xSemaphoreGive(buttonCounts.lock);
				for (unsigned char i = 0; i < 1; i++)
					gdispDrawString(TEXT_X(print_string[i]) + SCREEN_SLIDE_X_UART,
							TEXT_Y(i) - 25 + SCREEN_SLIDE_Y_UART - 50, print_string[i], font1, Black);
			}

			if(xSemaphoreTake(joystickUart.lock, portMAX_DELAY) == pdTRUE){
				move_uart_x = joystickUart.joystick.x;
				move_uart_y = joystickUart.joystick.y;
				xSemaphoreGive(joystickUart.lock);
			}

		}
	}
}

void exercise3Display(void * params){
	struct buttons buttonStatus;
	const unsigned char next_state_signal = PREV_TASK;
	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			while(xQueueReceive(ButtonQueue, &buttonStatus,0) == pdTRUE);
			if (buttonCount(BUT_E)){
				xQueueSend(StateQueue, &next_state_signal, 100);
			}
            // Clear background
			gdispClear(White);


			if(xSemaphoreTake(printString.lock, portMAX_DELAY) == pdTRUE){
				for(unsigned char i = 0; i < 15; i++){
					gdispDrawString(TEXT_X(printString.print[i]) - 20, TEXT_Y(i) - 150, printString.print[i], font1, Black);
				}
				xSemaphoreGive(printString.lock);
			}
		}
	}
}

//Exercise 3.2.2
void circleBlinkDynamic(void *params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t delayPeriod = 500;
	while (1) {
		xSemaphoreGive(BlinkCircle2Hz);
		vTaskDelayUntil(&xLastTickTime, delayPeriod);
	}
}

void circleBlinkStatic(void *params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t delayPeriod = 1000;
	while (1) {
		xSemaphoreGive(BlinkCircle1Hz);
		vTaskDelayUntil(&xLastTickTime, delayPeriod);
	}
}

//Exercise 3.2.3
void timesButtonAPressed(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t pollingRate = 20;
	while(1){
		if(buttonCount(BUT_A)){
			xSemaphoreGive(ButtonAPressed);
		}
		vTaskDelayUntil(&xLastTickTime, pollingRate);
	}
}

void timesButtonBPressed(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t pollingRate = 20;
	while(1){
		if(buttonCount(BUT_B)){
			xSemaphoreGive(ButtonBPressed);
		}
		vTaskDelayUntil(&xLastTickTime, pollingRate);
	}
}

void resetTimer(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t delayTime = RESET_TIMER_EX_3 * 1000;
	while(1){
		xSemaphoreGive(ResetTimer);
		vTaskDelayUntil(&xLastTickTime, delayTime);
	}
}

// Exercise 3.2.4
void increaseVariable(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t delayTime = 1000; // 1 second
	while(1){
		xSemaphoreGive(increaseVariable_Semaphore);
		vTaskDelayUntil(&xLastTickTime, delayTime);
	}
}

// Exercise 3.2.5
void sendPositionUART(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t pollingRate = 20;
	struct buttons buttonStatus;
	struct coord prev_joystick, abs_joystick;
	const int joystick_threshold = 20;
	unsigned char send=0;
	while(1){
		xQueueReceive(ButtonQueue, &buttonStatus, 100);
		const uint8_t checksum = buttonStatus.joystick.x ^ buttonStatus.joystick.y
				^ buttonStatus.A ^ buttonStatus.B ^ buttonStatus.C ^ buttonStatus.D
				^ buttonStatus.E ^ buttonStatus.K;
		if(buttonStatus.A)
			send = 1;
		if(buttonStatus.B)
			send = 1;
		if(buttonStatus.C)
			send = 1;
		if(buttonStatus.D)
			send = 1;
		if(buttonStatus.K)
			send = 1;
		abs_joystick.x = abs(buttonStatus.joystick.x - prev_joystick.x);
		abs_joystick.y = abs(buttonStatus.joystick.y - prev_joystick.y);
		if(abs_joystick.x > joystick_threshold)
			send = 1;
		if(abs_joystick.y > joystick_threshold)
			send = 1;
		if(send){
			UART_SendData(startByte);
			UART_SendData(buttonStatus.joystick.x);
			UART_SendData(buttonStatus.joystick.y);
			UART_SendData(buttonStatus.A);
			UART_SendData(buttonStatus.B);
			UART_SendData(buttonStatus.C);
			UART_SendData(buttonStatus.D);
			UART_SendData(buttonStatus.E);
			UART_SendData(buttonStatus.K);
			UART_SendData(checksum);
			UART_SendData(stopByte);
			memcpy(&prev_joystick, &buttonStatus.joystick, sizeof(struct coord));
		}
		send=0;
		vTaskDelayUntil(&xLastTickTime, pollingRate);
	}
}

void receivePositionUART(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	const TickType_t pollingRate = 20;
	char input;
	uint8_t pos = 0;
	char checksum;
	char buffer[11] = { { 0 } };
	struct buttons buttonStatus_internal = { { 0 } };
	while (1) {
		// wait for data in queue
		xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);
		// decode package by buffer position
		switch (pos) {
		// start byte
		case 0:
			if (input != startByte)
				break;
			buffer[pos] = input;
			pos++;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			// read received data in buffer
			buffer[pos] = input;
			pos++;
			break;
		case 10:
			// Check if package is corrupted
			checksum = buffer[1] ^ buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5]
					^ buffer[6] ^ buffer[7] ^ buffer[8];
			if (input == stopByte && checksum == buffer[9]) {
				buttonStatus_internal.joystick.x = buffer[1];
				buttonStatus_internal.joystick.y = buffer[2];
				buttonStatus_internal.A = buffer[3];
				buttonStatus_internal.B = buffer[4];
				buttonStatus_internal.C = buffer[5];
				buttonStatus_internal.D = buffer[6];
				buttonStatus_internal.E = buffer[7];
				buttonStatus_internal.K = buffer[8];

				xSemaphoreTake(buttonCounts.lock, portMAX_DELAY);
				if(buttonStatus_internal.A)
					buttonCounts.buttons.A++;
				if(buttonStatus_internal.B)
					buttonCounts.buttons.B++;
				if(buttonStatus_internal.C)
					buttonCounts.buttons.C++;
				if(buttonStatus_internal.D)
					buttonCounts.buttons.D++;
				xSemaphoreGive(buttonCounts.lock);

				xSemaphoreTake(joystickUart.lock, portMAX_DELAY);
				if(buttonStatus_internal.joystick.x){
					memcpy(&joystickUart.joystick.x, &buffer[1], sizeof(uint8_t));
				}
				if(buttonStatus_internal.joystick.y){
					memcpy(&joystickUart.joystick.y, &buffer[2], sizeof(uint8_t));
				}
				xSemaphoreGive(joystickUart.lock);
			}
			pos = 0;
			break;
		}
		vTaskDelayUntil(&xLastTickTime, pollingRate);
	}
}

// Exercise 3.3
void ex4task1(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	TickType_t first_exec;
	first_exec = xLastTickTime;
	const TickType_t delay = 1;
	while(1){
		if((xTaskGetTickCount() - first_exec) < 15){
			xSemaphoreTake(printString.lock, portMAX_DELAY);
			strcat(printString.print[xTaskGetTickCount() - first_exec], "1 ");
			xSemaphoreGive(printString.lock);
		}
		vTaskDelayUntil(&xLastTickTime, delay);
	}
}
void ex4task2(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	TickType_t first_exec;
	first_exec = xLastTickTime;
	const TickType_t delay = 2;
	while(1){
		if((xTaskGetTickCount() - first_exec) < 15){
			xSemaphoreTake(printString.lock, portMAX_DELAY);
			strcat(printString.print[xTaskGetTickCount() - first_exec], "2 ");
			xSemaphoreGive(printString.lock);
			xSemaphoreGive(start_task3 );
		}
		vTaskDelayUntil(&xLastTickTime, delay);
	}
}
void ex4task3(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	TickType_t first_exec;
	first_exec = xLastTickTime;
	while(1){
		if((xTaskGetTickCount() - first_exec) < 15){
			xSemaphoreTake(start_task3, portMAX_DELAY);
			xSemaphoreTake(printString.lock, portMAX_DELAY);
			strcat(printString.print[xTaskGetTickCount() - first_exec], "3 ");
			xSemaphoreGive(printString.lock);
		}
	}
}
void ex4task4(void * params){
	TickType_t xLastTickTime;
	xLastTickTime = xTaskGetTickCount();
	TickType_t first_exec;
	first_exec = xLastTickTime;
	const TickType_t delay = 4;
	while(1){
		if((xTaskGetTickCount() - first_exec) < 15){
			xSemaphoreTake(printString.lock, portMAX_DELAY);
			strcat(printString.print[xTaskGetTickCount() - first_exec], "4 ");
			xSemaphoreGive(printString.lock);
		}
		vTaskDelayUntil(&xLastTickTime, delay);
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
		buttonStatus.A = buttonCount(BUT_A);
		buttonStatus.B = buttonCount(BUT_B);
		buttonStatus.C = buttonCount(BUT_C);
		buttonStatus.D = buttonCount(BUT_D);
		buttonStatus.E = 0;
		buttonStatus.K = buttonCount(BUT_K);
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

