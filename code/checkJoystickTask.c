/*
 * checkJoystickTask.c
 *
 *  Created on: Dec 3, 2019
 *      Author: lab_espl_stud04
 */

#include <math.h>
#include <stdio.h>
#include "includes.h"
#include "checkJoystickTask.h"

#define OFFSET_X		0.0
#define OFFSET_Y		0.0

//extern QueueHandle_t ButtonQueue;
//extern QueueHandle_t StateQueue;
extern QueueHandle_t JoystickAngleQueue;
extern QueueHandle_t JoystickPulseQueue;

void checkJoystickTask (void * params){
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t PollingRate = 20;
	struct coord joystick = { { 0 } };
	double angle = 45.0;
	double num = 1.0;
	unsigned char pulse = JOYSTICK_PULSE_NULL;

	while (1) {
		joystick.x = (double) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		joystick.y = (double) (255 - (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4));

		// CURRENT JOYSTICK ANGLE OUTPUT
//		angle = CONVERT_TO_DEG * atan ((joystick.y - OFFSET_X) / (joystick.x - OFFSET_Y));
		angle = atan(num) * CONVERT_TO_DEG;
		xQueueSend(JoystickAngleQueue, &angle, portMAX_DELAY);

		// SEND UP/DOWN/LEFT/RIGHT PULSE
		if(angle >= 45.0 && angle <= 135.0)
			pulse = JOYSTICK_PULSE_UP;
		else if(angle >= 135.0 && angle <= 225.0)
			pulse = JOYSTICK_PULSE_LEFT;
		else if(angle >= 225.0 && angle <= 315.0)
			pulse = JOYSTICK_PULSE_DOWN;
		else if(angle >= 315.0 && angle <= 45.0)
			pulse = JOYSTICK_PULSE_RIGHT;
		else
			pulse = JOYSTICK_PULSE_NULL;

		xQueueSend(JoystickPulseQueue, &pulse, portMAX_DELAY);
		vTaskDelayUntil(&xLastWakeTime, PollingRate);
	}
}
