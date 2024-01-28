/*
 * input.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include "inputs.h"




// private variables
uint16_t NGearMap[TOTAL_GEARS][2] = {

    {3600, 3750},  // Gear 0
    {3900, 4000},  // Gear 1
    {3200, 3300},  // Gear 2
    {2250, 2400},  // Gear 3
    {1300, 1500},  // Gear 4
    {400, 600}     // Gear 5

};

// CAN
uint8_t BUpShiftRequestCAN;
uint8_t BDownShiftRequestCAN;
uint8_t BLaunchRequestCAN;
int8_t rClutchPaddleCAN;
uint16_t nEngineCAN;

uint8_t CANErrorCount;

// ADC
uint16_t NGearRawFiltered;
volatile uint8_t bufferSide;	// flag to determine the first or second half of the adc buffer for averaging

// private functions declaration
uint8_t calculateActualNGear(uint16_t NGearRaw);


void InitInputs(void){

}

void ReadInputs(InputStruct *input){

	// NGear Conditioning
	input->NGear = calculateActualNGear(NGearRawFiltered);


	// transfer CAN data into myInputs struct
	input->BUpShiftRequest = BUpShiftRequestCAN;
	input->BDownShiftRequest = BDownShiftRequestCAN;
	input->BLaunchRequest = BLaunchRequestCAN;
	input->rClutchPaddle = rClutchPaddleCAN;
	input->nEngine = nEngineCAN;

	input->NCANErrors = CANErrorCount;	// update can error count if any
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	// TODO: refine the function
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t CANRxData[8];

	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, CANRxData);

	switch(RxHeader.StdId) {

	 case STEERING_RX_ID :

		BUpShiftRequestCAN = CANRxData[0];
		BDownShiftRequestCAN = CANRxData[1];
		BLaunchRequestCAN = CANRxData[2];
		rClutchPaddleCAN = CANRxData[3];

		break;

	 case ECU_RX_ID:

		 nEngineCAN = CANRxData[0] << 8 | CANRxData[1];

		 break;

	 }

}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
	CANErrorCount++;
}


uint8_t calculateActualNGear(uint16_t NGearRaw) {

    for (uint8_t gear = 0; gear < TOTAL_GEARS; ++gear) {
        if (NGearRaw >= NGearMap[gear][0] && NGearRaw <= NGearMap[gear][1]) {
            return gear;
        }
    }
    return 255; // If no match found, return 255!
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
	// we enter here every time ADC_BUFFER_SIZE/2 samples have been moved to the adcRawValue buffer by the DMA

	uint32_t adcRawAccumulator = 0;
	int adcBufferIndexMin, adcBufferIndexMax;

	if(bufferSide == 1) {
		bufferSide = 0;		// we change the buffer side flag for the next iteration
		adcBufferIndexMin = ADC_BUFFER_SIZE/2;
		adcBufferIndexMax = ADC_BUFFER_SIZE;
	}
	else {
		bufferSide = 1;
		adcBufferIndexMin = 0;
		adcBufferIndexMax = ADC_BUFFER_SIZE/2;
	}


	for(int i = adcBufferIndexMin; i < adcBufferIndexMax; i++) {
		adcRawAccumulator += adcRawValue[i];
	}

	NGearRawFiltered = adcRawAccumulator / (ADC_BUFFER_SIZE/2);
}
