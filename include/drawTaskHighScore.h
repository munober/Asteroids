/*
 * drawTaskHighScore.h
 *
 *  Created on: Dec 9, 2019
 *      Author: lab_espl_stud04
 */

#ifndef INCLUDE_DRAWTASKHIGHSCORE_H_
#define INCLUDE_DRAWTASKHIGHSCORE_H_


typedef struct highscore{
	uint16_t score;
	uint16_t score_remote;
	char tag[1][10];
} highscore_t;


#endif /* INCLUDE_DRAWTASKHIGHSCORE_H_ */
