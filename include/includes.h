/**
 * Main include file.
 *
 * @author: Jonathan Müller-Boruttau, Nadja Peters nadja.peters@tum.de (RCS, TUM)
 * 			Teodor Fratiloiu, Frederik Zumegen
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

#define HEADING_ANGLE_NULL			0
#define HEADING_ANGLE_E				1
#define HEADING_ANGLE_NE			2
#define HEADING_ANGLE_N				3
#define HEADING_ANGLE_NW			4
#define HEADING_ANGLE_W				5
#define HEADING_ANGLE_SW			6
#define HEADING_ANGLE_S				7
#define HEADING_ANGLE_SE			8

#define DISPLAY_CENTER_X DISPLAY_SIZE_X/2
#define DISPLAY_CENTER_Y DISPLAY_SIZE_Y/2

#include "buttons.h"

typedef enum {
	false = 0,
	true = 1
} boolean;

typedef enum {
	is_slave = 0,
	is_master = 1
} uart_master_or_slave;


struct coord_draw {
	int16_t x;
	int16_t y;
};
struct coord_int {
	int16_t x;
	int16_t y;
};

struct coord_flt {
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

typedef enum {
	no_ort = 0,
	N = 1,
	NE = 2,
	E = 3,
	SE = 4,
	S = 5,
	SW = 6,
	W = 7,
	NW = 8
} compass_orientation;

typedef enum {
	left = 0,
	right = 1,
	up = 2,
	down = 3
} sides;

struct players_ship {
	struct coord_flt speed_current;
	struct coord_flt speed_goal;
	struct coord_flt position;
	struct coord_flt position_old;
	player_status state;
};

struct player_input{
	int8_t thrust;
	int16_t angle;
	int16_t shots_fired;
};

struct asteroid {
    struct coord_flt position;
    struct coord_int spawn_position;
    sides spawn_side;
    compass_orientation initial_orientation;
    compass_orientation orientation;
    hit_counter remain_hits;
    TickType_t hit_timestamp;
    int16_t shape;
    boolean position_locked;
};

struct saucer {
	struct coord_int position;
	boolean position_locked;
	int16_t route_number;
	int16_t turn_number;
	boolean turning;
    hit_counter remain_hits;
	struct coord_flt shots[10];
	saucer_shot_direction shot_direction[10];
	boolean shot_fired[10];
	uint8_t shot_number;
	float ratios[10]; // This variable describes the ratio of x to y component of the distance between saucer and player at time of saucer fire
};

struct shot {
	struct coord_int position;
	int16_t angle;
	shot_status status;
};

struct shot_multiplayer {
	int x;
	int y;
	int heading;
	shot_status status;
};

struct coord_byte {
	uint8_t x;
	uint8_t y;
};

struct remote_player {
	struct coord_byte player;
	uint8_t player_angle;
	struct coord_byte laser[10];
};

struct remote_sync {
	struct coord_byte asteroid[10];
	uint8_t score;
};
