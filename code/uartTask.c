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

extern QueueHandle_t RemoteQueuePlayer;
extern QueueHandle_t RemoteQueueSync;

void uartTask (void * params){
	char uart_input;
	char uart_buffer[20] = { { 0 } };
	char checksum;
	TickType_t uart_start = xTaskGetTickCount();
	TickType_t uart_send_alive_period = UART_SEND_ALIVE_PERIOD;
	TickType_t uart_check_alive_period = UART_CHECK_ALIVE_PERIOD;
	static const uint8_t alive_byte_standard = UART_STANDARD_BYTE;
	static const uint8_t alive_byte_uart_connected = UART_CONNECTED_BYTE;
	unsigned int can_start_game = 0;
	boolean uart_connected = false;
	uint8_t pos = 0;
	uart_master_or_slave uart_master_or_slave = is_master;
	while(1){
//		Receving over UART
		xQueueReceive(ESPL_RxQueue, &uart_input, 0);
		switch((xTaskGetTickCount() - uart_start) % 20){
		case 0:
			uart_buffer[0] = uart_input;
			break;
		case 8:
			uart_buffer[1] = uart_input;
			break;
		case 16:
			uart_buffer[2] = uart_input;
			break;
		default:
			break;
		}
	}
}
