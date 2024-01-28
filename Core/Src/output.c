/*
 * output.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include "output.h"


void Output( ApplicationStruct *application){

	end_of_shift(application);


	if(application->up_button_out){
		application->target_gear++;

		if(application->target_gear!=application->current_gear+1){
			application->target_gear--;
		}

		if(application->target_gear > total_gears){
			application->target_gear=5;
		}
		else if(application->target_gear > 1 && application->target_gear <= 5){
			shiftup_activation(application);
		}
		else if(application->target_gear == 1 && application->clutch_detection){
			shiftdown_activation(application);
		}
		else {
			application->target_gear = application-> current_gear;
		}
	}


	if(application->down_button_out){
		application->target_gear--;

		if(application->target_gear!= application->current_gear-1){
			application->target_gear++;
		}

		if(application->target_gear < 0){
			application->target_gear=0;
			application->current_gear=application->target_gear;
		}
		else if(application->target_gear<1){
			neutral_activation(application);
		}
		else if(application->target_gear>=1){
			shiftdown_activation(application);
		}
	}


	if(msg_previous<current){

		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		TxData[0]=(application->down_port_state || application->up_port_state);
		TxData[1]=application->current_gear;

		msg_previous=current;
		msg_previous+=msg_interval;

		CAN_Tx(STEERING_ID,2,TxData);
	}
}

void CAN_Tx(uint32_t ID, uint8_t dlc, uint8_t* data) {

	uint32_t TxMailbox;

	TxHeader.DLC = dlc;
	TxHeader.StdId = ID;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;

	if(HAL_CAN_AddTxMessage(&hcan, &TxHeader, data, &TxMailbox) != HAL_OK) {
	}
}


void shiftup_activation(ApplicationStruct *application){ // Shift up function
	HAL_GPIO_WritePin(GPIOA, UP_PORT_Pin, GPIO_PIN_RESET);

	application->up_port_state = 1;

	shift_end_time = HAL_GetTick() + shifting_interval;
}

void shiftdown_activation(ApplicationStruct *application){ // Shift down function
	HAL_GPIO_WritePin(GPIOA, DOWN_PORT_Pin, GPIO_PIN_SET);

	application->down_port_state = 1;

	shift_end_time = HAL_GetTick() + shifting_interval;
}

void neutral_activation(ApplicationStruct *application){ //Neutral shift function
	HAL_GPIO_WritePin(GPIOA, UP_PORT_Pin, GPIO_PIN_RESET);

	application->up_port_state = 1;

	shift_end_time = HAL_GetTick() + neutral_interval;
}



void end_of_shift(ApplicationStruct *application) {  //Shift Handling

	// Check if a shift is in progress and if the delay has ended
	if ((application->up_port_state || application->down_port_state) && HAL_GetTick() >= shift_end_time) {

		// Reset the port action and update the current gear

		HAL_GPIO_WritePin(GPIOA, UP_PORT_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, DOWN_PORT_Pin, GPIO_PIN_RESET);

		application-> current_gear = application->target_gear;
		application->up_port_state = 0;
		application->down_port_state=0;
	}
}



