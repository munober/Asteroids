/*
 * gameConfig.h
 *
 *  Created on: Dec 18, 2019
 *      Author: Teodor Fratiloiu
 *      		Frederik Zumegen
 */

#ifndef INCLUDE_GAMECONFIG_H_
#define INCLUDE_GAMECONFIG_H_

// This number should have 6 digits. It will be displayed on the start menu.
#define BUILD_NUMBER                        696969

#define FRAMTERATE							50
#define FPS_PERIOD							1000 / FRAMTERATE

// Configuration of the state machine
#define STATE_COUNT					        8
#define MAIN_MENU_STATE  		 	        1
#define SINGLE_PLAYER_STATE			        2
#define CHEATS_MENU_STATE			        3
#define HIGHSCORE_DISPLAY_STATE		        4
#define HIGHSCORE_INTERFACE_STATE	        5
#define MULTIPLAYER_STATE			        6
#define SINGLE_PLAYER_LEVEL_2		        7
#define SINGLE_PLAYER_LEVEL_3		        8

#define STARTING_STATE                      MAIN_MENU_STATE

#define STARTING_LIVES_LEVEL_ONE            3
#define STARTING_LIVES_LEVEL_TWO            6
#define STARTING_LIVES_LEVEL_THREE          9
#define STARTING_LIVES_MULTIPLAYER	    	20

// Speed of laser blaster shots
#define LASER_BLASTER_SPEED		   	            3
// Speed of saucer blaster
#define SAUCER_BLASTER_SPEED					1
// Speed of asteroids
#define ASTEROID_SPEED							0.5

// Inertia and movement
#define SPEED_SCALING_FACTOR					32 // dividing joystick input by this factor
#define INERTIA_SPEED_INITIAL_X                 3 // starting inertial movement spped on x axis
#define INERTIA_SPEED_INITIAL_Y                 2 // starting inertial movement spped on y axis
#define INERTIA_DECELERATE_X                    0.3 // rate of inertial decelaration on x axis
#define INERTIA_DECELERATE_Y                    0.1 // rate of inertial decelaration on y axis
#define INERTIA_MIN_SPEED_X                     0.4 // minimum movement speed on x axis
#define INERTIA_MIN_SPEED_Y                     0.3 // minimum movement speed on y axis
#define INERTIA_TIME_INCREMENT					100 // period at which the object will decelerate with above rates
#define ROTATION_SPEED							8 // in degrees per frame
#define	PLAYER_SPEED_MAX_X						5 // maximum reachable speed on x axis
#define	PLAYER_SPEED_MAX_Y						5 // maximum reachable speed on y axis
#define PLAYER_STANDARD_ACCELERATION_X			0.1 // rate of acceleration on joystick input on x axis
#define PLAYER_STANDARD_ACCELERATION_Y			0.1 // rate of acceleration on joystick input on y axis

// UART
#define UART_CONNECTED_BYTE						0x55
#define UART_STANDARD_BYTE						0xAA
#define ONE_FRAME_DURATION						20 // milsecs
#define UART_SEND_ALIVE_PERIOD					ONE_FRAME_DURATION
#define UART_CHECK_ALIVE_PERIOD					ONE_FRAME_DURATION * 50

// Score amounts for hits
#define POINTS_ASTEROID_SMALL                   300 // destroyed after 1 hit
#define POINTS_ASTEROID_MEDIUM                  200 // turns into 1 small
#define POINTS_ASTEROID_LARGE                   100 // turns into 1 medium
#define POINTS_SAUCER_HIT                       1000 // for 1 hit

// Number of asteroids to destroy in each level
#define TO_DESTROY_LEVEL_1_SMALL            20
#define TO_DESTROY_LEVEL_2_SMALL            20
#define TO_DESTROY_LEVEL_2_MEDIUM           5
#define TO_DESTROY_LEVEL_3_SMALL			60
#define TO_DESTROY_LEVEL_3_MEDIUM           30
#define TO_DESTROY_LEVEL_3_LARGE            10
#define TO_DESTROY_MULTI_SMALL				60
#define TO_DESTROY_MULTI_MEDIUM           	30
#define TO_DESTROY_MULTI_LARGE            	10

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

// MULTIPLAYER
#define HIT_LIMIT_PLAYER_SHOT_MULTI				8

// Saucers
#define SAUCER_1_SPAWN_TIME					20
#define SAUCER_2_SPAWN_TIME					35

// Score thresholds for going on to the next levels.
// After beating game, will be spawning asteroids to let player go on to reach a new high score
#define LEVEL_TWO_SCORE_THRESHOLD          8000
#define LEVEL_THREE_SCORE_THRESHOLD        18000
#define BEAT_GAME_SCORE_THRESHOLD          29000

#define START_MENU_SCORE_INCREMENT			4000

#define GET_MORE_LIVES_LEVEL_ONE			4000
#define GET_MORE_LIVES_LEVEL_TWO			14000
#define GET_MORE_LIVES_LEVEL_THREE			20000

// UART Configuration

// Misc
#define SHOW_DEBUG_LVL_1					0
#define SHOW_DEBUG_LVL_2					0
#define SHOW_DEBUG_LVL_3					0
// This actually not needed thanks to fancy live switch by double pressing BUT_C to show/hide this line
#define SHOW_DEBUG_MULTI					0
#define SHOW_DEBUG_MAINMENU					0


#endif /* INCLUDE_GAMECONFIG_H_ */
