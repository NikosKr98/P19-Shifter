/*
 * inputs.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include "inputs.h"


// MACRO DEFINITIONS
#define PushEvent(ins_, event_) ins_->nEventStatus |= (1 << (uint32_t)(event_))
#define RaiseFault(ins_, fault_) ins_->nFaultStatus |= (1 << (uint32_t)(fault_))
#define ClearFault(ins_, fault_) ins_->nFaultStatus &= ~(1 << (uint32_t)(fault_))


// private variables

uint16_t NGearMap[TOTAL_GEARS][2] = {	// TODO: needs to go in maps.h file

    {3600, 3750},  // Gear 0
    {3900, 4000},  // Gear 1
    {3200, 3300},  // Gear 2
    {2250, 2400},  // Gear 3
    {1300, 1500},  // Gear 4
    {400, 600}     // Gear 5

};

// I/O Flags
uint8_t BUpShiftRequested=0, BDnShiftRequested=0, BLaunchRequested=0;

// CAN
uint8_t BUpShiftRequestCAN;
uint8_t BDnShiftRequestCAN;
uint8_t BLaunchRequestCAN;
int8_t rClutchPaddleCAN;
uint16_t nEngineCAN;

volatile uint8_t NCANErrorCount;
volatile uint16_t NCanGetRxErrorCount=0;

// ADC
uint16_t NGearRawFiltered;
volatile uint8_t NAdcBufferSide; // flag to determine the first or second half of the adc buffer for averaging

// private functions declaration
uint8_t calculateActualNGear(uint16_t NGear, uint16_t NGearRaw);
uint16_t MyHalfBufferAverage(uint16_t *buffer, uint16_t halfsize, uint8_t side);

void ReadInputs(InputStruct *inputs){

	// Reset events
	    inputs->nEventStatus = 0;


	// ---------------------------------------------------------------------------------------------------
		// NGear Conditioning

		// averaging
		NGearRawFiltered = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide);

		// mapping

		// TODO: the NGear cannot be in error because we use it for the control later on
				// possible solution is to create a continuous map i.e. 2.3~2nd and put it in error when we are in the middle of 2 gears
				// doing the average (in this case a round function) to determine the NGear from the raw value
				// the error will be if we exceed the up and down limits -> pot broken. in this case we need strategy to come back and use fully open loop control
				//currently if there is not a patch in the table we will be left with the last value which is not correct


		inputs->BNGearInError = calculateActualNGear(inputs->NGear, NGearRawFiltered);

		if(inputs->BNGearInError) {
			RaiseFault(inputs, NGEAR_IN_ERROR_FAULT);
		}
		else {
			ClearFault(inputs, NGEAR_IN_ERROR_FAULT);
		}
	// ---------------------------------------------------------------------------------------------------
		// Clutch Paddle Conditioning

		inputs->rClutchPaddleRaw = rClutchPaddleCAN;
		inputs->rClutchPaddle = CLAMP(inputs->rClutchPaddleRaw, CLUTCH_PADDLE_MIN, CLUTCH_PADDLE_MAX);
		// TODO: make a strategy for error detection (in rClutchPaddleRaw, or in possible extra  error variable)

	// ---------------------------------------------------------------------------------------------------
		// PCB Supply Voltage Conditioning

		//	inputs->VSupply = (float)ADC_VALUE * 3.3 / 4095 * (Voltage divider gain)
		// TODO: set the analog inputs and decide filtering (or not)

	// ---------------------------------------------------------------------------------------------------
		// Copy CAN data to struct

		inputs->BUpShiftRequest = BUpShiftRequestCAN;
		inputs->BDnShiftRequest = BDnShiftRequestCAN;
		inputs->BLaunchRequest = BLaunchRequestCAN;

		inputs->nEngine = nEngineCAN;

		inputs->NCANErrors = NCANErrorCount;			// update can error count
		inputs->NCANRxErrors = NCanGetRxErrorCount;	// update can Rx error count

	// ---------------------------------------------------------------------------------------------------
		// Events Parsing

		if(inputs->BUpShiftRequest && !BUpShiftRequested) {
			BUpShiftRequested = 1;
			PushEvent(inputs, UPSHIFT_PRESS_EVT);
		}
		else if(!inputs->BUpShiftRequest && BUpShiftRequested) {
			BUpShiftRequested = 0;
		}

		if(inputs->BDnShiftRequest && !BDnShiftRequested) {
			BDnShiftRequested = 1;
			PushEvent(inputs, DNSHIFT_PRESS_EVT);
		}
		else if(!inputs->BDnShiftRequest && BDnShiftRequested) {
			BDnShiftRequested = 0;
		}

		if(inputs->BLaunchRequest && !BLaunchRequested) {
			BLaunchRequested = 1;
			PushEvent(inputs, LAUNCH_PRESS_EVT);
		}
		else if(!inputs->BLaunchRequest && BLaunchRequested) {
			BLaunchRequested = 0;
		}

		if(inputs->rClutchPaddle > CLUTCH_PADDLE_PRESSED_THRESHOLD) {
			PushEvent(inputs, CLUTCH_PADDLE_PRESS_EVT);
		}

		if(inputs->BUpShiftRequest && inputs->BDnShiftRequest) {
			RaiseFault(inputs, BOTH_PADS_PRESSED_FAULT);
		}
		else {
			ClearFault(inputs, BOTH_PADS_PRESSED_FAULT);
		}

	// ---------------------------------------------------------------------------------------------------





}

void InitInputs(void){
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcRawValue, ADC_BUFFER_SIZE);
}

uint8_t CheckFaults(InputStruct *inputs) {
	if(inputs->nFaultStatus) return 1;
	return 0;
}

void CAN_RX(CAN_HandleTypeDef *hcan, uint32_t RxFifo) {

	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxBuffer[8];

	if(HAL_CAN_GetRxMessage(hcan, RxFifo, &RxHeader, RxBuffer) != HAL_OK) {
		NCanGetRxErrorCount++;
		return;
	}

	 //Don't forget to add and enable filters for each message
	switch(RxHeader.StdId) {

	 case STEERING_RX_ID :

		BUpShiftRequestCAN = RxBuffer[0];
		BDnShiftRequestCAN = RxBuffer[1];
		BLaunchRequestCAN = RxBuffer[2];
		rClutchPaddleCAN = RxBuffer[3];

		break;

	 case ECU_RX_ID:

		 nEngineCAN = RxBuffer[0] << 8 | RxBuffer[1];

		 break;

	 default:
		 break;
	 }
}


uint8_t calculateActualNGear(uint16_t NGear, uint16_t NGearRaw) {

    for (uint8_t gear = 0; gear < TOTAL_GEARS; ++gear) {
        if (NGearRaw >= NGearMap[gear][0] && NGearRaw <= NGearMap[gear][1]) {
        	NGear = gear;
        	return 0;
        }
    }
    return 1; // If no match found, return error!
}


uint16_t MyHalfBufferAverage(uint16_t *buffer, uint16_t halfsize, uint8_t side) {

	uint32_t Accumulator=0;
	uint16_t Offset = (side == 1 ? halfsize : 0);

	for(uint16_t i=0; i<halfsize; i++) {
		Accumulator += buffer[i + Offset];
	}

	Accumulator /= halfsize;
	return (uint16_t)Accumulator;

}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	CAN_RX(hcan, CAN_RX_FIFO0);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	CAN_RX(hcan, CAN_RX_FIFO1);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
	NCANErrorCount++;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
	// we enter here every time ADC_BUFFER_SIZE/2 samples have been moved to the adcRawValue buffer by the DMA

	if(hadc == &hadc1) {
		NAdcBufferSide ^= 1;	// changes from 0 to 1
	}
}
