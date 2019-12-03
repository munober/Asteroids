/*
 * buttons.h
 *
 *  Created on: Nov 14, 2019
 *      Author: lab_espl_stud04
 */

#ifndef INCLUDE_BUTTONS_H_
#define INCLUDE_BUTTONS_H_



struct coord {
	uint8_t x;
	uint8_t y;
};

typedef struct joystick_angle_pulse {
	struct coord axis;
	uint16_t angle;
	struct coord pulse;
}joystick_angle_pulse_t;

typedef struct buttons {
    struct coord joystick;
    struct coord joystick_direct;
    unsigned char A;
    unsigned char B;
    unsigned char C;
    unsigned char D;
    unsigned char E;
    unsigned char K;
} buttons_t;

typedef struct locked_buttons{
	SemaphoreHandle_t lock;
	buttons_t buttons;
}locked_buttons_t;

typedef struct locked_joystick{
	SemaphoreHandle_t lock;
	struct coord joystick;
}locked_joystick_t;


locked_buttons_t initButtons(void);
locked_joystick_t initJoystick(void);

#endif /* INCLUDE_BUTTONS_H_ */
