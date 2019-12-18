/*
 * uartTask.c
 *
 *  Created on: Dec 18, 2019
 *      Author: lab_espl_stud04
 */
#include "includes.h"
#include "uartTask.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uartTask.h"

void uartTask (void * params){
	const TickType_t uart_send_alive_rate = 5;
	TickType_t send_alive_timestamp = xTaskGetTickCount();
// 	const uint8_t checksum = buttonStatus.joystick.x ^ buttonStatus.joystick.y;
	unsigned char i_am_alive = "Hi";
	unsigned char input;
	while(1){
		if(xTaskGetTickCount() - send_alive_timestamp > uart_send_alive_rate){
//			UART_SendData(i_am_alive);
			send_alive_timestamp = xTaskGetTickCount();
		}
	}
//	vTaskDelayUntil(&xLastTickTime, pollingRate);
}
