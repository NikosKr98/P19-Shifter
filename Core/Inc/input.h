/*
 * input.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */



#ifndef INC_INPUT_H_
#define INC_INPUT_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"
#define adc_counter_const 1000
#define FROM_STEERING_ID 0x310

typedef struct {
	uint8_t  up_button_request;
	uint8_t  down_button_request;
	uint8_t  launch_button_request;
	uint16_t clutch_position;
	uint32_t gear_value1;
	uint32_t actual_gear;
	uint16_t adc_counter;
	uint8_t up;
	uint8_t down;

} InputStruct;

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern CAN_HandleTypeDef hcan;

extern CAN_FilterTypeDef FilterConfig0;
extern CAN_RxHeaderTypeDef RxHeader;
extern uint8_t RxData[8];

extern uint32_t current, button_previous, button_interval;


void Input_init(InputStruct *input);
void Input_Read(InputStruct *input);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

#endif /* INC_INPUT_H_ */
