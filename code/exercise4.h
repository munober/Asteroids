/*
 * exercise4.h
 *
 *  Created on: Nov 18, 2019
 *      Author: lab_espl_stud04
 */

#ifndef CODE_EXERCISE4_H_
#define CODE_EXERCISE4_H_

typedef struct ex4output {
    char print[15][100];
} ex4output_t;

typedef struct locked_ex4output {
	SemaphoreHandle_t lock;
	ex4output_t ex4output;
} locked_ex4output_t;

locked_ex4output_t initEx4Output(void);

#endif /* CODE_EXERCISE4_H_ */
