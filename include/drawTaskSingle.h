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

struct coord_draw {
	uint16_t x;
	uint16_t y;
};

typedef enum {
	none = 0,
	one = 1,
	two = 2,
	three = 3

} hit_counter;

struct asteroid {
    struct coord_draw position;
    hit_counter remain_hits;
};

#endif /* INCLUDE_DRAWTASKSINGLE_H_ */

