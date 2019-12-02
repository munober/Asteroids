/*
 *  Created on: Nov 27, 2019
 *      Author: Freddy
 */

#ifndef __DRAWTASKSINGLE_H__
#define __DRAWTASKSINGLE_H__

struct coord_draw {
	uint8_t x;
	uint8_t y;
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

void drawTaskSingle(void * params);

#endif
