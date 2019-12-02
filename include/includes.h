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
#include "staticFunctions.h"
#include "buttons.h"
#include "exercise4.h"

#define STATE_COUNT 3

#define MAIN_MENU_STATE   1
#define PAUSE_MENU_STATE   2
#define SINGLE_PLAYER_STATE		3
#define DISPLAY_SIZE_X  		320
#define DISPLAY_SIZE_Y  		240

#define TEXT_X(TEXT)	 		DISPLAY_SIZE_X / 2 - (gdispGetStringWidth(TEXT, font1) / 2)
#define TEXT_Y(LINE)	 		DISPLAY_SIZE_Y / 2 - (gdispGetFontMetric(font1, fontHeight) * -(LINE + 0.5)) + 65
