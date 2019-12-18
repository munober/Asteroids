/*
 * drawTaskSingle.h
 *
 *  Created on: Dec 2, 2019
 *      Author: lab_espl_stud04
 */

#ifndef INCLUDE_DRAWTASKSINGLE_H_
#define INCLUDE_DRAWTASKSINGLE_H_

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
	boolean position_locked;
	int16_t route_number;
	int16_t turn_number;
	boolean turning;
    hit_counter remain_hits;
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

#endif /* INCLUDE_DRAWTASKSINGLE_H_ */

