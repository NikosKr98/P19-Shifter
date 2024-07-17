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

// Timing variables
uint32_t tInputsTimmer;
uint32_t tBDriverKillTimer, tBFalseNeutral, tVUpDn;

// I/O Flags
uint8_t BUpShiftRequested, BDnShiftRequested, BLaunchRequested, BDeclutchRequested, BClutchPaddlePressed;
uint8_t BFalseNeutralState;

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
	// Driver Kill

		// Inverted logic!! DriverKill=1 means ShutDown is Open, DriverKill=0 means ShutDown is closed
	if(inputs->NSHIFTERDIN04 && (tBDriverKillTimer < tInputsTimmer) && inputs->BDriverKill) {
		inputs->BDriverKill = 0;
		tBDriverKillTimer = tInputsTimmer + DRIVER_KILL_DEBOUNCE;
	}
	else if(!inputs->NSHIFTERDIN04 & !inputs->BDriverKill) {
		inputs->BDriverKill = 1;
	}

	// ---------------------------------------------------------------------------------------------------
	// NGear Input

	// Analog Input
	inputs->VNGear = inputs->VSHIFTERAnalog04;

	// mapping
	inputs->BNGearInError = My2DMapInterpolate(TOTAL_GEARS, NGearMap, inputs->VNGear, &(inputs->NGearRaw), VNGEAR_MARGIN_MIN, VNGEAR_MARGIN_MAX);

	// conditioning (round float to nearest integer)
	inputs->NGear = (uint8_t)round(inputs->NGearRaw);

	// CLAMPING
	inputs->NGear = CLAMP(inputs->NGear, 0, MAX_GEAR);

	// False Neutral detection
	if(inputs->NGearRaw >= NGearRawLimsMaxMap[inputs->NGear] || inputs->NGearRaw <= NGearRawLimsMinMap[inputs->NGear]) {
		if(!BFalseNeutralState) {
			BFalseNeutralState = 1;
			tBFalseNeutral = tInputsTimmer + FALSE_NEUTRAL_DEBOUNCE;
		}
	}
	else {
		BFalseNeutralState = 0;
	}

	if((tBFalseNeutral < tInputsTimmer) && BFalseNeutralState) { //leave some time for the NGear to settle before deciding if it is in false neutral and to avoid flickering
		inputs->BFalseNeutral = 1; // it gets reset inside the controller code at the post shift phase after a successful gear change
	}

	// check for errors
	if(inputs->BNGearInError) {
		inputs->NGear = NGEAR_INERROR_DEFAULT;
	}

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
	// Clutch Paddle Inputs

	// CAN Input
	inputs->BrClutchPaddleRawCANInError = BrClutchPaddleRawInErrorCAN;
	inputs->rClutchPaddleRawCAN = rClutchPaddleRawCAN;

	// Analog Input & Mapping
	inputs->VrClutchPaddleRawAnalog = inputs->VSHIFTERAnalog02;
	inputs->BrClutchPaddleRawAnalogInError= My2DMapInterpolate(CLUTCH_PADDLE_MAP_SIZE, rClutchPaddleMap, inputs->VrClutchPaddleRawAnalog, &(inputs->rClutchPaddleRawAnalog), VrCLUTCH_PADDLE_MARGIN_MIN, VrCLUTCH_PADDLE_MARGIN_MAX);


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
	// Up-Dn Shift Inputs

	// CAN Input
	inputs->BUpShiftButtonCANInError = BUpShiftButtonCANInError;
	inputs->BDnShiftButtonCANInError = BDnShiftButtonCANInError;
	inputs->BUpShiftButtonCAN = BUpShiftButtonCAN;
	inputs->BDnShiftButtonCAN = BDnShiftButtonCAN;

	// Analog Input & Debouncing
	if(tVUpDn < tInputsTimmer) {
		inputs->VUpDnButtonAnalog = inputs->VSHIFTERAnalog03;
		tVUpDn = tInputsTimmer + VUPDN_DEBOUNCE;
	}

	// STUCK detection ???

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
	// Launch Button

	// Launch Input Strategy
	if(inputs->BSteeringWheelFitted) {
		inputs->BLaunchRequest = inputs->BSWButtonB;
		inputs->BLaunchRequestInError = 0;
	}
	else {
		inputs->BLaunchRequestInError = 1;
		inputs->BLaunchRequest = 0;		// we force to zero if in error
	}


	// ---------------------------------------------------------------------------------------------------
	// Rotary Switch

	inputs->VSwhitchA = inputs->VSHIFTERAnalog05;
	inputs->BNSwitchAInError = My2DMapInterpolate(SWITCHA_MAP_SIZE, NSWitchAmap, inputs->VSwhitchA, &(inputs->NSwitchARaw), VNSWITCH_MARGIN, VNSWITCH_MARGIN);

	inputs->NSwitchA = CLAMP((uint8_t)round(inputs->NSwitchARaw), 1, SWITCHA_MAP_SIZE);

	// ---------------------------------------------------------------------------------------------------
	// PCB Supply Voltage

	inputs->VSupply = inputs->VSHIFTERAnalog01 / VSUPPLY_DIVIDER_GAIN;


	// ---------------------------------------------------------------------------------------------------
	// nEngine

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
	// INPUTS  STATUS

	inputs->NInputsStatusWord = 0;
	inputs->NInputsStatusWord |= inputs->BNGearInError 					<< 0;
	inputs->NInputsStatusWord |= inputs->BUpShiftButtonCANInError 		<< 1;
	inputs->NInputsStatusWord |= inputs->BDnShiftButtonCANInError 		<< 2;
	inputs->NInputsStatusWord |= inputs->BUpDnShiftButtonAnalogInError 	<< 3;
	inputs->NInputsStatusWord |= inputs->BUpShiftRequestInError 		<< 4;
	inputs->NInputsStatusWord |= inputs->BDnShiftRequestInError 		<< 5;
	inputs->NInputsStatusWord |= inputs->BrClutchPaddleRawCANInError 	<< 6;
	inputs->NInputsStatusWord |= inputs->BrClutchPaddleRawAnalogInError << 7;
	inputs->NInputsStatusWord |= inputs->BrClutchPaddleInError 			<< 8;
	inputs->NInputsStatusWord |= inputs->BLaunchRequestInError 			<< 9;
	inputs->NInputsStatusWord |= inputs->BDeclutchRequestInError 		<< 10;
	inputs->NInputsStatusWord |= inputs->BNSwitchAInError 				<< 11;
	inputs->NInputsStatusWord |= inputs->BnEngineInError 				<< 12;
	inputs->NInputsStatusWord |= !inputs->BSteeringWheelFitted 			<< 13;	// inverted in order to simulate the error state
	inputs->NInputsStatusWord |= 0							 			<< 14;
	inputs->NInputsStatusWord |= 0						 				<< 15;

	inputs->NInputsStatusWord |= inputs->BDriverKill	 				<< 16;
	inputs->NInputsStatusWord |= inputs->BUpShiftRequest 				<< 17;
	inputs->NInputsStatusWord |= inputs->BDnShiftRequest 				<< 18;
	inputs->NInputsStatusWord |= inputs->BFalseNeutral	 				<< 19;
	inputs->NInputsStatusWord |= 0										<< 20;
	inputs->NInputsStatusWord |= 0										<< 21;
	inputs->NInputsStatusWord |= 0										<< 22;
	inputs->NInputsStatusWord |= 0										<< 23;
	inputs->NInputsStatusWord |= inputs->BLaunchRequest	 				<< 24;
	inputs->NInputsStatusWord |= inputs->BDeclutchRequest 				<< 25;
	inputs->NInputsStatusWord |= (inputs->NrClutchPaddleSource & 0x02)	<< 26;	// 2 bits (26,27)
	inputs->NInputsStatusWord |= (inputs->NBUpshiftRequestSource & 0x02)<< 28;	// 2 bits (28,29)
	inputs->NInputsStatusWord |= (inputs->NBDnshiftRequestSource & 0x02)<< 30;	// 2 bits (30,31)

	// ---------------------------------------------------------------------------------------------------

}

void InitInputs(void) {
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcRawValue, ADC_BUFFER_SIZE);
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

	 case SIU_TX_ID01 :
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

	 case ECU_TX_ID01:
		 tCANECULastSeen = HAL_GetTick();
		 nEngineRawCAN = (uint16_t)(RxBuffer[1] << 8 | RxBuffer[0]);
		 break;

	 default:
		 break;
	 }
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
