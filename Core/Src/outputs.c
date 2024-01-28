/*
 * output.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include "outputs.h"

//  private variables
uint32_t msg_interval, msg_previous, button_previous, button_interval, msg_interval,shift_end_time;
uint8_t TxData[8];

// private function declarations
void CAN_Tx(uint32_t ID, uint8_t dlc, uint8_t* data);
void shiftup_activation(OutputStruct *output);
void shiftdown_activation(OutputStruct *output);
void neutral_activation(OutputStruct *output);
void end_of_shift(OutputStruct *output);


void InitOutputs(void) {

}

void WriteOutputs(OutputStruct *output) {

	end_of_shift(output);


	if(output->up_button_out){
		output->target_gear++;

		if(output->target_gear!=output->current_gear+1){
			output->target_gear--;
		}

		if(output->target_gear > TOTAL_GEARS){
			output->target_gear=5;
		}
		else if(output->target_gear > 1 && output->target_gear <= 5){
			shiftup_activation(output);
		}
		else if(output->target_gear == 1 && output->clutch_detection){
			shiftdown_activation(output);
		}
		else {
			output->target_gear = output-> current_gear;
		}
	}


	if(output->down_button_out){
		output->target_gear--;

		if(output->target_gear!= output->current_gear-1){
			output->target_gear++;
		}

		if(output->target_gear < 0){
			output->target_gear=0;
			output->current_gear=output->target_gear;
		}
		else if(output->target_gear<1){
			neutral_activation(output);
		}
		else if(output->target_gear>=1){
			shiftdown_activation(output);
		}
	}





		TxData[0]=(output->down_port_state || output->up_port_state);
		TxData[1]=output->current_gear;

		CAN_Tx(STEERING_ID,2,TxData);

}

void CAN_Tx(uint32_t ID, uint8_t dlc, uint8_t* data) {

	CAN_TxHeaderTypeDef TxHeader;

	uint32_t TxMailbox;

	TxHeader.DLC = dlc;
	TxHeader.StdId = ID;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;

	// TODO: fix CANTx with round robin
	if(HAL_CAN_AddTxMessage(&hcan, &TxHeader, data, &TxMailbox) != HAL_OK) {

	}

}


void shiftup_activation(OutputStruct *output){ // Shift up function
	HAL_GPIO_WritePin(GPIOA, UP_PORT_Pin, GPIO_PIN_RESET);

	output->up_port_state = 1;

	shift_end_time = HAL_GetTick() + shifting_interval;
}

void shiftdown_activation(OutputStruct *output){ // Shift down function
	HAL_GPIO_WritePin(GPIOA, DOWN_PORT_Pin, GPIO_PIN_SET);

	output->down_port_state = 1;

	shift_end_time = HAL_GetTick() + shifting_interval;
}

void neutral_activation(OutputStruct *output){ //Neutral shift function
	HAL_GPIO_WritePin(GPIOA, UP_PORT_Pin, GPIO_PIN_RESET);

	output->up_port_state = 1;

	shift_end_time = HAL_GetTick() + neutral_interval;
}



void end_of_shift(OutputStruct *output) {  //Shift Handling

	// Check if a shift is in progress and if the delay has ended
	if ((output->up_port_state || output->down_port_state) && HAL_GetTick() >= shift_end_time) {

		// Reset the port action and update the current gear

		HAL_GPIO_WritePin(GPIOA, UP_PORT_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, DOWN_PORT_Pin, GPIO_PIN_RESET);

		output-> current_gear = output->target_gear;
		output->up_port_state = 0;
		output->down_port_state=0;
	}
}



