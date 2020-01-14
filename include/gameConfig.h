/*
 * gameConfig.h
 *
 *  Created on: Dec 18, 2019
 *      Author: Teodor Fratiloiu
 */

#ifndef INCLUDE_GAMECONFIG_H_
#define INCLUDE_GAMECONFIG_H_

// This number should have 6 digits. It will be displayed on the start menu.
#define BUILD_NUMBER                        101022

// Configuration of the state machine
#define STATE_COUNT					        11
#define MAIN_MENU_STATE  		 	        1
#define SINGLE_PLAYER_STATE			        2
#define PAUSE_MENU_STATE  			        3
#define CHEATS_MENU_STATE			        4
#define HIGHSCORE_DISPLAY_STATE		        5
#define HIGHSCORE_INTERFACE_STATE	        6
#define MULTIPLAYER_STATE			        7
#define SINGLE_PLAYER_LEVEL_2		        8
#define SINGLE_PLAYER_LEVEL_3		        9
#define PAUSE_MENU_LEVEL_2					10
#define PAUSE_MENU_LEVEL_3					11
#define STARTING_STATE                      SINGLE_PLAYER_LEVEL_3

#define STARTING_LIVES_LEVEL_ONE            3
#define STARTING_LIVES_LEVEL_TWO            6
#define STARTING_LIVES_LEVEL_THREE          900

// Speed of laser blaster shots
#define LASER_BLASTER_SPEED		   	            3
// Speed of saucer blaster
#define SAUCER_BLASTER_SPEED					1
// Speed of asteroids
#define ASTEROID_SPEED							0.5

// Inertia speed
#define INERTIA_SPEED_INITIAL_X                 4
#define INERTIA_SPEED_INITIAL_Y                 2
#define INERTIA_DECELERATE_X                    0.3
#define INERTIA_DECELERATE_Y                    0.1
#define INERTIA_MIN_SPEED_X                     0.7
#define INERTIA_MIN_SPEED_Y                     0.2
#define INERTIA_TIME_INCREMENT					200

// Score amounts for hits
#define POINTS_ASTEROID_SMALL                   300 // destroyed after 1 hit
#define POINTS_ASTEROID_MEDIUM                  200 // turns into 1 small
#define POINTS_ASTEROID_LARGE                   100 // turns into 1 medium
#define POINTS_SAUCER_HIT                       1000 // for 1 hit

// Number of asteroids to destroy in each level
#define TO_DESTROY_LEVEL_1_SMALL            20
#define TO_DESTROY_LEVEL_2_SMALL            20
#define TO_DESTROY_LEVEL_2_MEDIUM           5
#define TO_DESTROY_LEVEL_3_MEDIUM           9000
#define TO_DESTROY_LEVEL_3_LARGE            9000

// Respawn boundaries in each level (will respawn as long as number left is above this)
#define RESPAWN_SMALL_LEVEL_1               10
#define RESPAWN_SMALL_LEVEL_2               4
#define RESPAWN_SMALL_LEVEL_3               1
#define RESPAWN_MEDIUM_LEVEL_3              5
#define RESPAWN_LARGE_LEVEL_3               5

// How close the fired blaster shots have to get to the asteroids to register a hit
#define HIT_LIMIT_SHOT_SMALL				6
#define HIT_LIMIT_SHOT_MEDIUM				8
#define HIT_LIMIT_SHOT_LARGE				12

// How close the asteroids have to get to the player to register a hit
#define HIT_LIMIT_SMALL				        6
#define HIT_LIMIT_MEDIUM			        8
#define HIT_LIMIT_LARGE				        12 

// Score thresholds for going on to the next levels.
// After beating game, will be spawning asteroids to let player go on to reach a new high score
#define LEVEL_TWO_SCORE_THRESHOLD          8000
#define LEVEL_THREE_SCORE_THRESHOLD        18000
#define BEAT_GAME_SCORE_THRESHOLD          21000

// UART Configuration


#endif /* INCLUDE_GAMECONFIG_H_ */
