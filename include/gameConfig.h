/*
 * gameConfig.h
 *
 *  Created on: Dec 18, 2019
 *      Author: lab_espl_stud04
 */

#ifndef INCLUDE_GAMECONFIG_H_
#define INCLUDE_GAMECONFIG_H_


#define LASER_BLASTER_SPEED			        3

// How close the fired blaster shots have to get to the asteroids to register a hit
#define HIT_LIMIT_SHOT_SMALL				6
#define HIT_LIMIT_SHOT_MEDIUM				8
#define HIT_LIMIT_SHOT_LARGE				12

// How close the asteroids have to get to the player to register a hit
#define HIT_LIMIT_SMALL				        6
#define HIT_LIMIT_MEDIUM			        8
#define HIT_LIMIT_LARGE				        12 

// Score thresholds for going on to the next levels
// After beating game, will be spawning asteroids to let player go on to reach a new high score
#define LEVEL_TWO_SCORE_THRESHOLD          4000
#define LEVEL_THREE_SCORE_THRESHOLD        12000
#define BEAT_GAME_SCORE_THRESHOLD          24000



#endif /* INCLUDE_GAMECONFIG_H_ */
