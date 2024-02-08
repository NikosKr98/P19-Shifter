/*
 * utils.c
 *
 *  Created on: Jan 28, 2024
 *      Author: orestis
 */

#include <Utils.h>

void print(char *msg, ...) {

	char buff[100];
	va_list args;
	va_start(args, msg);
	vsprintf(buff, msg, args);
	va_end(args);

#if USB_DEBUG
	HAL_UART_Transmit(&huart3, (uint8_t *)buff, strlen(buff), 10);
#endif
}

uint8_t My2DMapInterpolate(int size, const float map[][size], float input, float *output, float minMargin, float maxMargin) {
	float dx, dy;
	int i;

	if(input < map[0][0] - minMargin) {
		// if input is less than the smaller element of the map minus a small margin,
		// we declare the input in error and assign the min value of the map
		*output = map[1][0];
		return 1;
	}
	if(input > map[0][size-1] + maxMargin) {
		// if input is greater than the largest element of the map plus a small margin,
		// we declare the input in error and assign the max value of the map
		*output = map[1][size-1];
		return 1;
	}

	// we find i so that map[0][i] < input < map[0][i+1]
	for(i=0; i<size-1; i++) {
		if(map[0][i+1] > input)
			break;
	}

	// we interpolate
	dx = map[0][i+1] - map[0][i];
	dy = map[1][i+1] - map[1][i];

	*output = (float)(map[1][i] + (input - map[0][i]) * dy/dx);
	return 0;
}
