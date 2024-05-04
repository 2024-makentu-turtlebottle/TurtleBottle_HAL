/*
 * get_slope.h
 *
 *  Created on: May 5, 2024
 *      Author: czli
 */

#ifndef INC_GET_SLOPE_H_
#define INC_GET_SLOPE_H_

#include "arm_math.h"
#include "stdbool.h"
#include "math.h"

#define HEIGHT 80
#define WIDTH 60
#define CHANNELS 3

#define DISTANCE 11

#define NUM_DATA_MAX 50

extern unsigned char data[HEIGHT][WIDTH][CHANNELS];
extern unsigned char dst[HEIGHT][WIDTH];

float32_t get_slope();

// 定義像素點結構
typedef struct {
    int x;
    int y;
} Point;

// 定義隊列結構
typedef struct {
    Point items[WIDTH * HEIGHT];
    int front;
    int rear;
} Queue;

#endif /* INC_GET_SLOPE_H_ */
