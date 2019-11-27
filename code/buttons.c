/*
 * buttons.c
 *
 *  Created on: Nov 14, 2019
 *      Author: lab_espl_stud04
 */

#include "FreeRTOS.h"
#include "semphr.h"

#include "buttons.h"

locked_buttons_t initButtons(void)
{
	locked_buttons_t ret = {{0}};

	ret.lock = xSemaphoreCreateMutex();

	return ret;
}


locked_joystick_t initJoystick(void){
	locked_joystick_t ret = {{0}};

	ret.lock = xSemaphoreCreateMutex();

	return ret;
}

