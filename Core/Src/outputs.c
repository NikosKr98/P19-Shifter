/*
 * output.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include <Outputs.h>

// CAN

uint8_t CANTxBuffer[8] = {0};


uint32_t nCanTxErrorCount=0;
uint32_t nCanOldestMailbox=4, nCanSecondOldestMailbox=2, nCanYoungestMailbox=1;

// CLUTCH
uint16_t xClutchTargetOut;


// PWM
// Timer Parameters
#define TIMER_COUTER_PERIOD 1000
#define TIMER_PULSE TIMER_COUTER_PERIOD/2

#define TIMER_MIN_PRESCALER 17
#define TIMER_MAX_PRESCALER 1000
uint32_t nTimerPrescaler;


// private function declarations
void CAN_TX(uint32_t ID, uint8_t dlc, uint8_t* data);
void shiftup_activation(OutputStruct *output);
void shiftdown_activation(OutputStruct *output);
void neutral_activation(OutputStruct *output);
void end_of_shift(OutputStruct *output);


void InitOutputs(void) {

	// TODO: start the timer with initial target (CLUTCH_REST_POSITION) the released value (make the #define and also use it in the maps??)

	// set the duty cycle to 0 before enabling the PWM in order to avoid unwanted movement
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, 0);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
}

void WriteOutputs(OutputStruct *output) {

	// CLUTCH

	// Clamping to avoid out of bounds values
	xClutchTargetOut = CLAMP(output->xClutchTarget, xCLUTCH_ABSOLUTE_MIN, xCLUTCH_ABSOLUTE_MAX);

	// The output for the clutch servo is a +5V (or 3.3V) pulse 50% duty cycle 1500us +- 400us (1500 central position, 1900 or 1100 is fully pressed) to

	// we double the auto reload counter to multiply the frequency by 2
	// (the servo expects the pulse to be 900 - 2100 usec) so the period of the pulse needs to be the double,
	//since the duty cycle is 50%)
	xClutchTargetOut *= 2;

	// think about not putting the duty cycle at 50% but to try and fine tune the compare and autoreload.
	// think about the auto preload function. It is now enabled, is it correct?
	// update the Timer Registers, using the TIM_Exported_Macros
//	__HAL_TIM_SET_PRESCALER(&htim1, nTimerPrescaler - 1);
	__HAL_TIM_SET_AUTORELOAD(&htim1, xClutchTargetOut -1 );
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, xClutchTargetOut/2);


	// Shifting Ports
	// TODO: Think about doing a check if both requests are 1 in order to not do nothing or to always give priority to up or down shift
	HAL_GPIO_WritePin(DO03_GPIO_Port, DO03_Pin, output->BUpShiftPortState);
	HAL_GPIO_WritePin(DO02_GPIO_Port, DO02_Pin, output->BDnShiftPortState);


	// Toggle Switches
	// output->BSWLEDA

	uint8_t BLaunchButtonCANSW = output->BLaunchControl;
	uint8_t BSparkCutCANSW = output->BSparkCut;


	CANTxBuffer[6] = 0;
	CANTxBuffer[6] |= (BLaunchButtonCANSW				& 0x01) << 0;
	CANTxBuffer[6] |= (BSparkCutCANSW 					& 0x01) << 1;
	CANTxBuffer[6] |= (0							  	& 0x01) << 2;
	CANTxBuffer[6] |= (0								& 0x01) << 3;
	CANTxBuffer[6] |= (0							    & 0x01) << 4;
	CANTxBuffer[6] |= (0								& 0x01) << 5;
	CANTxBuffer[6] |= (0								& 0x01) << 6;
	CANTxBuffer[6] |= (0								& 0x01) << 7;


	CAN_TX(ECU_SWITCH_TX_ID, 8 , CANTxBuffer);
	// CAN
	//	CAN_TX(ECU_SWITCH_TX_ID, 8 , CANTxBuffer);
	// add the info of the shutDown (DriverKill) in the CAN for others to see
	// send the command for the outputs of the steering (LEDS) (think about sending frequency and duty instead of On-OFF
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
