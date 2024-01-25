/*
 * application.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "input.h"

#define total_gears 5
#define clutch_detection_threshold 1000

typedef struct {
	int8_t current_gear;
	int8_t target_gear;
	uint8_t up_port_state;
	uint8_t down_port_state;
	uint32_t shift_end_time;
	uint8_t up_button_out;
	uint8_t down_button_out;
	uint16_t clutch_position;
	uint32_t actual_gear;
	uint8_t clutch_detection;
} ApplicationStruct;

extern uint32_t current, button_previous, button_interval;




void Application(InputStruct *input, ApplicationStruct *application);
void Application_init(ApplicationStruct *application);
uint16_t calculateActualGear(InputStruct *input);

#endif /* INC_APPLICATION_H_ */
