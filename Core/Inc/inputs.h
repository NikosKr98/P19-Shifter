/*
 * input.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */



#ifndef INC_INPUTS_H_
#define INC_INPUTS_H_

#include "utils.h"


#define STEERING_RX_ID 0x310
#define ECU_RX_ID 0x311

#define TOTAL_GEARS 6
#define MAX_GEAR 5

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern CAN_HandleTypeDef hcan;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;

typedef struct {

	uint8_t  up_button_request;
	uint8_t  down_button_request;
	uint8_t  launch_button_request;
	uint16_t clutch_position;
	uint32_t gear_value1;
	uint32_t actual_gear;
	uint16_t adc_counter;

	uint8_t NGear;				// actual gear based on filtered gear potentiometer
	uint8_t BUpShiftRequest;	// steering wheel UpShift request (reflects the state of the paddle)
	uint8_t BDownShiftRequest;	// steering wheel DownShift request (reflects the state of the paddle)
	uint8_t BLaunchRequest;		// steering wheel Launch control  request (reflects the state of the button)
	int8_t rClutchPaddle;		// Steering wheel clutch paddle percentage (can be from -104% to 104% to allow margin)
	int16_t nEngine;			// engine RPM taken from the ECU

	uint8_t NCANErrors;			// CAN Receiving error count

} InputStruct;

void InitInputs(void);
void ReadInputs(InputStruct *input);

#endif /* INC_INPUTS_H_ */
