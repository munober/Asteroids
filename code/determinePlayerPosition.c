/*
 * determinePlayerPosition.c
 *
 *  Created on: Dec 4, 2019
 *      Author: lab_espl_stud04
 */
#include "includes.h"
#include "determinePlayerPosition.h"
#include "drawTaskSingle.h"
#include <math.h>

extern QueueHandle_t JoystickAngle360Queue;

uint16_t determinePlayerPositionX(uint8_t thrust, uint16_t angle, uint16_t current_x, uint16_t current_y){
	struct coord_draw position_internal;
	uint16_t ret = 0;
	position_internal.x = current_x;
	position_internal.y = current_y;
	position_internal.x += 10*thrust;
	position_internal.y += 10*thrust;
	return ret;
}

uint16_t determinePlayerPositionY(uint8_t thrust, uint16_t angle, uint16_t current_x, uint16_t current_y){
	struct coord_draw position_internal;
	uint16_t ret = 0;
	position_internal.x = current_x;
	position_internal.y = current_y;
	position_internal.x += 10*thrust;
	position_internal.y += 10*thrust;
	return ret;
}

// This piece of code is kinda odd and doesnt work properly, but dont delete it yet.
//
//		if(joystick_internal.axis.x <= 128){
//			if(joystick_internal.axis.y <= 128){
//				joystick_internal.angle = CONVERT_TO_DEG * atan2((joystick_internal.axis.y)/(joystick_internal.axis.x));
//				joystick_internal.angle = joystick_internal.angle + 90;
//			}
//			else{
//				joystick_internal.angle = CONVERT_TO_DEG * atan2((joystick_internal.axis.y - 128)/(joystick_internal.axis.x));
//				joystick_internal.angle = joystick_internal.angle + 180;
//			}
//		}
//		else{
//			if(joystick_internal.axis.y <= 128){
//				joystick_internal.angle = CONVERT_TO_DEG * atan2((joystick_internal.axis.y)/(joystick_internal.axis.x - 128));
//				joystick_internal.angle = joystick_internal.angle + 0;
//			}
//			else{
//				joystick_internal.angle = CONVERT_TO_DEG * atan2((joystick_internal.axis.y - 128)/(joystick_internal.axis.x - 128));
//				joystick_internal.angle = joystick_internal.angle + 270;
//			}
//		}
