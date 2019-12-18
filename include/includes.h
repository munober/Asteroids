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

/* Game settings and stuff */
#include "buttonDebounce.h"
#include "buttons.h"
#include "gameConfig.h"

#define STATE_COUNT					9

#define MAIN_MENU_STATE  		 	1
#define SINGLE_PLAYER_STATE			2
#define PAUSE_MENU_STATE  			3
#define CHEATS_MENU_STATE			4
#define HIGHSCORE_DISPLAY_STATE		5
#define HIGHSCORE_INTERFACE_STATE	6
#define MULTIPLAYER_STATE			7
#define SINGLE_PLAYER_LEVEL_2		8
#define SINGLE_PLAYER_LEVEL_3		9

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

#define DISPLAY_CENTER_X DISPLAY_SIZE_X/2
#define DISPLAY_CENTER_Y DISPLAY_SIZE_Y/2

#include "buttons.h"

typedef enum {
	false = 0,
	true = 1
} boolean;

struct coord_draw {
	int16_t x;
	int16_t y;
};

struct coord_saucer_shots {
	float x;
	float y;
};

struct direction {
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
};

typedef enum {
	fine = 0,
	hit = 1
} player_status;

typedef enum {
	hide = 0,
	spawn = 1
} shot_status;

typedef enum {
	none = 0,
	one = 1,
	two = 2,
	three = 3
} hit_counter;

typedef enum {
	up_and_left = 0,
	up_and_right = 1,
	down_and_right = 2,
	down_and_left = 3
} saucer_shot_direction;

struct players_ship {
	struct coord_draw position;
	struct coord_draw position_old;
	player_status state;
};

struct player_input{
	int8_t thrust;
	int16_t angle;
	int16_t shots_fired;
};

struct asteroid {
    struct coord_draw position;
    struct coord_draw spawn_position;
    hit_counter remain_hits;
    int16_t shape;
    boolean position_locked;
};

struct saucer {
	struct coord_draw position;
	int16_t route_number;
	int16_t turn_number;
	boolean turning;
	struct coord_saucer_shots shots[10];
	saucer_shot_direction shot_direction[10];
	boolean shot_fired[10];
	uint8_t shot_number;
	float ratios[10]; // This variable describes the ratio of x to y component of the distance between saucer and player at time of saucer fire
};

struct shot {
	struct coord_draw position;
	int16_t angle;
	shot_status status;
};
