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
uint32_t tInputsTimmer;
uint32_t tToggleSwitch01, tToggleSwitch02, tToggleSwitch03, tToggleSwitch04;
uint32_t tBDriverKillTimer;

// I/O Flags
uint8_t BUpShiftRequested, BDnShiftRequested, BLaunchRequested, BDeclutchRequested, BClutchPaddlePressed;

// CAN
volatile uint8_t BUpShiftButtonCAN, BUpShiftButtonCANInError;;
volatile uint8_t BDnShiftButtonCAN, BDnShiftButtonCANInError;
volatile uint8_t BButtonACAN, BButtonBCAN, BButtonCCAN, BButtonDCAN, BButtonECAN, BButtonFCAN;
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
int8_t rClutchPaddleDeclutch;


// private functions declaration
uint8_t calculateActualNGear(uint16_t NGear, uint16_t NGearRaw);
uint16_t MyHalfBufferAverage(uint16_t *buffer, uint16_t halfsize, uint8_t side, uint8_t offset);

void ReadInputs(InputStruct *inputs){

	// Reset events
	inputs->nEventStatus = 0;

    tInputsTimmer = HAL_GetTick();

	// ---------------------------------------------------------------------------------------------------
	//Analog Inputs

	//ADC Averaging
	inputs->NADCChannel01Raw = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide, 0);	// PA0
	inputs->NADCChannel02Raw = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide, 1);	// PA1
	inputs->NADCChannel03Raw = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide, 2);	// PA2
	inputs->NADCChannel04Raw = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide, 3);	// PA3
	inputs->NADCChannel05Raw = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide, 5);	// PA5
	inputs->NADCChannel06Raw = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide, 4);	// PA4
	inputs->NADCChannel07Raw = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide, 7);	// PA7
	inputs->NADCChannel08Raw = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide, 6);	// PA7

	//Voltage Conversion
	inputs->VSHIFTERAnalog01 = (float)(inputs->NADCChannel01Raw * MCU_SUPPLY_VOLTAGE / 4095.0);
	inputs->VSHIFTERAnalog02 = (float)(inputs->NADCChannel02Raw * MCU_SUPPLY_VOLTAGE / 4095.0);
	inputs->VSHIFTERAnalog03 = (float)(inputs->NADCChannel03Raw * MCU_SUPPLY_VOLTAGE / 4095.0);
	inputs->VSHIFTERAnalog04 = (float)(inputs->NADCChannel04Raw * MCU_SUPPLY_VOLTAGE / 4095.0);
	inputs->VSHIFTERAnalog05 = (float)(inputs->NADCChannel05Raw * MCU_SUPPLY_VOLTAGE / 4095.0);
	inputs->VSHIFTERAnalog06 = (float)(inputs->NADCChannel06Raw * MCU_SUPPLY_VOLTAGE / 4095.0);
	inputs->VSHIFTERAnalog07 = (float)(inputs->NADCChannel07Raw * MCU_SUPPLY_VOLTAGE / 4095.0);
	inputs->VSHIFTERAnalog08 = (float)(inputs->NADCChannel08Raw * MCU_SUPPLY_VOLTAGE / 4095.0);

	//Digital Inputs
	inputs->NSHIFTERDIN01 = HAL_GPIO_ReadPin(DIN01_GPIO_Port, DIN01_Pin);
	inputs->NSHIFTERDIN02 = HAL_GPIO_ReadPin(DIN02_GPIO_Port, DIN02_Pin);
	inputs->NSHIFTERDIN03 = HAL_GPIO_ReadPin(DIN03_GPIO_Port, DIN03_Pin);
	inputs->NSHIFTERDIN04 = HAL_GPIO_ReadPin(DIN04_GPIO_Port, DIN04_Pin);

	// Steering Wheel Buttons
	inputs->BSWButtonA = BButtonACAN;
	inputs->BSWButtonB = BButtonBCAN;
	inputs->BSWButtonC = BButtonCCAN;
	inputs->BSWButtonD = BButtonDCAN;
	inputs->BSWButtonE = BButtonECAN;
	inputs->BSWButtonF = BButtonFCAN;

	// ---------------------------------------------------------------------------------------------------

	// TODO: we need to think the order of execution of the inputs (now they are a bit random)
	// check if there are dependencies

	// TODO: think about putting the analog read (buckup) buttons inside the CAN error state to gain some time during normal running
	// or, do them at the same time and compare inputs

	// ---------------------------------------------------------------------------------------------------
	// Driver Kill

		// Inverted logic!! DriverKill=1 means ShutDown is Open, DriverKill=0 means ShutDown is closed
	if(inputs->NSHIFTERDIN04 & (tBDriverKillTimer < tInputsTimmer) && inputs->BDriverKill) {
		inputs->BDriverKill = 0;
		tBDriverKillTimer = tInputsTimmer + DRIVER_KILL_DEBOUNCE;
	}
	else if(!inputs->NSHIFTERDIN04 & !inputs->BDriverKill) {
		inputs->BDriverKill = 1;
	}

	// ---------------------------------------------------------------------------------------------------
	// NGear Conditioning

	// Analog Input
	inputs->VNGear = inputs->VSHIFTERAnalog04;

	// mapping
	inputs->BNGearInError = My2DMapInterpolate(TOTAL_GEARS, NGearMap, inputs->VNGear, &(inputs->NGearRaw), VNGEAR_MARGIN_MIN, VNGEAR_MARGIN_MAX);

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

	if((tCANSteeringWheelLastSeen + STEERING_WHEEL_FITTED_INTERVAL) < tInputsTimmer) {
		inputs->BSteeringWheelFitted = 0;
	}
	else {
		inputs->BSteeringWheelFitted = 1;
	}

	// ---------------------------------------------------------------------------------------------------
	// DECLUTCH Input

	if(inputs->BSteeringWheelFitted) {
		inputs->BDeclutchRequest = inputs->BSWButtonF;
		inputs->BDeclutchRequestInError = 0;
	}
	else {
		inputs->BDeclutchRequestInError = 1;
		inputs->BDeclutchRequest = 0;		// we force to zero if in error
	}

	// ---------------------------------------------------------------------------------------------------
	// Clutch Paddle Conditioning

	// CAN Input
	inputs->BrClutchPaddleRawCANInError = BrClutchPaddleRawInErrorCAN;
	inputs->rClutchPaddleRawCAN = rClutchPaddleRawCAN;

	// Analog Input & Mapping
	inputs->VrClutchPaddleRawAnalog = inputs->VSHIFTERAnalog02;
	inputs->BrClutchPaddleRawAnalogInError= My2DMapInterpolate(CLUTCH_PADDLE_MAP_SIZE, rClutchMap, inputs->VrClutchPaddleRawAnalog, &(inputs->rClutchPaddleRawAnalog), VrCLUTCH_PADDLE_MARGIN_MIN, VrCLUTCH_PADDLE_MARGIN_MAX);


	// Clutch Paddle Input Strategy
	if(inputs->BSteeringWheelFitted && !inputs->BrClutchPaddleRawCANInError) {
		rClutchPaddleRaw = inputs->rClutchPaddleRawCAN;
		inputs->NrClutchPaddleSource = CAN;
		inputs->BrClutchPaddleInError = 0;

	}
	else if(!inputs->BrClutchPaddleRawAnalogInError) {
		rClutchPaddleRaw = (int8_t)round(inputs->rClutchPaddleRawAnalog);
		inputs->NrClutchPaddleSource = Analog;
		inputs->BrClutchPaddleInError = 0;
	}
	else {
		inputs->BrClutchPaddleInError = 1;
		inputs->NrClutchPaddleSource = NoSource;
		rClutchPaddleRaw = rCLUTCH_PADDLE_IN_ERROR_DEFAULT;
	}

	// DECLUTCH
	if(!inputs->BDeclutchRequestInError) {
		rClutchPaddleDeclutch = (inputs->BDeclutchRequest == 1 ? rCLUTCH_ON_DECLUTCH : 0);	// we use the button to fully press the clutch
	}

	// CLAMPING
	inputs->rClutchPaddle = CLAMP(MAX(rClutchPaddleRaw, rClutchPaddleDeclutch), CLUTCH_PADDLE_MIN, CLUTCH_PADDLE_MAX);


	// ---------------------------------------------------------------------------------------------------
	// Up-Dn Shift Conditioning

	// CAN Input
	inputs->BUpShiftButtonCANInError = BUpShiftButtonCANInError;
	inputs->BDnShiftButtonCANInError = BDnShiftButtonCANInError;
	inputs->BUpShiftButtonCAN = BUpShiftButtonCAN;
	inputs->BDnShiftButtonCAN = BDnShiftButtonCAN;

	// Analog Input
	// TODO: Debouncing and STUCK detection ???
	inputs->VUpDnButtonAnalog = inputs->VSHIFTERAnalog03;

	// Level checking
	if(inputs->NBUpDnShiftButtonAnalog >= VUPDN_NOPRESS) {
		inputs->NBUpDnShiftButtonAnalog = 0;	// None
		inputs->BUpDnShiftButtonAnalogInError = 0;
	}
	else if(inputs->VUpDnButtonAnalog <= VUPDN_UPSHIFT_MAX && inputs->VUpDnButtonAnalog >= VUPDN_UPSHIFT_MIN) {
		inputs->NBUpDnShiftButtonAnalog = 1;	// Up Shift
		inputs->BUpDnShiftButtonAnalogInError = 0;
	}
	else if(inputs->VUpDnButtonAnalog <= VUPDN_DNSHIFT_MAX && inputs->VUpDnButtonAnalog >= VUPDN_DNSHIFT_MIN) {
		inputs->NBUpDnShiftButtonAnalog = 2;	// Dn Shift
		inputs->BUpDnShiftButtonAnalogInError = 0;
	}
	else if(inputs->VUpDnButtonAnalog <= VUPDN_BOTHPRESSED_MAX && inputs->VUpDnButtonAnalog >= VUPDN_BOTHPRESSED_MIN) {
		inputs->NBUpDnShiftButtonAnalog = 0;	// None
		inputs->BUpDnShiftButtonAnalogInError = 0;
	}
	else {
		inputs->NBUpDnShiftButtonAnalog = 0;	// Error
		inputs->BUpDnShiftButtonAnalogInError = 1;
	}


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
		inputs->NBUpshiftRequestSource = NoSource;
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
		inputs->NBDnshiftRequestSource = NoSource;
	}

	// ---------------------------------------------------------------------------------------------------
	// Launch

	// Launch Input Strategy
	if(inputs->BSteeringWheelFitted) {
		inputs->BLaunchRequest = inputs->BSWButtonD;
		inputs->BLaunchRequestInError = 0;
	}
	else {
		inputs->BLaunchRequestInError = 1;
		inputs->BLaunchRequest = 0;		// we force to zero if in error
	}

	// ---------------------------------------------------------------------------------------------------
	// Toggle Switches

	// Toggle 1
	if(inputs->BSWButtonA && tToggleSwitch01 < tInputsTimmer) {
		inputs->NToggleSwitch01State ^= 1;
		tToggleSwitch01 = tInputsTimmer + TOGGLE_SWITCH_DEBOUNCE;
	}

	// Toggle 2
	if(inputs->BSWButtonB && tToggleSwitch02 < tInputsTimmer) {
		inputs->NToggleSwitch02State ^= 1;
		tToggleSwitch02 = tInputsTimmer + TOGGLE_SWITCH_DEBOUNCE;
	}

	// Toggle 3
	if(inputs->BSWButtonC && tToggleSwitch03 < tInputsTimmer) {
		inputs->NToggleSwitch03State ^= 1;
		tToggleSwitch03 = tInputsTimmer + TOGGLE_SWITCH_DEBOUNCE;
	}

	// Toggle 4
	if(inputs->BSWButtonE && tToggleSwitch04 < tInputsTimmer) {
		inputs->NToggleSwitch04State ^= 1;
		tToggleSwitch04 = tInputsTimmer + TOGGLE_SWITCH_DEBOUNCE;
	}


	// ---------------------------------------------------------------------------------------------------
	// PCB Supply Voltage Conditioning

	inputs->VSupply = inputs->VSHIFTERAnalog01 / VSUPPLY_DIVIDER_GAIN;


	// ---------------------------------------------------------------------------------------------------
	// nEngine Conditioning

	// CAN Input

	if((tCANECULastSeen + ECU_COMMS_LOST_INTERVAL) < tInputsTimmer) {
		inputs->BnEngineInError = 1;
		inputs->BnEngineReliable = 0;
		inputs->nEngine = 0; 		// we force to zero if in error
	}
	else {
		inputs->BnEngineInError = 0;
		inputs->BnEngineReliable = 1;
	}

	inputs->nEngine = nEngineRawCAN; // TODO: conversion??
	// TODO: we have both in error and reliable. In the controller we will consider reliable as the strategy
	// think about doing extra checks apart from CANRx timing, such as noise and out of bounds checks


	if(inputs->BnEngineInError) {
		inputs->nEngine = nENGINE_IN_ERROR_DEFAULT; 		// we force to zero if in error
	}

	// ---------------------------------------------------------------------------------------------------
	// CAN Diagnostics

	inputs->NCANErrors = NCANErrorCount;			// update can error count
	inputs->NCANRxErrors = NCanGetRxErrorCount;		// update can Rx error count

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

	if(!inputs->BDeclutchRequestInError && inputs->BDeclutchRequest && !BDeclutchRequested) {
		BDeclutchRequested = 1;
		PushEvent(inputs, DECLUTCH_PRESS_EVT);
	}
	else if(!inputs->BDeclutchRequestInError && !inputs->BDeclutchRequest && BDeclutchRequested) {
		BDeclutchRequested = 0;
		PushEvent(inputs, DECLUTCH_RELEASE_EVT);
	}

	if(!inputs->BrClutchPaddleInError) {
		if (inputs->rClutchPaddle >= CLUTCH_PADDLE_PRESSED_THRESHOLD && !BClutchPaddlePressed) {
			PushEvent(inputs, CLUTCH_PADDLE_PRESS_EVT);
			BClutchPaddlePressed = 1;
		}
		else if (inputs->rClutchPaddle <= CLUTCH_PADDLE_RELEASED_THRESHOLD && BClutchPaddlePressed) {
			PushEvent(inputs, CLUTCH_PADDLE_RELEASE_EVT);
			BClutchPaddlePressed = 0;
		}
	}



	// ---------------------------------------------------------------------------------------------------

}

void InitInputs(void) {
	HAL_ADCEx_Calibration_Start(&hadc1);
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

	 case SIU_RX_ID :
		 tCANSteeringWheelLastSeen = HAL_GetTick();

		 BUpShiftButtonCANInError 		= (RxBuffer[0] >> 0) & 0x01;
		 BDnShiftButtonCANInError 		= (RxBuffer[0] >> 1) & 0x01;

		 BrClutchPaddleRawInErrorCAN 	= (RxBuffer[0] >> 6) & 0x01;

		 BUpShiftButtonCAN 				= (RxBuffer[1] >> 0) & 0x01;
		 BDnShiftButtonCAN 				= (RxBuffer[1] >> 1) & 0x01;
		 BButtonACAN	 				= (RxBuffer[1] >> 2) & 0x01;
		 BButtonBCAN	 				= (RxBuffer[1] >> 3) & 0x01;
		 BButtonCCAN	 				= (RxBuffer[1] >> 4) & 0x01;
		 BButtonDCAN	 				= (RxBuffer[1] >> 5) & 0x01;
		 BButtonECAN	 				= (RxBuffer[1] >> 6) & 0x01;
		 BButtonFCAN	 				= (RxBuffer[1] >> 7) & 0x01;

		 rClutchPaddleRawCAN 			= RxBuffer[2];

		 break;

	 case ECU_RX_ID:
		 tCANECULastSeen = HAL_GetTick();
		 nEngineRawCAN = (uint16_t)(RxBuffer[0] << 8 | RxBuffer[1]);
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


uint16_t MyHalfBufferAverage(uint16_t *buffer, uint16_t halfsize, uint8_t side, uint8_t offset) {

	uint32_t Accumulator = 0;
	uint16_t SideOffset = (side == 1 ? halfsize : 0);
	uint16_t maxArrayIndex = halfsize / ADC_NUMBER_OF_CHANNELS;

 	for(uint16_t i=0; i< maxArrayIndex; i++) {
		Accumulator += buffer[(i * ADC_NUMBER_OF_CHANNELS) + offset + SideOffset];
	}

	Accumulator /= maxArrayIndex;
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
