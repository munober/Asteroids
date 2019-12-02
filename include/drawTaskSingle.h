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
	uint8_t x;
	uint8_t y;
};

struct asteroid {
    struct coord position;
};




#endif /* INCLUDE_DRAWTASKSINGLE_H_ */
