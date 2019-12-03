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

extern QueueHandle_t JoystickQueue;

void checkJoystickTask (void * params){
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t PollingRate = 10;
	struct joystick_angle_pulse joystick_internal;

	while (1) {
		joystick_internal.axis.x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		joystick_internal.axis.y = (uint8_t) (255 - (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4));

		// SEND UP/LEFT/DOWN/RIGHT PULSE

		// X AXIS
		if (joystick_internal.axis.y < 120){
			joystick_internal.pulse.y = JOYSTICK_PULSE_UP;
		}
		else if (joystick_internal.axis.y > 140){
			joystick_internal.pulse.y = JOYSTICK_PULSE_DOWN;
		}

		// Y AXIS
		if (joystick_internal.axis.x < 120){
			joystick_internal.pulse.x = JOYSTICK_PULSE_LEFT;
		}
		else if (joystick_internal.axis.x > 140){
			joystick_internal.pulse.x = JOYSTICK_PULSE_RIGHT;
		}
		if(joystick_internal.axis.y >= 120 && joystick_internal.axis.y <= 140){
			if(joystick_internal.axis.x >= 120 && joystick_internal.axis.x <= 140){
				joystick_internal.pulse.y = JOYSTICK_PULSE_NULL;
				joystick_internal.pulse.x = JOYSTICK_PULSE_NULL;
			}
		}

		// CURRENT JOYSTICK ANGLE
		switch(joystick_internal.pulse.x){
		case JOYSTICK_PULSE_LEFT:
			if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
				joystick_internal.angle = 135;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
				joystick_internal.angle = 225;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_NULL)
				joystick_internal.angle = 180;
			break;
		case JOYSTICK_PULSE_RIGHT:
			if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
				joystick_internal.angle = 45;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
				joystick_internal.angle = 315;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_NULL)
				joystick_internal.angle = 0;
			break;
		case JOYSTICK_PULSE_NULL:
			if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
				joystick_internal.angle = 90;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
				joystick_internal.angle = 270;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_NULL)
				joystick_internal.angle = 0;
			break;
		}


		// This piece of code is kinda odd and doesnt work properly, but dont delete it yet.

//		if(joystick_internal.axis.x <= 128){
//			if(joystick_internal.axis.y <= 128){
//				joystick_internal.angle = CONVERT_TO_DEG * atan((joystick_internal.axis.y)/(joystick_internal.axis.x));
//				joystick_internal.angle = joystick_internal.angle + 90;
//			}
//			else{
//				joystick_internal.angle = CONVERT_TO_DEG * atan((joystick_internal.axis.y - 128)/(joystick_internal.axis.x));
//				joystick_internal.angle = joystick_internal.angle + 180;
//			}
//		}
//		else{
//			if(joystick_internal.axis.y <= 128){
//				joystick_internal.angle = CONVERT_TO_DEG * atan((joystick_internal.axis.y)/(joystick_internal.axis.x - 128));
//				joystick_internal.angle = joystick_internal.angle + 0;
//			}
//			else{
//				joystick_internal.angle = CONVERT_TO_DEG * atan((joystick_internal.axis.y - 128)/(joystick_internal.axis.x - 128));
//				joystick_internal.angle = joystick_internal.angle + 270;
//			}
//		}

		xQueueSend(JoystickQueue, &joystick_internal, 0);
		vTaskDelayUntil(&xLastWakeTime, PollingRate);
	}
}
