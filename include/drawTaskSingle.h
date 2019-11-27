/*
 *  Created on: Nov 27, 2019
 *      Author: Freddy
 */

#ifndef __DRAWTASKSINGLE_H__
#define __DRAWTASKSINGLE_H__

struct coord {
	uint8_t x;
	uint8_t y;
};

struct asteroid {
    struct coord a1;
};

void drawTaskSingle(void * params);

#endif
