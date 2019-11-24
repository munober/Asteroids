/*
 * exercise4.c
 *
 *  Created on: Nov 18, 2019
 *      Author: lab_espl_stud04
 */

#include "FreeRTOS.h"
#include "semphr.h"

#include "exercise4.h"

locked_ex4output_t initEx4Output(void){
	locked_ex4output_t ret = {{0}};

	ret.lock = xSemaphoreCreateMutex();

	return ret;
}
