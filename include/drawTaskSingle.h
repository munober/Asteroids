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
};

struct shot {
	struct coord_draw position;
	int16_t angle;
	shot_status status;
};

#endif /* INCLUDE_DRAWTASKSINGLE_H_ */

