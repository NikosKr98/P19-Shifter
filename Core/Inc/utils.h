/*
 * utils.h
 *
 *  Created on: Jan 28, 2024
 *      Author: orestis
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include "stm32f1xx_hal.h"
#include "main.h"

#define USB_DEBUG 0					// debug information through ST-Link Virtual COM Port (1 = enable, 0 = disable)

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1;

// MACRO DEFINITIONS
#define BIT(var_,pos_)     ((var_ >> pos_) & 0x1)
#define LOGNOT(bit_)       ((bit_ ^ 0x1) & 0x1)
#define MIN(a, b) 			((a) < (b) ? (a) : (b))
#define MAX(a,b)           (a > b ? a : b)
#define CLAMP(x,l,h)       (MIN(MAX(x,l),h))
#define ABS(a)             (a > 0 ? a : (-a))


#define BYTE_TO_BIN_PAT "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BIN(byte)		\
	(byte & 0x80 ? '1' : '0'),	\
	(byte & 0x40 ? '1' : '0'),	\
	(byte & 0x20 ? '1' : '0'),	\
	(byte & 0x10 ? '1' : '0'),	\
	(byte & 0x08 ? '1' : '0'),	\
	(byte & 0x04 ? '1' : '0'),	\
	(byte & 0x02 ? '1' : '0'),	\
	(byte & 0x01 ? '1' : '0')


// FUNCTION PROTOTYPES
void print(char *msg, ...);
uint8_t My2DMapInterpolate(int size, const float map[][size], float input, float *output, float minMargin, float maxMargin);


#endif /* INC_UTILS_H_ */
