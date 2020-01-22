/*
 * checkJoystickTask.c
 *
 *  Created on: Dec 3, 2019
 *      Author: Teodor Fratiloiu
 */

#include <math.h>
#include <stdio.h>
#include "includes.h"
#include "checkJoystickTask.h"

extern QueueHandle_t JoystickQueue;
//extern QueueHandle_t PlayerNavigationQueue;

float rotationAngle(float x, float y){
	float angle;
	angle = atan2f(y, x);
	return angle;
}

void checkJoystickTask (void * params){
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t PollingRate = 10;
	struct joystick_angle_pulse joystick_internal;
	struct joystick_angle_pulse joystick_internal_old;

	while (1) {
		joystick_internal.axis.x = (uint8_t)(ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		joystick_internal.axis.y = (uint8_t)(255 - (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4));

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

		// CURRENT JOYSTICK ANGLE IN ONE OF 8 VALUES
		switch(joystick_internal.pulse.x){
		case JOYSTICK_PULSE_LEFT:
			if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
				joystick_internal.angle = JOYSTICK_ANGLE_NW;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
				joystick_internal.angle = JOYSTICK_ANGLE_SW;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_NULL)
				joystick_internal.angle = JOYSTICK_ANGLE_W;
			break;
		case JOYSTICK_PULSE_RIGHT:
			if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
				joystick_internal.angle = JOYSTICK_ANGLE_NE;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
				joystick_internal.angle = JOYSTICK_ANGLE_SE;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_NULL)
				joystick_internal.angle = JOYSTICK_ANGLE_E;
			break;
		case JOYSTICK_PULSE_NULL:
			if(joystick_internal.pulse.y == JOYSTICK_PULSE_UP){
				joystick_internal.angle = JOYSTICK_ANGLE_N;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_DOWN){
				joystick_internal.angle = JOYSTICK_ANGLE_S;
			}
			else if(joystick_internal.pulse.y == JOYSTICK_PULSE_NULL)
				joystick_internal.angle = JOYSTICK_ANGLE_NULL;
			break;
		}

		// Use this because it makes interaction more responsive
		if(joystick_internal_old.angle != joystick_internal.angle){
			xQueueSend(JoystickQueue, &joystick_internal, 0);
			memcpy(&joystick_internal_old, &joystick_internal, sizeof(struct joystick_angle_pulse));
		}

		vTaskDelayUntil(&xLastWakeTime, PollingRate);
	}
}
