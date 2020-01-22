/*
 * stateMachineTask.h
 *
 *  Created on: Dec 2, 2019
 *      Author: Teodor Fratioiu
 *      		Frederik Zumegen
 */

#ifndef INCLUDE_DRAWTASKHIGHSCOREINTERFACE_H_
#define INCLUDE_DRAWTASKHIGHSCOREINTERFACE_H_


typedef struct score{
	uint16_t score;
	char name[1][6];
}score_t;


#endif /* INCLUDE_DRAWTASKHIGHSCOREINTERFACE_H_ */
