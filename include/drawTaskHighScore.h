/*
 * stateMachineTask.h
 *
 *  Created on: Dec 2, 2019
 *      Author: Teodor Fratioiu
 *      		Frederik Zumegen
 */

#ifndef INCLUDE_DRAWTASKHIGHSCORE_H_
#define INCLUDE_DRAWTASKHIGHSCORE_H_


typedef struct highscore{
	uint16_t score;
	uint16_t score_remote;
	char tag[1][10];
} highscore_t;


#endif /* INCLUDE_DRAWTASKHIGHSCORE_H_ */
