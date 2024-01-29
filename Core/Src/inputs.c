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

volatile uint8_t NCANErrorCount;
volatile uint16_t NCanGetRxErrorCount=0;

// ADC
uint16_t NGearRawFiltered;
volatile uint8_t NAdcBufferSide; // flag to determine the first or second half of the adc buffer for averaging

// private functions declaration
uint8_t calculateActualNGear(uint16_t NGearRaw);
uint16_t MyHalfBufferAverage(uint16_t *buffer, uint16_t halfsize, uint8_t side);

void ReadInputs(InputStruct *input){

	// NGear Conditioning

		// averaging
		NGearRawFiltered = MyHalfBufferAverage(adcRawValue, ADC_BUFFER_HALF_SIZE, NAdcBufferSide);

		// mapping
		input->NGear = calculateActualNGear(NGearRawFiltered);

	// PCB Supply Voltage Conditioning
//	input->VSupply = (float)ADC_VALUE * 3.3 / 4095 * (Voltage divider gain)
	// TODO: set the analog input and decide filtering (or not)

	// transfer CAN data into myInputs struct
	input->BUpShiftRequest = BUpShiftRequestCAN;
	input->BDownShiftRequest = BDownShiftRequestCAN;
	input->BLaunchRequest = BLaunchRequestCAN;
	input->rClutchPaddle = rClutchPaddleCAN;
	input->nEngine = nEngineCAN;

	input->NCANErrors = NCANErrorCount;	// update can error count if any

}

void InitInputs(void){
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

	 case STEERING_RX_ID :

		BUpShiftRequestCAN = RxBuffer[0];
		BDownShiftRequestCAN = RxBuffer[1];
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


uint8_t calculateActualNGear(uint16_t NGearRaw) {

    for (uint8_t gear = 0; gear < TOTAL_GEARS; ++gear) {
        if (NGearRaw >= NGearMap[gear][0] && NGearRaw <= NGearMap[gear][1]) {
            return gear;
        }
    }
    return 255; // If no match found, return 255!
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
