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

locked_string_t initString(void){
	locked_string_t ret = {" ", "Tick 1: ", "Tick 2: ", "Tick 3: ", "Tick 4: ", "Tick 5: ",
			"Tick 6: ", "Tick 7: ", "Tick 8: ", "Tick 9: ", "Tick 10: ",
			"Tick 11: ", "Tick 12: ", "Tick 13: ", "Tick 14: ", "Tick 15: "	};

	ret.lock = xSemaphoreCreateMutex();

	return ret;
}
