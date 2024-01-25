/*
 * input.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include "input.h"

void Input_init(InputStruct *input){

	input->up_button_request=0;
	input->down_button_request=0;
	input->launch_button_request=0;
	input->adc_counter=0;
}

void Input_Read(InputStruct *input){

	HAL_ADC_PollForConversion(&hadc1,100);
	uint16_t gear_value = HAL_ADC_GetValue(&hadc1);

	input->adc_counter++;

	/* Normal Averaging */
	if(input->adc_counter < adc_counter_const){
		input->gear_value1 += gear_value;
	}
	else{
		input->actual_gear = input->gear_value1 / input->adc_counter;
		gear_value = 0;
		input->gear_value1=0;
		input->adc_counter = 0;
	}

	input->up_button_request = RxData[0]; //UP BUTTON REQUEST!

	input->down_button_request = RxData[1];//DOWN BUTTON REQUEST!

	input->clutch_position = (( RxData[2] << 8) | RxData[3]); // CLUTCH POSITION CALCUILATIONS!
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);

	switch(RxHeader.StdId) {

	 case FROM_STEERING_ID :

	 break;

	 }
}
