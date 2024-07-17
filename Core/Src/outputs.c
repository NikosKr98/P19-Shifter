/*
 * output.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include <Outputs.h>
#include <Maps.h>

// CAN

uint8_t CANTxBuffer[8] = {0};


uint32_t nCanTxErrorCount=0;
uint32_t nCanOldestMailbox=4, nCanSecondOldestMailbox=2, nCanYoungestMailbox=1;

// SERVO
uint16_t xClutchTargetOut;
uint16_t rServoDemand;

// PWM
// Timer Parameters
#define TIMER_COUTER_PERIOD 1000
#define TIMER_PULSE TIMER_COUTER_PERIOD/2

#define TIMER_MIN_PRESCALER 17
#define TIMER_MAX_PRESCALER 1000
uint32_t nTimerPrescaler;

// timing variables
uint32_t tOutputsTimer;

uint8_t TxData[8];

// private function declarations
void CAN_TX(uint32_t ID, uint8_t dlc, uint8_t* data);


void InitOutputs(void) {

	// we start the timer with initial target (CLUTCH_REST_POSITION) the released value (make the #define and also use it in the maps??)
	__HAL_TIM_SET_AUTORELOAD(&htim1, (CLUTCH_SERVO_MIN*2) -1 );
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, (CLUTCH_SERVO_MIN*2)/2);

	// set the duty cycle to 0 before enabling the PWM in order to avoid unwanted movement
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, 0);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
}

void WriteOutputs(InputStruct *inputs, OutputStruct *outputs) {

	tOutputsTimer = HAL_GetTick();

	// CLUTCH

	// we convert from mm to timer prescaler
	My2DMapInterpolate(CLUTCH_SERVO_MAP_SIZE, xClutchTarget_rServoDemandMap, outputs->xClutchTarget, &outputs->rServoDemandRaw, CLUTCH_TARGET_MIN_MARGIN, CLUTCH_TARGET_MAX_MARGIN);

	// convert from float to uint16_t
	outputs->rServoDemand  =(uint16_t)round(outputs->rServoDemandRaw);

	// Clamping to avoid out of bounds values
	outputs->rServoDemand = CLAMP(outputs->rServoDemand, CLUTCH_SERVO_ABSOLUTE_MAX, CLUTCH_SERVO_ABSOLUTE_MIN);

	// Actuated flag (it will be applied on the next cycle because it gets saved in the controller)
	outputs->BClutchActuated = (outputs->rServoDemand <= CLUTCH_SERVO_ACTUATED ? 1 : 0);

	// The output for the clutch servo is a +5V (or 3.3V) pulse 50% duty cycle 1500us +- 400us (1500 central position, 1900 or 1100 is fully pressed) to
	// we double the auto reload counter to multiply the frequency by 2
	// (the servo expects the pulse to be 900 - 2100 usec) so the period of the pulse needs to be the double,
	//since the duty cycle is 50%)
	outputs->rServoDemand *= 2;

	// think about not putting the duty cycle at 50% but to try and fine tune the compare and autoreload.
	// think about the auto preload function. It is now enabled, is it correct?
	// update the Timer Registers, using the TIM_Exported_Macros
	//__HAL_TIM_SET_PRESCALER(&htim1, nTimerPrescaler - 1);
	__HAL_TIM_SET_AUTORELOAD(&htim1, outputs->rServoDemand -1 );
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, outputs->rServoDemand/2);


	// Shifting

	// TODO: Think about doing a check if both requests are 1 in order to not do nothing or to always give priority to up or down shift
	HAL_GPIO_WritePin(DO02_GPIO_Port, DO02_Pin, outputs->BDnShiftPortState);
	HAL_GPIO_WritePin(DO03_GPIO_Port, DO03_Pin, outputs->BUpShiftPortState);


	// Toggle Switches
	// output->BSWLEDA

	uint8_t BLaunchButtonCANSW = output->BLaunchControl;
	uint8_t BSparkCutCANSW = output->BSparkCut;

	// ECU control


	// CAN

	// use the output->BUseButtonsForMultifunction to pop up the message for the multifunction in the screen
	// send the display index (remember it is already 0-based)
	// send the command for the outputs of the steering (LEDS) (think about sending frequency and duty instead of On-OFF, in order to have also the flashing action?

	// ---------------------------------------------------------------------------------------------------
	// Frame 1: Shifter Feedback

	TxData[0] = inputs->NGear;
	TxData[1] = inputs->rClutchPaddle;
	TxData[2] = (uint8_t)(outputs->xClutchTarget * 10) >> 8;
	TxData[3] = (uint8_t)outputs->xClutchTarget;
	TxData[4] = (uint8_t)(inputs->nEngine >> 8);
	TxData[5] = (uint8_t)inputs->nEngine;

	uint16_t VSupplyCAN = (uint16_t)(inputs->VSupply * 1000);

	TxData[6] = (uint8_t)(VSupplyCAN >> 8);
	TxData[7] = (uint8_t)VSupplyCAN;

	CAN_TX(SHIFTER_TX_ID01, 8, TxData);

	// ---------------------------------------------------------------------------------------------------
	// Frame 2: Shifter Control 1

	TxData[0] = (uint8_t)outputs->NDispalyPage;
	TxData[1] = outputs->NMultifunctionActiveSwitch;
	TxData[2] = (uint8_t)(outputs->NMultifunction[outputs->NMultifunctionActiveSwitch-1] + 1);
	TxData[3] = 0;
	TxData[4] = outputs->NDisplayFlags;

	TxData[5] = 0;
	TxData[5] |= outputs->BUseButtonsForMultifunction	>> 0;
	TxData[5] |= 0		>> 1;
	TxData[5] |= 0		>> 2;
	TxData[5] |= 0		>> 3;
	TxData[5] |= outputs->BSWLEDA						>> 4;
	TxData[5] |= outputs->BSWLEDB						>> 5;
	TxData[5] |= outputs->BSWLEDC						>> 6;
	TxData[5] |= outputs->BSWLEDD						>> 7;

	TxData[6] = inputs->NCANErrors;
	TxData[7] = inputs->NCANRxErrors;

	CAN_TX(SHIFTER_TX_ID02, 8, TxData);

	// ---------------------------------------------------------------------------------------------------
	// Frame 3: Shifter Control 2

	TxData[0] = 0;	// Reserved for ECU control
	TxData[1] = 0;
	TxData[2] = 0;
	TxData[3] = 0;
	TxData[4] = 0;
	TxData[5] = 0;
	TxData[6] = 0;
	TxData[7] = 0;

	CAN_TX(SHIFTER_TX_ID03, 8, TxData);

	// ---------------------------------------------------------------------------------------------------
	// Frame 4: Shifter Status

	TxData[0] = (uint8_t)(inputs->NInputsStatusWord >> 0);
	TxData[1] = (uint8_t)(inputs->NInputsStatusWord >> 8);
	TxData[2] = (uint8_t)(inputs->NInputsStatusWord >> 16);
	TxData[3] = (uint8_t)(inputs->NInputsStatusWord >> 24);

	TxData[4] = (uint8_t)(outputs->NControllerStatusWord >> 0);
	TxData[5] = (uint8_t)(outputs->NControllerStatusWord >> 8);
	TxData[6] = (uint8_t)(outputs->NControllerStatusWord >> 16);
	TxData[7] = (uint8_t)(outputs->NControllerStatusWord >> 24);

	CAN_TX(SHIFTER_TX_ID04, 8, TxData);

	// ---------------------------------------------------------------------------------------------------
	// Frame 5: ECU




	// ---------------------------------------------------------------------------------------------------
	// Frame 6: ECU	Switch Control

	TxData[0] = 0;
	TxData[1] = 0;
	TxData[2] = 0;
	TxData[3] = 0;
	TxData[4] = 0;
	TxData[5] = 0;

	TxData[6] = 0;
	TxData[6] |= (inputs->BLaunchRequest	& 0x01) << 0;
	TxData[6] |= (outputs->BSparkCut		& 0x01) << 1;
	TxData[6] |= (0						  	& 0x01) << 2;
	TxData[6] |= (0							& 0x01) << 3;
	TxData[6] |= (0							& 0x01) << 4;
	TxData[6] |= (0							& 0x01) << 5;
	TxData[6] |= (0							& 0x01) << 6;
	TxData[6] |= (0							& 0x01) << 7;

	TxData[7] = 0;

	CAN_TX(SHIFTER_TX_ID06, 8 , TxData);

	// ---------------------------------------------------------------------------------------------------

}

void CAN_TX(uint32_t ID, uint8_t dlc, uint8_t* data) {

	CAN_TxHeaderTypeDef CanTxHeader;
	uint32_t nCanTxMailbox;

	CanTxHeader.DLC = dlc;
	CanTxHeader.StdId = ID;
	CanTxHeader.IDE = CAN_ID_STD;
	CanTxHeader.RTR = CAN_RTR_DATA;

	__HAL_TIM_SET_COUNTER(&htim2, 0);
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
