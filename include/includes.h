/**
 * Main include file.
 *
 * @author: Jonathan Müller-Boruttau, Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 **/
#include "stdio.h"

/* Board includes */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f429i_discovery_lcd.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* uGFX inclues. */
#include "gfx.h"

/* convenience functions and init includes */

#include "ESPL_functions.h"
#include "main.h"

/* ESPL Excercise Code */
#include "buttonDebounce.h"
#include "buttons.h"

#define STATE_COUNT					7

#define MAIN_MENU_STATE  		 	1
#define SINGLE_PLAYER_STATE			2
#define PAUSE_MENU_STATE  			3
#define CHEATS_MENU_STATE			4
#define HIGHSCORE_DISPLAY_STATE		5
#define HIGHSCORE_INTERFACE_STATE	6
#define MULTIPLAYER_STATE			7

#define DISPLAY_SIZE_X  		320
#define DISPLAY_SIZE_Y  		240

#define TEXT_X(TEXT)	 		DISPLAY_SIZE_X / 2 - (gdispGetStringWidth(TEXT, font1) / 2)
#define TEXT_Y(LINE)	 		DISPLAY_SIZE_Y / 2 - (gdispGetFontMetric(font1, fontHeight) * -(LINE + 0.5)) + 65

#define PI 						3.14159265
#define CONVERT_TO_DEG			180.0 / PI
#define CONVERT_TO_RAD			PI / 180.0

#define JOYSTICK_PULSE_UP			1
#define JOYSTICK_PULSE_LEFT			2
#define JOYSTICK_PULSE_DOWN			3
#define JOYSTICK_PULSE_RIGHT		4
#define JOYSTICK_PULSE_NULL			5

#define JOYSTICK_ANGLE_NULL			0
#define JOYSTICK_ANGLE_E			360
#define JOYSTICK_ANGLE_NE			45
#define JOYSTICK_ANGLE_N			90
#define JOYSTICK_ANGLE_NW			135
#define JOYSTICK_ANGLE_W			180
#define JOYSTICK_ANGLE_SW			225
#define JOYSTICK_ANGLE_S			270
#define JOYSTICK_ANGLE_SE			315
