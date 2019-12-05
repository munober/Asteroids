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

//uint16_t determinePlayerPositionX(uint8_t thrust, uint16_t angle, uint16_t current_x, uint16_t current_y){
//	struct coord_draw position_internal;
//	position_internal.x = current_x;
//	position_internal.x += 10*thrust;
//	return position_internal.x;
//}
//
//uint16_t determinePlayerPositionY(uint8_t thrust, uint16_t angle, uint16_t current_x, uint16_t current_y){
//	struct coord_draw position_internal;
//	position_internal.y = current_y;
//	position_internal.y += 10*thrust;
//	return position_internal.y;
//}
