/*
 * output.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include <Outputs.h>

// CAN
uint32_t nCanTxErrorCount=0;
uint32_t nCanOldestMailbox=4, nCanSecondOldestMailbox=2, nCanYoungestMailbox=1;
volatile uint8_t bCanTxMailboxEmpty[3] = {1,1,1};


//  private variables
uint32_t msg_interval, msg_previous, button_previous, button_interval, msg_interval,shift_end_time;
uint8_t TxData[8];

// private function declarations
void CAN_TX(uint32_t ID, uint8_t dlc, uint8_t* data);
void shiftup_activation(OutputStruct *output);
void shiftdown_activation(OutputStruct *output);
void neutral_activation(OutputStruct *output);
void end_of_shift(OutputStruct *output);


void InitOutputs(void) {

	// TODO: start the timer with initial target the released value (make the #define and also use it in the maps??)
}

void WriteOutputs(OutputStruct *output) {

	// CLUTCH

	// Do a clipping on the xClutchTarget to make sure we do not exceed the servo min and max values
	// put the target directly in the timer period function
	// The output for the clutch servo is a +5V pulse 50% dutycycle 1500us +- 400us (1500 central position, 1900 or 1100 is fully pressed) to



	// Shifting Ports
	// remember: Upshift: activated when writing 0 and not activating when writing 1
	//			 Dnshift: activated when writing 1 and not activating when writing 0

	HAL_GPIO_WritePin(GPIOA, UP_PORT_Pin, !output->BUpShiftPortState);
	HAL_GPIO_WritePin(GPIOA, DOWN_PORT_Pin, output->BDnShiftPortState);



	// CAN


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


void CAN_TX(uint32_t ID, uint8_t dlc, uint8_t* data) {

	CAN_TxHeaderTypeDef CanTxHeader;
	uint32_t nCanTxMailbox;

	CanTxHeader.DLC = dlc;
	CanTxHeader.StdId = ID;
	CanTxHeader.IDE = CAN_ID_STD;
	CanTxHeader.RTR = CAN_RTR_DATA;

	uint32_t wait = __HAL_TIM_GET_COUNTER(&htim2) + CAN_TX_TIMEOUT;
	while((HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0) && (__HAL_TIM_GET_COUNTER(&htim2) < wait));

	if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0) {	// all mailboxes are still filled
		HAL_CAN_AbortTxRequest(&hcan, nCanOldestMailbox);
	}

	if(HAL_CAN_AddTxMessage(&hcan, &CanTxHeader, data, &nCanTxMailbox) != HAL_OK) {
		print("Failed to Add Message can 1\n");
		nCanTxErrorCount++;
		return;
	}

	// Mailbox aging adjustment
	if(nCanTxMailbox != nCanYoungestMailbox) {

		if(nCanTxMailbox != nCanSecondOldestMailbox) {
			nCanOldestMailbox = nCanSecondOldestMailbox;
			nCanSecondOldestMailbox = nCanYoungestMailbox;
			nCanYoungestMailbox = nCanTxMailbox;
		}
		else {
			nCanSecondOldestMailbox = nCanYoungestMailbox;
			nCanYoungestMailbox = nCanTxMailbox;
		}
	}

}
