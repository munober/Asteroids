/*
 * buttons.c
 *
 *  Created on: Nov 14, 2019
 *      Author: Teodor Fratiloiu
 */

//This file has some useful mutexes, but since
//the queues have worked so nicely, we decided to give up
//on the mutexes.

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

