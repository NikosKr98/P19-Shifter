/*
 * inputs.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include <Inputs.h>
#include <Maps.h>

// MACRO DEFINITIONS
#define PushEvent(ins_, event_) ins_->nEventStatus |= (1 << (uint32_t)(event_))
#define RaiseFault(ins_, fault_) ins_->nFaultStatus |= (1 << (uint32_t)(fault_))
#define ClearFault(ins_, fault_) ins_->nFaultStatus &= ~(1 << (uint32_t)(fault_))

// Timing variables
uint32_t tCurrent;

// I/O Flags
uint8_t BUpShiftRequested=0, BDnShiftRequested=0, BLaunchRequested=0, BEmergencyRequested=0;

// CAN
volatile uint8_t BUpShiftButtonCAN, BUpShiftButtonCANInError;;
volatile uint8_t BDnShiftButtonCAN, BDnShiftButtonCANInError;
volatile uint8_t BLaunchRequestCAN, BLaunchButtonCANInError;
volatile uint8_t BEmergencyButtonCAN, BEmergencyButtonCANInError;
volatile int8_t rClutchPaddleRawCAN, BrClutchPaddleRawInErrorCAN;
volatile uint16_t nEngineRawCAN;
volatile uint32_t tCANSteeringWheelLastSeen;
volatile uint32_t tCANECULastSeen;

volatile uint8_t NCANErrorCount;
volatile uint16_t NCanGetRxErrorCount=0;

// Analog
uint16_t NGearRawADCFiltered;
volatile uint8_t NAdcBufferSide; // flag to determine the first or second half of the adc buffer for averaging
int8_t rClutchPaddleRaw;


// private functions declaration
uint8_t calculateActualNGear(uint16_t NGear, uint16_t NGearRaw);
uint16_t MyHalfBufferAverage(uint16_t *buffer, uint16_t halfsize, uint8_t side);

void ReadInputs(InputStruct *inputs){

	// Reset events
	    inputs->nEventStatus = 0;

	    tCurrent = HAL_GetTick();

	    // TODO: we need to think the order of execution of the inputs (now they are a bit random)
	    // check if there are dependencies

	    // TODO: think about putting the analog read (buckup) buttons inside the CAN error state to gain some time during normal running
	    // or, do them at the same time and compare inputs

	// ---------------------------------------------------------------------------------------------------
		// Driver Kill

		//	TODO: inputs->BDriverKill = digital read... (debouncing)

	// ---------------------------------------------------------------------------------------------------
	    // NGear Conditioning

		// ADC averaging
		NGearRawADCFiltered = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide);

		// voltage conversion
		inputs->VNGearRaw = (float)(NGearRawADCFiltered * 3.3 / 4095.0);

		// mapping
		inputs->BNGearInError = My2DMapInterpolate(TOTAL_GEARS, NGearMap, inputs->VNGearRaw, &(inputs->NGearRaw), VNGEAR_MARGIN_MIN, VNGEAR_MARGIN_MAX);

		// TODO: think about checking the float NGear for +-0.2 to define false neutral

		// conditioning (round float to nearest integer)
		inputs->NGear = (uint8_t)round(inputs->NGearRaw);

		// CLAMPING
		inputs->NGear = CLAMP(inputs->NGear, 0, MAX_GEAR);

		// check for errors
		if(inputs->BNGearInError) {
			RaiseFault(inputs, NGEAR_FAULT);
			// inputs->NGear = 1; // TODO: is it correct??? not sure. I would put 1 to be able trigger antistall and to be generic for all functions
		}
		else ClearFault(inputs, NGEAR_FAULT);

	// ---------------------------------------------------------------------------------------------------
		// Steering Wheel Fitted Check

		if((tCANSteeringWheelLastSeen + STEERING_WHEEL_FITTED_INTERVAL) < tCurrent) {
			inputs->BSteeringWheelFitted = 0;
			RaiseFault(inputs, STEERING_WHEEL_FAULT);
		}
		else {
			inputs->BSteeringWheelFitted = 1;
			ClearFault(inputs, STEERING_WHEEL_FAULT);
		}

	// ---------------------------------------------------------------------------------------------------
		// Emergency Button Conditioning

		// CAN Input
		inputs->BEmergencyButtonCANInError = BEmergencyButtonCANInError;
		inputs->BEmergencyButtonCAN = BEmergencyButtonCAN;

		// Analog Input
				// TODO: digital read with debounce
				// do the window up and down checks and define if in the correct range
				// define if in error


		// Emergency Input Strategy
		if(inputs->BSteeringWheelFitted && !inputs->BEmergencyButtonCANInError) {
			inputs->BEmergencyRequest = inputs->BEmergencyButtonCAN;
			inputs->NBEmergencyRequestSource = CAN;
			inputs->BEmergencyRequestInError = 0;
		}
		else if(!inputs->BEmergencyButtonAnalogInError) {
			inputs->BEmergencyRequest = inputs->BEmergencyButtonAnalog;
			inputs->NBEmergencyRequestSource = Analog;
			inputs->BEmergencyRequestInError = 0;
		}
		else {
			inputs->BEmergencyRequestInError = 1;
			inputs->BEmergencyRequest = 0;		// we force to zero if in error
		}

	// ---------------------------------------------------------------------------------------------------
		// Clutch Paddle Conditioning

		// CAN Input
		inputs->BrClutchPaddleRawCANInError = BrClutchPaddleRawInErrorCAN;
		inputs->rClutchPaddleRawCAN = rClutchPaddleRawCAN;

		// Analog Input
		// TODO: analog read, convert to voltage and map to -x% 10x%

		// Clutch Paddle Input Strategy
		if(inputs->BSteeringWheelFitted && !inputs->BrClutchPaddleRawCANInError) {
			rClutchPaddleRaw = inputs->rClutchPaddleRawCAN;
			inputs->NrClutchPaddleSource = CAN;
			inputs->BrClutchPaddleInError = 0;

		}
		else if(!inputs->BrClutchPaddleRawAnalogInError) {
			rClutchPaddleRaw = inputs->rClutchPaddleRawAnalog;
			inputs->NrClutchPaddleSource = Analog;
			inputs->BrClutchPaddleInError = 0;
		}
		else {
			inputs->BrClutchPaddleInError = 1;
			rClutchPaddleRaw = (inputs->BEmergencyRequest == 1 ? 100 : 0);	// we use the extra button to fully press the clutch
		}

		// CLAMPING
		inputs->rClutchPaddle = CLAMP(rClutchPaddleRaw, CLUTCH_PADDLE_MIN, CLUTCH_PADDLE_MAX);


	// ---------------------------------------------------------------------------------------------------
		// Up-Dn Shift Conditioning

		// CAN Input
		inputs->BUpShiftButtonCANInError = BUpShiftButtonCANInError;
		inputs->BDnShiftButtonCANInError = BDnShiftButtonCANInError;
		inputs->BUpShiftButtonCAN = BUpShiftButtonCAN;
		inputs->BDnShiftButtonCAN = BDnShiftButtonCAN;

		// Analog Input
				// TODO: analog read from double voltage divider, convert to voltage
				// do the window up and down checks and define if in the correct range
				// define if in error


		// UpShift Input Strategy
		if(inputs->BSteeringWheelFitted && !inputs->BUpShiftButtonCANInError) {
			inputs->BUpShiftRequest = inputs->BUpShiftButtonCAN;
			inputs->NBUpshiftRequestSource = CAN;
			inputs->BUpShiftRequestInError = 0;
		}
		else if(!inputs->BUpDnShiftButtonAnalogInError) {
			inputs->BUpShiftRequest = (inputs->NBUpDnShiftButtonAnalog == 1 ? 1 : 0);
			inputs->NBUpshiftRequestSource = Analog;
			inputs->BUpShiftRequestInError = 0;
		}
		else {
			inputs->BUpShiftRequestInError = 1;
			inputs->BUpShiftRequest = 0;		// we force to zero if in error
		}

		// DnShift Input Strategy
		if(inputs->BSteeringWheelFitted && !inputs->BDnShiftButtonCANInError) {
			inputs->BDnShiftRequest = inputs->BDnShiftButtonCAN;
			inputs->NBDnshiftRequestSource = CAN;
			inputs->BDnShiftRequestInError = 0;
		}
		else if(!inputs->BUpDnShiftButtonAnalogInError) {
			inputs->BDnShiftRequest = (inputs->NBUpDnShiftButtonAnalog == 2 ? 1 : 0);
			inputs->NBDnshiftRequestSource = Analog;
			inputs->BDnShiftRequestInError = 0;
		}
		else {
			inputs->BDnShiftRequestInError = 1;
			inputs->BDnShiftRequest = 0;		// we force to zero if in error
		}

	// ---------------------------------------------------------------------------------------------------
		// Launch Button Conditioning

		inputs->BLaunchButtonCANInError = BLaunchButtonCANInError;
		inputs->BLaunchButtonCAN = BLaunchRequestCAN;

		// Launch Input Strategy
		if(inputs->BSteeringWheelFitted && !inputs->BLaunchButtonCANInError) {
			inputs->BLaunchRequest = inputs->BLaunchButtonCAN;
			inputs->BLaunchRequestInError = 0;
		}
		else {
			inputs->BLaunchRequestInError = 1;
			inputs->BLaunchRequest = 0;		// we force to zero if in error
		}

	// ---------------------------------------------------------------------------------------------------
		// PCB Supply Voltage Conditioning

		//	inputs->VSupply = (float)ADC_VALUE * 3.3 / 4095.0 * (Voltage divider gain <- TBD as #define)
		// TODO: set the analog inputs and decide filtering (or not)


	// ---------------------------------------------------------------------------------------------------
		// nEngine Conditioning

		// CAN Input
		if((tCANECULastSeen + ECU_COMMS_LOST_INTERVAL) < tCurrent) {
			inputs->BnEngineInError = 1;
			inputs->BnEngineReliable = 0;
			inputs->nEngine = 0; 		// we force to zero if in error
//			RaiseFault(inputs, ECU_COMMS_FAULT); // TODO: we temporarily comment if for testing without the ECU
		}
		else {
			inputs->BnEngineInError = 0;
			inputs->BnEngineReliable = 1;
			ClearFault(inputs, ECU_COMMS_FAULT);
		}

		inputs->nEngine = nEngineRawCAN; // TODO: conversion??
		// TODO: we have both in error and reliable. In the controller we will consider reliable as the strategy
		// think about doing extra checks apart from CANRx timing


		if(inputs->BnEngineInError) {
			inputs->nEngine = 0; 		// we force to zero if in error
		}

	// ---------------------------------------------------------------------------------------------------
		// CAN Diagnostics
		inputs->NCANErrors = NCANErrorCount;			// update can error count
		inputs->NCANRxErrors = NCanGetRxErrorCount;	// update can Rx error count

	// ---------------------------------------------------------------------------------------------------
		// EVENTS

		if(!inputs->BUpShiftRequestInError && inputs->BUpShiftRequest && !BUpShiftRequested) {
			BUpShiftRequested = 1;
			PushEvent(inputs, UPSHIFT_PRESS_EVT);
		}
		else if(!inputs->BUpShiftRequestInError && !inputs->BUpShiftRequest && BUpShiftRequested) {
			BUpShiftRequested = 0;
			PushEvent(inputs, UPSHIFT_RELEASE_EVT);
		}

		if(!inputs->BDnShiftRequestInError && inputs->BDnShiftRequest && !BDnShiftRequested) {
			BDnShiftRequested = 1;
			PushEvent(inputs, DNSHIFT_PRESS_EVT);
		}
		else if(!inputs->BDnShiftRequestInError && !inputs->BDnShiftRequest && BDnShiftRequested) {
			BDnShiftRequested = 0;
			PushEvent(inputs, DNSHIFT_RELEASE_EVT);
		}

		if(!inputs->BLaunchRequestInError && inputs->BLaunchRequest && !BLaunchRequested) {
			BLaunchRequested = 1;
			PushEvent(inputs, LAUNCH_PRESS_EVT);
		}
		else if(!inputs->BLaunchRequestInError && !inputs->BLaunchRequest && BLaunchRequested) {
			BLaunchRequested = 0;
			PushEvent(inputs, LAUNCH_RELEASE_EVT);
		}

		if(!inputs->BEmergencyRequestInError && inputs->BEmergencyRequest && !BEmergencyRequested) {
			BEmergencyRequested = 1;
			PushEvent(inputs, EMERGENCY_PRESS_EVT);
		}
		else if(!inputs->BEmergencyRequestInError && !inputs->BEmergencyRequest && BEmergencyRequested) {
			BEmergencyRequested = 0;
			PushEvent(inputs, EMERGENCY_RELEASE_EVT);
		}

		// TODO: the release gets triggered always, so think of a better way to create only 1 event, or eliminate it completely
//		if(!inputs->BrClutchPaddleInError && (inputs->rClutchPaddle >= CLUTCH_PADDLE_PRESSED_THRESHOLD)) {
//			PushEvent(inputs, CLUTCH_PADDLE_PRESS_EVT);
//		}
//		else if(!inputs->BrClutchPaddleInError) {
//			PushEvent(inputs, CLUTCH_PADDLE_RELEASE_EVT);
//		}


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
		 tCANSteeringWheelLastSeen = HAL_GetTick();
		 BUpShiftButtonCAN = RxBuffer[0] & 0x01;
		 BUpShiftButtonCANInError = RxBuffer[0] & 0x80;	// TODO: TBC...
		 BDnShiftButtonCAN = RxBuffer[1] & 0x01;
		 BDnShiftButtonCANInError = RxBuffer[1] & 0x80; 	// TODO: TBC...
		 BLaunchRequestCAN = RxBuffer[2];
		 BLaunchButtonCANInError = RxBuffer[2] & 0x80; 	// TODO: TBC...
		 BEmergencyButtonCAN = RxBuffer[3];
		 BEmergencyButtonCANInError = RxBuffer[3] & 0x80; // TODO: TBC...
		 rClutchPaddleRawCAN = RxBuffer[4];
		 // TODO: BrClutchPaddleRawInErrorCAN = ... 	// TODO: TBC...
		 break;

	 case ECU_RX_ID:
		 tCANECULastSeen = HAL_GetTick();
		 nEngineRawCAN = RxBuffer[0] << 8 | RxBuffer[1];
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
