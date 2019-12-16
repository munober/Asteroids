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

typedef enum {
	false = 0,
	true = 1
} boolean;

struct coord_draw {
	int16_t x;
	int16_t y;
};

typedef enum {
	fine = 0,
	hit = 1
} player_status;

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
	char asteroids_to_destroy; // This is a counter for how many asteroids have to be destroyed until level completion
};

struct player_input{
	uint8_t thrust;
	uint16_t angle;
};

struct asteroid {
    struct coord_draw position;
    struct coord_draw spawn_position;
    hit_counter remain_hits;
    int16_t shape;
};

struct saucer {
	struct coord_draw position;
	int16_t route_number;
	int16_t turn_number;
	boolean turning;
};

#endif /* INCLUDE_DRAWTASKSINGLE_H_ */

