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
