/*
 * buttons.c
 *
 *  Created on: Nov 6, 2019
 *      Author: Teodor Fratiloiu
 */

#include "stm32f4xx.h"
#include "ESPL_functions.h"
#include "buttonDebounce.h"


int lastButtonState[6] = {0};
int buttonState[6] = {0};
unsigned long lastDebounceTime[6] = {0};
//unsigned long pressCount[6] = {0};
const unsigned long debounceDelay = 15;

unsigned char buttonCount(char button_index){
	static char button_state;
	switch(button_index){
		case BUT_A: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_A,
				ESPL_Pin_Button_A); break;
		case BUT_B: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_B,
				ESPL_Pin_Button_B); break;
		case BUT_C: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_C,
				ESPL_Pin_Button_C); break;
		case BUT_D: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_D,
				ESPL_Pin_Button_D); break;
		case BUT_E: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_E,
				ESPL_Pin_Button_E); break;
		case BUT_K: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_K,
				ESPL_Pin_Button_K); break;
		default:
			break;
	}
	if ((button_state == 1) && (lastButtonState[button_index] == 0)){
		lastDebounceTime[button_index] = xTaskGetTickCount();
	}
	if ((xTaskGetTickCount() - lastDebounceTime[button_index]) > debounceDelay){
		if (button_state != buttonState[button_index]){
			buttonState[button_index] = button_state;
			if(button_state == 1){
				return 1;
				//lastButtonState[button_index] = button_state;
			}
		}
    }
	lastButtonState[button_index] = button_state;
	return 0;
}

int lastButtonStateTwo[6] = {0};
int buttonStateTwo[6] = {0};
unsigned long lastDebounceTimeTwo[6] = {0};
//unsigned long pressCountTwo[6] = {0};

unsigned char buttonCountWithLiftup(char button_index){
	static char button_state;
	switch(button_index){
		case BUT_A: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_A,
				ESPL_Pin_Button_A); break;
		case BUT_B: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_B,
				ESPL_Pin_Button_B); break;
		case BUT_C: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_C,
				ESPL_Pin_Button_C); break;
		case BUT_D: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_D,
				ESPL_Pin_Button_D); break;
		case BUT_E: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_E,
				ESPL_Pin_Button_E); break;
		case BUT_K: button_state = !GPIO_ReadInputDataBit(ESPL_Register_Button_K,
				ESPL_Pin_Button_K); break;
		default:
			break;
	}
	if ((button_state == 1) && (lastButtonStateTwo[button_index] == 0)){
		lastDebounceTimeTwo[button_index] = xTaskGetTickCount();
	}
	if ((xTaskGetTickCount() - lastDebounceTimeTwo[button_index]) > debounceDelay){
		if (button_state != buttonStateTwo[button_index]){
			buttonStateTwo[button_index] = button_state;
//			if(button_state == 1){
//				return 1;
//			}
			return 1;
		}
    }
	lastButtonStateTwo[button_index] = button_state;
	return 0;
}

